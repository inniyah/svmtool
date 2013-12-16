//-------------------------------------------------------------------
// The code was written by Vikas C. Raykar 
// and is copyrighted under the Lessr GPL: 
//
// Copyright (C) 2006 Vikas C. Raykar
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 or later.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU Lesser General Public License for more details. 
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, 
// MA 02111-1307, USA.  
//
// The author may be contacted via email at: vikas(at)cs(.)umd(.)edu 
//-------------------------------------------------------------------

//-------------------------------------------------------------
// File    : UnivariateDensityDerivative.cpp
// Purpose : Implementation for for UnivariateDensityDerivative
// Author  : Vikas C. Raykar (vikas@cs.umd.edu)
// Date    : September 17, 2005
//-------------------------------------------------------------

#include "UnivariateDensityDerivative.h"

#include <math.h>

//-------------------------------------------------------------------
// Constructor.
//
// PURPOSE                                                    
// -------   
// Initialize the class. 
// Read the parameters.
//
// PARAMETERS                                                      
// ----------
// NSources             --> number of sources, N.
// MTargets             --> number of targets, M.
// pSources             --> pointer to sources, px(N).
// pTargets             --> pointer to the targets, py(M).
// Bandwidth            --> the source bandwidth, h.
// Order                --> order of the derivative, r.  
// pDensityDerivative   --> pointer the the evaluated Density 
//-------------------------------------------------------------------


UnivariateDensityDerivative::UnivariateDensityDerivative(int NSources,
			int MTargets,
			double *pSources,
			double *pTargets,
			double Bandwidth,
		      int Order,
			double *pDensityDerivative)
{
	N = NSources;
	M = MTargets;
	px = pSources;
	h = Bandwidth;
	r = Order;
	py = pTargets;
	pD = pDensityDerivative;
}

//-------------------------------------------------------------------
// Destructor.
//-------------------------------------------------------------------

UnivariateDensityDerivative::~UnivariateDensityDerivative() {
}

//-------------------------------------------------------------------
// Actual function to evaluate the Univariate Density Derivative.
//-------------------------------------------------------------------

void UnivariateDensityDerivative::Evaluate() {
	double two_h_square=2*h*h;
	double pi=3.14159265358979;
	double q=(pow(-1,r))/(sqrt(2*pi)*N*(pow(h,(r+1))));

	for(int j=0; j<M; j++)
	{
		pD[j]=0.0;

		for(int i=0; i<N; i++)
		{
			double temp=py[j]-px[i];
			double norm=temp*temp;
			
			pD[j] = pD[j]+(hermite(temp/h,r)*exp(-norm/two_h_square));

		}
		pD[j]=pD[j]*q;
	}
}

//-------------------------------------------------------------------
// Recursive implementation of the Hermite polynomial.
//-------------------------------------------------------------------

double UnivariateDensityDerivative::hermite(double x, int r) {
	if(r==0)
	{
		return (1.0);
	}
	else if(r==1)
	{
		return (x);
	}
	else
	{
		return (x*hermite(x,r-1))-((r-1)*hermite(x,r-2));
	}

}

#if 0

#include <cstdlib>
#include <cstdio>
#include <ctime>

int main(int argc, const char * argv[])
{
	srand(time(NULL)); // Initialize random seed
	int N = 100; // Input: number of source points
	int M = 100; // Input: number of target points
	double X[N]; // Input: array of N source points
	double Y[M]; // Input: array of M target points
	for (int i = 0; i < N; ++i) X[i] = rand() / double(RAND_MAX); // Generate N random numbers between 0.0 and 1.0
	for (int i = 0; i < M; ++i) Y[i] = rand() / double(RAND_MAX); // Generate M random numbers between 0.0 and 1.0
	double h = 0.1; // Input: source bandwidth or scale, must be a positive number, greater than zero
	double r = 6; // Input: derivative order, must be a positive integer, or zero
	double D[M]; // Output: array of M elements, density derivative evaluated at each target point
	for (int i = 0; i < M; ++i) D[i] = 0.0; // Initialize results array

	UnivariateDensityDerivative * pUDD = new UnivariateDensityDerivative(N, M, X, Y, h, r, D);
	pUDD->Evaluate();
	delete pUDD;

	printf("D = { ");
	for (int i = 0; i < M; ++i) printf("%lf ", D[i] / pow(10.0, r));
	printf("}\n");

	return EXIT_SUCCESS;
}

#endif
