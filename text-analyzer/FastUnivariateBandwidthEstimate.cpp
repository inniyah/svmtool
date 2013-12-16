#include "FastUnivariateBandwidthEstimate.h"
#include "FastUnivariateDensityDerivative.h"
#include "LMMin.h"

#include <math.h>
#include <assert.h>

double FastUnivariateBandwidthEstimate::lm_data_struct::h_function(double h) {
	assert(finite(h));

	double lambda = constant2 * pow(h, 5.0 / 7.0); 

	double * D4 = new double[N];

	FastUnivariateDensityDerivative * pUDD = new FastUnivariateDensityDerivative(N, N, X, X, lambda, 4.0, epsil, D4);
	pUDD->Evaluate();
	delete pUDD;

	double phi4 = sum(N, D4) / (N - 1.0);

	delete [] D4;

	double value = h - constant1 * pow(phi4, -1.0 / 5.0);
	return value;
}

void FastUnivariateBandwidthEstimate::lm_evaluate(
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
//     AMISE optimal bandwidth estimation for univariate kernel 
//     density estimation. [FAST METHOD] 
//     [Shether Jones Solve-the-equation plug-in method.] 
//     -------------------------------------------------------------------------- 
//     INPUTS 
//     -------------------------------------------------------------------------- 
//     N                 --> number of source points. 
//     X                 --> 1 x N matrix of N source points. 
//     epsil           --> accuracy parametr for fast density derivative 
//                              estimation [1e-3 to 1e-6] 
//     -------------------------------------------------------------------------- 
//     OUTPUTS 
//     -------------------------------------------------------------------------- 
//     h                 --> the optimal bandiwdth estimated using the 
//                              Shether Jones Solve-the-equation plug-in 
//                              method. 
//     -------------------------------------------------------------------------- 
//     Implementation based on: 
// 
//     V. C. Raykar and R. Duraiswami 'Very fast optimal bandwidth  
//     selection for univariate kernel density estimation' 
//     Technical Report CS-TR-4774, Dept. of Computer  
//     Science, University of Maryland, College Park. 
// 
//    S.J. Sheather and M.C. Jones. 'A reliable data-based 
//    bandwidth selection method for kernel density estimation' 
//    J. Royal Statist. Soc. B, 53:683-690, 1991 
// -------------------------------------------------------------------------- 

/*
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
*/
double FastUnivariateBandwidthEstimate::Evaluate(unsigned int N, const double * X, double epsil, int verbosity) {
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

	FastUnivariateDensityDerivative * pUDD;

	double * D4 = new double[N];

	pUDD = new FastUnivariateDensityDerivative(N, N,
		X_shifted_scaled, X_shifted_scaled, g1, 4.0, epsil, D4);
	pUDD->Evaluate();
	delete pUDD;

	phi4 = sum(N, D4) / (N - 1.0); 

	delete [] D4; D4 = NULL;

	double * D6 = new double[N];

	pUDD = new FastUnivariateDensityDerivative(N, N,
		X_shifted_scaled, X_shifted_scaled, g2, 6.0, epsil, D6);
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
	data.epsil = epsil;

	// auxiliary parameters
	lm_status_struct status;
	lm_control_struct control = lm_control_double;
	control.verbosity = verbosity;

	// perform the fit
	lmmin( n_par, par, m_dat, (const void*) &data,
		&FastUnivariateBandwidthEstimate::lm_evaluate, &control, &status );

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

	double epsilon = 1e-6; // Input: error
	FastUnivariateBandwidthEstimate estimate;
	h = estimate.Evaluate(N, X, epsilon, 3);

	double r = 6; // Input: derivative order, must be a positive integer, or zero
	double D[M]; // Output: array of M elements, density derivative evaluated at each target point
	for (int i = 0; i < M; ++i) D[i] = 0.0; // Initialize results array

	FastUnivariateDensityDerivative * pUDD = new FastUnivariateDensityDerivative(N, M, X, Y, h, r, epsilon, D);
	pUDD->Evaluate();
	delete pUDD;

	printf("D = { ");
	for (int i = 0; i < M; ++i) printf("%.3lf ", D[i] / pow(10.0, r));
	printf("}\n");

	return EXIT_SUCCESS;
}

#endif
