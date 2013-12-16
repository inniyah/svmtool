/* 
 * Contents:  Declarations for Levenberg-Marquardt minimization.
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

#ifndef LMMIN_H_BDBBDD5C_650C_11E3_95D1_525400DA3F0D
#define LMMIN_H_BDBBDD5C_650C_11E3_95D1_525400DA3F0D

#include "LMStruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
  lmmin() determines a vector par that minimizes the sum of squared elements
  of a vector fvec that is computed by a user-supplied function evaluate().
  On success, par represents a local minimum, not necessarily a global one;
  it may depend on its starting value.
  
  For applications in curve fitting, the wrapper function lmcurve offers a
  simplified API. See LMCurve.h.
  
  The Levenberg-Marquardt minimization starts with a steepest-descent
  exploration of the parameter space, and achieves rapid convergence by
  crossing over into the Newton-Gauss method. For more background,
  see Moré 1978, Madsen et al. 2004, and comments in the source code.
  
  Function arguments:
  
  n_par: Number of free variables. Dimension of parameter vector par.
  par: Parameter vector. On input, it must contain a reasonable guess;
          on output, it contains the solution found to minimize ||fvec||.
  m_dat: Dimension of vector fvec. Must statisfy n_par <= m_dat.
  data: This pointer is ignored by the fit algorithm, except for appearing
          as an argument in all calls to the user-supplied routines evaluate
          and printout.
  evaluate: Pointer to a user-supplied function that computes m_dat elements
          of vector fvec for a given parameter vector par. If evaluate return
          with *userbreak set to a negative value, lmmin() will interrupt the
          fitting and terminate.
  printout: A function that prints messages about the fit progress. Legitimate
          values of this pointer argument are NULL (to indicate that no messages
          are wanted), &lm_printout_std (pointer to a default implementation),
          or a pointer to a user-supplied function.
  control: Parameter collection for tuning the fit procedure. In most cases, the
          default &lm_control_double is adequate. If f is only computed with
          single-precision accuracy, &lm_control_float should be used. See below.
  status: A record used to return information about the minimization process.
  
  
  control has the following members (for more details, see the source file
  LMStruct.h):
  
  double control.ftol: Relative error desired in the sum of squares; recommended
          setting: somewhat above machine precision; less if fvec is computed
          with reduced accuracy.
  double control.xtol: Relative error between last two approximations;
          recommended setting: as ftol.
  double control.gtol: A measure for degeneracy; recommended setting: as ftol.
  double control.epsilon: Step used to calculate the Jacobian; recommended
          setting: as ftol, but definitely less than the accuracy of fvec.
  double control.stepbound: Initial bound to steps in the outer loop, generally
          between 0.01 and 100; recommended value is 100.
  int control.patience: Used to set the maximum number of function evaluations
          to patience*n_par.
  int control.scale_diag: Logical switch (0 or 1). If 1, then scale parameters
          to their initial value. This is the recommended setting.
  FILE* control.msgfile: Progress messages will be written to this file. Typically
          stdout or stderr. The value NULL will be interpreted as stdout.
  int control.verbosity: If nonzero, some progress information from within the LM
          algorithm is written to control.stream.
  int control.n_maxpri: -1, or maximum number of parameters to print.
  int control.m_maxpri: -1, or maximum number of residuals to print.
  
  
  status has the following members:
  
  double status.fnorm: Norm of the vector fvec;
  int status.nfev: Actual number of iterations;
  int status.outcome: Status of minimization; for the corresponding text message,
          print lm_infmsg[status.outcome];
          for a short code, print lm_shortmsg[status.outcome].
  int status.userbreak: Set when termination has been forded by the user supplied
          routine evaluate.
  
  
  Initializing parameter records: The parameter record control should always be
  initialized from supplied default records:
          lm_control_struct control = lm_control_double; // or _float
  After this, parameters may be overwritten:
          control.patience = 500; // allow more iterations
          control.verbosity = 15; // for verbose monitoring
  An application written this way is guaranteed to work even if new parameters are
  added to lm_control_struct. Conversely, addition of parameters is not considered
  an API change; it may happen without increment of the major version number.
*/

/* Levenberg-Marquardt minimization. */
void lmmin( int n_par, double *par, int m_dat, const void *data, 
            void (*evaluate) (const double *par, int m_dat, const void *data,
                              double *fvec, int *userbreak),
            const lm_control_struct *control, lm_status_struct *status );
/*
 *   This routine contains the core algorithm of our library.
 *
 *   It minimizes the sum of the squares of m nonlinear functions
 *   in n variables by a modified Levenberg-Marquardt algorithm.
 *   The function evaluation is done by the user-provided routine 'evaluate'.
 *   The Jacobian is then calculated by a forward-difference approximation.
 *
 *   Parameters:
 *
 *      n is the number of variables (INPUT, positive integer).
 *
 *      x is the solution vector (INPUT/OUTPUT, array of length n).
 *        On input it must be set to an estimated solution.
 *        On output it yields the final estimate of the solution.
 *
 *      m is the number of functions to be minimized (INPUT, positive integer).
 *        It must fulfill m>=n.
 *
 *      data is a pointer that is ignored by lmmin; it is however forwarded
 *        to the user-supplied functions evaluate and printout.
 *        In a typical application, it contains experimental data to be fitted.
 *
 *      evaluate is a user-supplied function that calculates the m functions.
 *        Parameters:
 *          n, x, m, data as above.
 *          fvec is an array of length m; on OUTPUT, it must contain the
 *            m function values for the parameter vector x.
 *          userbreak is an integer pointer. When *userbreak is set to a 
 *            nonzero value, lmmin will terminate.
 *
 *      control contains INPUT variables that control the fit algorithm,
 *        as declared and explained in lmstruct.h
 *
 *      status contains OUTPUT variables that inform about the fit result,
 *        as declared and explained in lmstruct.h
 */

/* Refined calculation of Eucledian norm. */
double lm_enorm( int, const double * );

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LMMIN_H_BDBBDD5C_650C_11E3_95D1_525400DA3F0D */
