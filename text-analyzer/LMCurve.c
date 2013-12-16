/* 
 * Contents:  Levenberg-Marquardt curve-fitting
 * Copyright: Joachim Wuttke, Forschungszentrum Juelich GmbH (2004-2013)
 * 
 *   Copyright (c) 1980-1999 University of Chicago,
 *                           as operator of Argonne National Laboratory
 *   Copyright (c) 2004-2013 Joachim Wuttke, Forschungszentrum Juelich GmbH
 * 
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 * 
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 
 *   This software is provided by the copyright holders and contributors "as is"
 *   and any express or implied warranties, including, but not limited to, the
 *   implied warranties of merchantability and fitness for a particular purpose
 *   are disclaimed. In no event shall the copyright holder or contributors
 *   be liable for any direct, indirect, incidental, special, exemplary, or
 *   consequential damages (including, but not limited to, procurement of
 *   substitute goods or services; loss of use, data, or profits; or business
 *   interruption) however caused and on any theory of liability, whether in
 *   contract, strict liability, or tort (including negligence or otherwise)
 *   arising in any way out of the use of this software, even if advised of the
 *   possibility of such damage.
 * 
 *   This file is originally part of lmfit (Levenberg-Marquardt least squares
 *   fitting), a self-contained C library for minimization and curve fitting,
 *   release 5.1 (29 Aug 2013)
 * 
 *   See: http://apps.jcns.fz-juelich.de/lmfit
 */

#include "LMMin.h"


typedef struct {
    const double *t;
    const double *y;
    double (*f) (double t, const double *par);
} lmcurve_data_struct;


void lmcurve_evaluate( const double *par, int m_dat, const void *data,
                       double *fvec, int *info )
{
    int i;
    for ( i = 0; i < m_dat; i++ )
        fvec[i] =
            ((lmcurve_data_struct*)data)->y[i] -
            ((lmcurve_data_struct*)data)->f(
                ((lmcurve_data_struct*)data)->t[i], par );
}


void lmcurve( int n_par, double *par, int m_dat, 
              const double *t, const double *y,
              double (*f)( double t, const double *par ),
              const lm_control_struct *control,
              lm_status_struct *status )
{
    lmcurve_data_struct data;
    data.t = t;
    data.y = y;
    data.f = f;

    lmmin( n_par, par, m_dat, (const void*) &data,
           lmcurve_evaluate, control, status );
}

#if 0

/* Example: Fit a data set y(x) by a curve f(x;p) */

#include <stdio.h>

/* model function: a parabola */
double f( double t, const double *p ) {
    return p[0] + p[1]*t + p[2]*t*t;
}

int main() {
    int n = 3; /* number of parameters in model function f */
    double par[3] = { 100, 0, -10 }; /* really bad starting value */
    /* data points: a slightly distorted standard parabola */
    int m = 9;
    int i;
    double t[9] = { -4., -3., -2., -1.,  0., 1.,  2.,  3.,  4. };
    double y[9] = { 16.6, 9.9, 4.4, 1.1, 0., 1.1, 4.2, 9.3, 16.4 };
    lm_control_struct control = lm_control_double;
    lm_status_struct status;
    control.verbosity = 7;
    printf( "Fitting ...\n" );
    lmcurve( n, par, m, t, y, f, &control, &status );
    printf( "Results:\n" );
    printf( "status after %d function evaluations:\n  %s\n",
            status.nfev, lm_infmsg[status.outcome] );
    printf("obtained parameters:\n");
    for ( i = 0; i < n; ++i)
        printf("  par[%i] = %12g\n", i, par[i]);
    printf("obtained norm:\n  %12g\n", status.fnorm );
    printf("fitting data as follows:\n");
    for ( i = 0; i < m; ++i)
        printf( "  t[%2d]=%4g y=%6g fit=%10g residue=%12g\n",
                i, t[i], y[i], f(t[i],par), y[i] - f(t[i],par) );
    return 0;
}

#endif
