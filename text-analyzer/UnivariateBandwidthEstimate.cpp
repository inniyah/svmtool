#include "UnivariateBandwidthEstimate.h"
#include "UnivariateDensityDerivative.h"
#include "LMMin.h"

#include <math.h>
#include <assert.h>

double UnivariateBandwidthEstimate::sum(unsigned int num, const double * data) {
	if (!num || !data) {
		return 0.0;
	}
	double total = 0.0;
	while (num) {
		--num;
		total += *data;
		++data;
	}
	return total;
}

double UnivariateBandwidthEstimate::min(unsigned int num, const double * data) {
	if (!num || !data) {
		return 0.0;
	}
	double value = *data;
	++data;
	--num;
	 while (num) {
		if (*data < value) {
			value= *data;
		}
		++data; --num;
	}
	return value;
}

double UnivariateBandwidthEstimate::max(unsigned int num, const double * data) {
	if (!num || !data) {
		return 0.0;
	}
	double value = *data;
	++data;
	--num;
	 while (num) {
		if (*data > value) {
			value= *data;
		}
		++data; --num;
	}
	return value;
}

double UnivariateBandwidthEstimate::std(unsigned int num, const double * data) {
	if (!num || !data) {
		return 0.0;
	}
	double E = 0.0;
	double inverse = 1.0 / static_cast<double>(num);
	double ave = inverse * sum(num, data);
	while (num) {
		E += pow(*data - ave, 2.0);
		++data; --num;
	}
	return sqrt(inverse * E);
}

double UnivariateBandwidthEstimate::lm_data_struct::h_function(double h) {
	assert(finite(h));

	double lambda = constant2 * pow(h, 5.0 / 7.0); 

	double * D4 = new double[N];

	UnivariateDensityDerivative * pUDD = new UnivariateDensityDerivative(N, N, X, X, lambda, 4.0, D4);
	pUDD->Evaluate();
	delete pUDD;

	double phi4 = sum(N, D4) / (N - 1.0);

	delete [] D4;

	double value = h - constant1 * pow(phi4, -1.0 / 5.0);
	return value;
}

void UnivariateBandwidthEstimate::lm_evaluate(
	const double * par,
	int m_dat,
	const void * p_data,
	double * fvec,
	int * userbreak )
{
	lm_data_struct * data = (lm_data_struct *)p_data;
	for (int i = 0; i < m_dat; ++i) {
		fvec[i] = data->h_function(par[0]);
	}
}

// -------------------------------------------------------------------------- 
// AMISE optimal bandwidth estimation for univariate kernel 
// density estimation. [SLOW METHOD] 
// [Shether Jones Solve-the-equation plug-in method.] 
// -------------------------------------------------------------------------- 
// INPUTS 
// -------------------------------------------------------------------------- 
//     N                 --> number of source points. 
//     X                 --> 1 x N matrix of N source points. 
// -------------------------------------------------------------------------- 
// OUTPUTS 
// -------------------------------------------------------------------------- 
//     h                 --> the optimal bandiwdth estimated using the 
//                              Shether Jones Solve-the-equation plug-in 
//                              method. 
// -------------------------------------------------------------------------- 
// Implementation based on: 
//
//    S.J. Sheather and M.C. Jones. 'A reliable data-based 
//    bandwidth selection method for kernel density estimation' 
//    J. Royal Statist. Soc. B, 53:683-690, 1991 
// -------------------------------------------------------------------------- 
double UnivariateBandwidthEstimate::Evaluate(unsigned int N, const double * X, int verbosity) {
	double phi4, phi6, phi8;

	// Scale the data to lie in the unit interval [0 1]
	double * X_shifted_scaled = new double[N];
	double shift = min(N, X);
	for (unsigned int i = 0; i < N; ++i) {
		X_shifted_scaled[i] = X[i] - shift;
	}
	double scale = 1.0 / (max(N, X_shifted_scaled));
	for (unsigned int i = 0; i < N; ++i) {
		X_shifted_scaled[i] = X_shifted_scaled[i] * scale;
	}

	// Compute an estimate of the standard deviation of the data
	double sigma = std(N, X_shifted_scaled); 

	// Estimate the density functionals ${\Phi}_6$ and ${\Phi}_8$ using the normal scale rule.
	phi6 = (-15.0 / (16.0 * sqrt(M_PI))) * pow(sigma, -7.0);
	phi8 = (105.0 / (32.0 * sqrt(M_PI))) * pow(sigma, -9.0);

	// Estimate the density functionals ${\Phi}_4$ and ${\Phi}_6$ using the kernel density
	// estimators with the optimal bandwidth based on the asymptotic MSE. 
	double g1 = pow(-6.0 / (sqrt(2.0*M_PI) * phi6 * N), 1.0 / 7.0);
	double g2 = pow(30.0 / (sqrt(2.0*M_PI) * phi8 * N), 1.0 / 9.0);

	UnivariateDensityDerivative * pUDD;

	double * D4 = new double[N];

	pUDD = new UnivariateDensityDerivative(N, N,
		X_shifted_scaled, X_shifted_scaled, g1, 4.0, D4);
	pUDD->Evaluate();
	delete pUDD;

	phi4 = sum(N, D4) / (N - 1.0); 

	delete [] D4; D4 = NULL;

	double * D6 = new double[N];

	pUDD = new UnivariateDensityDerivative(N, N,
		X_shifted_scaled, X_shifted_scaled, g2, 6.0, D6);
	pUDD->Evaluate();
	delete pUDD;

	phi6 = sum(N, D6) / (N - 1.0); 

	delete [] D6; D6 = NULL;

	// The bandwidth is the solution to the following equation.

	double constant1 = pow(1.0 / (2.0 * sqrt(M_PI) * N), 1.0 / 5.0);
	double constant2 = pow(-6.0 * sqrt(2.0) * phi4 / phi6, 1.0 / 7.0);

	double h_initial = constant1 * pow(phi4, -1.0 / 5.0); 

	const int n_par = 1; // Number of free variables. Dimension of parameter vector par
	double par[n_par] = { h_initial }; // Parameter vector
	const int m_dat = 1; // Dimension of vector fvec. Must statisfy n_par <= m_dat.

	lm_data_struct data;
	data.N = N;
	data.X = X_shifted_scaled;
	data.constant1 = constant1;
	data.constant2 = constant2;

	// auxiliary parameters
	lm_status_struct status;
	lm_control_struct control = lm_control_double;
	control.verbosity = verbosity;

	// perform the fit
	lmmin( n_par, par, m_dat, (const void*) &data,
		&UnivariateBandwidthEstimate::lm_evaluate, &control, &status );

	delete [] X_shifted_scaled; data.X = X_shifted_scaled = NULL;

	double h = par[0];
	h = h / scale; 
	return h;
}

#if 0

#include <cstdlib>
#include <cstdio>
#include <ctime>

int main(int argc, const char * argv[])
{
	srand(time(NULL)); // Initialize random seed
	int N = 100; // Input: number of source points
	int M = 10; // Input: number of target points
	double X[N]; // Input: array of N source points
	double Y[M]; // Input: array of M target points
	for (int i = 0; i < N; ++i) X[i] = rand() / double(RAND_MAX); // Generate N random numbers between 0.0 and 1.0
	for (int i = 0; i < M; ++i) Y[i] = rand() / double(RAND_MAX); // Generate M random numbers between 0.0 and 1.0
	double h = 0.1; // Input: source bandwidth or scale, must be a positive number, greater than zero

	UnivariateBandwidthEstimate estimate;
	h = estimate.Evaluate(N, X, 3);

	double r = 6; // Input: derivative order, must be a positive integer, or zero
	double D[M]; // Output: array of M elements, density derivative evaluated at each target point
	for (int i = 0; i < M; ++i) D[i] = 0.0; // Initialize results array

	UnivariateDensityDerivative * pUDD = new UnivariateDensityDerivative(N, M, X, Y, h, r, D);
	pUDD->Evaluate();
	delete pUDD;

	printf("D = { ");
	for (int i = 0; i < M; ++i) printf("%.3lf ", D[i] / pow(10.0, r));
	printf("}\n");

	return EXIT_SUCCESS;
}

#endif


/*

http://www.pudn.com/downloads185/sourcecode/graph/texture_mapping/detail869231.html

clear all; 
close all; 
clear functions; 
clc; 
 
disp('---------------------------------------------'); 
disp(sprintf('Example to demonstrate the use of fast optimal bandwidth selection')); 
disp('---------------------------------------------'); 
 
% sample normal mixture density of Marron and Wand [ can vary form 1-15] 
 
density_number=2; 
 
%number of datapoints 
 
N=5000; 
 
disp('---------------------------------------------'); 
disp(sprintf('Sampling %d points from a sample normal mixture density %d of Marron and Wand',N,density_number)); 
disp('---------------------------------------------'); 
 
Y=[-5.0:0.01:5.0]; 
[actual_density,X]=marron_wand_normal_mixtures(density_number,Y,N); 
 
disp('---------------------------------------------'); 
disp(sprintf('Estimating the AMISE optimal bandwidth-SLOW method')); 
disp('---------------------------------------------'); 
 
to=clock;     
[h_slow]=slow_univariate_bandwidth_estimate_STEPI(N,X); 
slow_time=etime(clock,to); 
 
disp(sprintf('h_optimal=%f Time taken=%f seconds',h_slow,slow_time)); 
 
epsil=1e-3; 
 
disp('---------------------------------------------'); 
disp(sprintf('Estimating the AMISE optimal bandwidth-Fast method with epsilon=%e',epsil)); 
disp('---------------------------------------------'); 
 
to=clock;     
[h_fast]=fast_univariate_bandwidth_estimate_STEPI(N,X,epsil); 
fast_time=etime(clock,to); 
 
disp(sprintf('h_optimal=%f Time taken=%f seconds',h_fast,fast_time)); 
 
rel_error_per=abs((h_slow-h_fast)/h_slow)*100; 
speedup=slow_time/fast_time; 
 
disp('---------------------------------------------'); 
disp(sprintf('Speedup=%f Relative percentage error=%d %% ',speedup,rel_error_per)); 
disp('---------------------------------------------'); 
 
disp('---------------------------------------------'); 
disp(sprintf('Plotting the KDE')); 
disp('---------------------------------------------'); 
 
min_x=min(X); 
min_y=min(Y); 
shift=min(min_x, min_y);  
X_shifted=X-shift; 
Y_shifted=Y-shift; 
max_x=max(X_shifted); 
max_y=max(Y_shifted); 
scale=1/max(max_x,max_y);  
X_shifted_scaled=X_shifted*scale; 
Y_shifted_scaled=Y_shifted*scale; 
 
M=length(Y); 
epsil=1e-6; 
[kde]=FastUnivariateDensityDerivative(N,M,X_shifted_scaled,Y_shifted_scaled,h_fast*scale,0,epsil); 
 
figure; 
plot(Y,actual_density,'k'); 
hold on; 
plot(Y,scale*kde,'r--'); 
plot(X,0,'k+'); 
legend('Actual','KDE with optimal h');

---------------------------------------------------------------------------------------------------

function [h]=slow_univariate_bandwidth_estimate_STEPI(N,X) 
 
%     AMISE optimal bandwidth estimation for univariate kernel 
%     density estimation. [SLOW METHOD] 
%     [Shether Jones Solve-the-equation plug-in method.] 
%     -------------------------------------------------------------------------- 
%     INPUTS 
%     -------------------------------------------------------------------------- 
%     N                 --> number of source points. 
%     X                 --> 1 x N matrix of N source points. 
%     -------------------------------------------------------------------------- 
%     OUTPUTS 
%     -------------------------------------------------------------------------- 
%     h                 --> the optimal bandiwdth estimated using the 
%                              Shether Jones Solve-the-equation plug-in 
%                              method. 
%     -------------------------------------------------------------------------- 
%     Implementation based on: 
% 
%    S.J. Sheather and M.C. Jones. 'A reliable data-based 
%    bandwidth selection method for kernel density estimation' 
%    J. Royal Statist. Soc. B, 53:683-690, 1991 
%-------------------------------------------------------------------------- 
 
 
% Scale the data to lie in the unit interval [0 1] 
 
shift=min(X); X_shifted=X-shift; 
scale=1/(max(X_shifted)); X_shifted_scaled=X_shifted*scale; 
 
% Compute an estimate of the standard deriviation of the data 
 
sigma=std(X_shifted_scaled); 
 
% Estimate the density functionals ${\Phi}_6$ and ${\Phi}_8$ using the normal scale rule. 
 
phi6=(-15/(16*sqrt(pi)))*(sigma^(-7)); 
phi8=(105/(32*sqrt(pi)))*(sigma^(-9)); 
 
% Estimate the density functionals ${\Phi}_4$ and ${\Phi}_6$ using the kernel density 
% estimators with the optimal bandwidth based on the asymptotic MSE. 
 
g1=(-6/(sqrt(2*pi)*phi6*N))^(1/7); 
g2=(30/(sqrt(2*pi)*phi8*N))^(1/9); 
 
[D4]=UnivariateDensityDerivative(N,N,X_shifted_scaled,X_shifted_scaled,g1,4); 
phi4=sum(D4)/(N-1); 
 
[D6]=UnivariateDensityDerivative(N,N,X_shifted_scaled,X_shifted_scaled,g2,6); 
phi6=sum(D6)/(N-1); 
 
% The bandwidth is the solution to the following  equation. 
 
constant1=(1/(2*sqrt(pi)*N))^(1/5); 
constant2=(-6*sqrt(2)*phi4/phi6)^(1/7); 
 
h_initial=constant1*phi4^(-1/5); 
 
options = optimset('Display','off','TolFun',1e-14,'TolX',1e-14,'LargeScale','on'); 
data.N=N; 
data.X=X_shifted_scaled; 
data.constant1=constant1; 
data.constant2=constant2; 
 
[h,resnorm,residual,exitflag,output] = lsqnonlin('h_function',h_initial,[0],[],options,data) ; 
 
h=h/scale; 
 
if exitflag>0    disp('The function converged to a solution.'); end 
if exitflag==0  disp('The maximum number of function evaluations or iterations was exceeded.'); end 
if exitflag<0    disp('The function did not converge to a solution.'); end 

---------------------------------------------------------------------------------------------------

function F = h_function(h,data) 
 
lambda=data.constant2*h^(5/7); 
 
[D4]=UnivariateDensityDerivative(data.N,data.N,data.X,data.X,lambda,4); 
phi4=sum(D4)/(data.N-1); 
 
F=h-data.constant1*phi4^(-1/5);

---------------------------------------------------------------------------------------------------

function [h]=fast_univariate_bandwidth_estimate_STEPI(N,X,epsil) 
 
%     AMISE optimal bandwidth estimation for univariate kernel 
%     density estimation. [FAST METHOD] 
%     [Shether Jones Solve-the-equation plug-in method.] 
%     -------------------------------------------------------------------------- 
%     INPUTS 
%     -------------------------------------------------------------------------- 
%     N                 --> number of source points. 
%     X                 --> 1 x N matrix of N source points. 
%     epsil           --> accuracy parametr for fast density derivative 
%                              estimation [1e-3 to 1e-6] 
%     -------------------------------------------------------------------------- 
%     OUTPUTS 
%     -------------------------------------------------------------------------- 
%     h                 --> the optimal bandiwdth estimated using the 
%                              Shether Jones Solve-the-equation plug-in 
%                              method. 
%     -------------------------------------------------------------------------- 
%     Implementation based on: 
% 
%     V. C. Raykar and R. Duraiswami 'Very fast optimal bandwidth  
%     selection for univariate kernel density estimation' 
%     Technical Report CS-TR-4774, Dept. of Computer  
%     Science, University of Maryland, College Park. 
% 
%    S.J. Sheather and M.C. Jones. 'A reliable data-based 
%    bandwidth selection method for kernel density estimation' 
%    J. Royal Statist. Soc. B, 53:683-690, 1991 
%    --------------------------------------------------------------------------- 
 
 
% Scale the data to lie in the unit interval 
 
shift=min(X); X_shifted=X-shift; 
scale=1/(max(X_shifted)); X_shifted_scaled=X_shifted*scale; 
 
% Compute an estimate of the standard deriviation of the data 
 
sigma=std(X_shifted_scaled); 
 
% Estimate the density functionals ${\Phi}_6$ and ${\Phi}_8$ using the normal scale rule. 
 
phi6=(-15/(16*sqrt(pi)))*(sigma^(-7)); 
phi8=(105/(32*sqrt(pi)))*(sigma^(-9)); 
 
% Estimate the density functionals ${\Phi}_4$ and ${\Phi}_6$ using the kernel density 
% estimators with the optimal bandwidth based on the asymptotic MSE. 
 
g1=(-6/(sqrt(2*pi)*phi6*N))^(1/7); 
g2=(30/(sqrt(2*pi)*phi8*N))^(1/9); 
 
[D4]=FastUnivariateDensityDerivative(N,N,X_shifted_scaled,X_shifted_scaled,g1,4,epsil); 
phi4=sum(D4)/(N-1); 
 
[D6]=FastUnivariateDensityDerivative(N,N,X_shifted_scaled,X_shifted_scaled,g2,6,epsil); 
phi6=sum(D6)/(N-1); 
 
% The bandwidth is the solution to the following  equation. 
 
constant1=(1/(2*sqrt(pi)*N))^(1/5); 
constant2=(-6*sqrt(2)*phi4/phi6)^(1/7); 
 
h_initial=constant1*phi4^(-1/5); 
 
options = optimset('Display','off','TolFun',1e-14,'TolX',1e-14,'LargeScale','on'); 
data.N=N; 
data.X=X_shifted_scaled; 
data.constant1=constant1; 
data.constant2=constant2; 
data.epsil=epsil; 
 
[h,resnorm,residual,exitflag,output] = lsqnonlin('fast_h_function',h_initial,[0],[],options,data) ; 
 
h=h/scale; 
 
if exitflag>0    disp('The function converged to a solution.'); end 
if exitflag==0  disp('The maximum number of function evaluations or iterations was exceeded.'); end 
if exitflag<0    disp('The function did not converge to a solution.'); end 

---------------------------------------------------------------------------------------------------

function F = fast_h_function(h,data) 
 
lambda=data.constant2*h^(5/7); 
[D4]=FastUnivariateDensityDerivative(data.N,data.N,data.X,data.X,lambda,4,data.epsil); 
phi4=sum(D4)/(data.N-1); 
F=h-data.constant1*phi4^(-1/5);

---------------------------------------------------------------------------------------------------

function [actual_density,X]=marron_wand_normal_mixtures(density_number,Y,N) 
 
% 
%    [actual_density,X]= 
%            marron_wand_normal_mixtures(density_number,Y,N) 
%     -------------------------------------------------------------------------- 
%     This function returns the actual density and samples from the 
%     15 normal mixture densities used by Marron and Wand in 
%      their paper: 
% 
%     J. S. Marron and M. P. Wand 'Exact Mean Integrated Squared 
%     Error' The Annals of Statistics, 1992, Vol. 20, No. 2, 712-736 
%     -------------------------------------------------------------------------- 
%     Author     :     Vikas.C.Raykar 
%     Date        :     25 June 2005 
%     Contact    :     vikas@cs.umd.edu 
%     -------------------------------------------------------------------------- 
%     INPUTS 
%     -------------------------------------------------------------------------- 
%     density_number --> can vary form 1-15 (See below) 
%     Y                       --> data points at which the actual density is 
%                                    to be evaluated 
%     N                       --> number of samples. 
%     -------------------------------------------------------------------------- 
%     OUTPUTS 
%     -------------------------------------------------------------------------- 
%     actual_density   --> the actual density evaluated at Y. 
%     X                      --->N samples from the density. 
%     -------------------------------------------------------------------------- 
% 
% 1   --> Gaussian 
% 2   --> Skewed unimodal 
% 3   --> Strongly skewed 
% 4   --> Kurtotic unimodal 
% 5   --> Outlier 
% 6   --> Bimodal 
% 7   --> Separated bimodal 
% 8   --> Skewed bimodal 
% 9   --> Trimodal 
% 10 -->  Claw 
% 11 -->  Double Claw 
% 12 -->  Aymmetric Claw 
% 13 -->  Asymmetric Double Claw 
% 14 -->  Smooth Comb 
% 15 -->  Discrete Comb 
 
if density_number==1 
    actual_density=univariate_normal(0,1,Y); 
    X=sample_univariate_normal(N,1,0,1,1); 
end 
 
if density_number==2 
    G=3; 
    weights=[1/5 1/5 3/5]; 
    m=[0 1/2 13/12]; 
    sigma=[1 2/3 5/9]; 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==3 
    G=8; 
    for j=1:8 
        weights(j)=1/8; 
        sigma(j)=(2/3)^(j-1); 
        m(j)=3*((2/3)^(j-1)-1); 
    end 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==4 
    G=2; 
    weights=[2/3 1/3]; 
    m=[0  0]; 
    sigma=[1 1/10]; 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
 
if density_number==5 
    G=2; 
    weights=[1/10 9/10]; 
    m=[0  0]; 
    sigma=[1 1/10]; 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==6 
    G=2; 
    weights=[1/2 1/2]; 
    m=[-1  1]; 
    sigma=[2/3 2/3]; 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==7 
    G=2; 
    weights=[1/2 1/2]; 
    m=[-3/2  3/2]; 
    sigma=[2/3 2/3]; 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==8 
    G=2; 
    weights=[3/4 1/4]; 
    m=[0 3/2]; 
    sigma=[1 1/3]; 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
 
if density_number==9 
    G=3; 
    weights=[9/20 9/20 1/10]; 
    m=[-6/5 6/5 0]; 
    sigma=[3/5 3/5 1/4]; 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
 
if density_number==10 
    G=6; 
    weights(1)=1/2; 
    sigma(1)=1; 
    m(1)=0; 
    for j=2:6 
        weights(j)=1/10; 
        sigma(j)=1/10; 
        m(j)=((j-2)/2)-1; 
    end 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==11 
    G=9; 
    weights(1)=49/100;    sigma(1)=2/3;    m(1)=-1; 
    weights(2)=49/100;    sigma(2)=2/3;    m(2)=1; 
    for j=3:9 
        weights(j)=1/350; 
        sigma(j)=1/100; 
        m(j)=((j-6)/2); 
    end 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==12 
    G=6; 
    weights(1)=1/2; 
    sigma(1)=1; 
    m(1)=0; 
    k=2; 
    for j=-2:1:2 
        weights(k)=2^(1-j)/31; 
        sigma(k)=2^(-j)/10; 
        m(k)=j+(1/2); 
        k=k+1; 
    end 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
 
if density_number==13 
    G=8; 
    weights(1)=46/100;    sigma(1)=2/3;    m(1)=-1; 
    weights(2)=46/100;    sigma(2)=2/3;    m(2)=1; 
    k=3; 
    for j=1:3 
        weights(k)=1/300; 
        sigma(k)=1/100; 
        m(k)=-j/2; 
        k=k+1; 
    end 
    for j=1:3 
        weights(k)=7/300; 
        sigma(k)=7/100; 
        m(k)=j/2; 
        k=k+1; 
    end 
 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
if density_number==14 
    G=6; 
    k=1; 
    for j=0:5 
        weights(k)=(2^(5-j))/63; 
        sigma(k)=((32/63))/(2^j); 
        m(k)=(65-(96*((1/2)^(j))))/21; 
        k=k+1; 
    end 
 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
 
if density_number==15 
    G=6; 
    k=1; 
    for j=0:2 
        weights(k)=2/7; 
        sigma(k)=2/7; 
        m(k)=(12*j-15)/7; 
        k=k+1; 
    end 
    for j=8:10 
        weights(k)=1/21; 
        sigma(k)=1/21; 
        m(k)=(2*j)/7; 
        k=k+1; 
    end 
 
    actual_density=zeros(size(Y)); 
    for i=1:G 
        actual_density=actual_density+weights(i)*univariate_normal(m(i),sigma(i),Y); 
    end 
    X=sample_univariate_normal(N,G,m,sigma,weights); 
end 
 
% figure; 
% plot(Y,actual_density,'k'); 
% hold on; 
% plot(X,0,'k+'); 
 
return 
 
%-------------------------------------------------------------------------- 
 
function [density]=univariate_normal(m,sigma,Y) 
 
density=(1/sqrt(2*pi*sigma*sigma))*exp(-((Y-m).^2)/(2*sigma*sigma)); 
 
return 
 
%-------------------------------------------------------------------------- 
 
function [X]=sample_univariate_normal(N,G,m,sigma,weights) 
 
for i=1:G 
    n(i)=round(weights(i)*N); 
end 
n(G)=n(G)+(N-sum(n)); 
 
X=[]; 
for i=1:G 
    X=cat(2,X,m(:,i)*ones(1,n(i))+sigma(i)*randn(1,n(i))); 
end 
 
%-------------------------------------------------------------------------- 


*/