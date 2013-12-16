/* 
 * Contents:  Declarations for Levenberg-Marquardt curve fitting
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

#ifndef LMCURVE_H_BDBBDF78_650C_11E3_95D2_525400DA3F0D
#define LMCURVE_H_BDBBDF78_650C_11E3_95D2_525400DA3F0D

#include "LMStruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
  lmcurve() wraps the more generic minimization function lmmin(), for use
  in curve fitting.
  
  lmcurve() determines a vector par that minimizes the sum of squared elements
  of a residue vector r[i] := y[i] - f(t[i];par). Typically, lmcurve() is used
  to approximate a data set t,y by a parametric function f(ti;par). On success,
  par represents a local minimum, not necessarily a global one; it may depend
  on its starting value.
  
  Function arguments:
  
  n_par: Number of free variables. Dimension of parameter vector par.
  par: Parameter vector. On input, it must contain a reasonable guess;
          on output, it contains the solution found to minimize ||r||.
  m_dat: Number of data points. Dimension of vectors t and y. Must statisfy
          n_par <= m_dat.
  t: Array of length m_dat. Contains the abcissae (time, or "x") for which
          function f will be evaluated.
  y: Array of length m_dat. Contains the ordinate values that shall be fitted.
  f: A user-supplied parametric function f(ti;par).
  control: Parameter collection for tuning the fit procedure. In most cases,
          the default &lm_control_double is adequate. If f is only computed
          with single-precision accuracy, &lm_control_float should be used.
          Parameters are explained in LMMin.h.
  princon: Parameter collection for tuning monitoring messages. For values
          NULL or &lm_princon_def, the fit is silent. Parameters are explained
          in LMMin.h.
  status: A record used to return information about the minimization process.
          For details, see LMMin.h.
*/

void lmcurve( int n_par, double *par, int m_dat,
              const double *t, const double *y,
              double (*f)( double t, const double *par ),
              const lm_control_struct *control,
              lm_status_struct *status );

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LMCURVE_H_BDBBDF78_650C_11E3_95D2_525400DA3F0D */
