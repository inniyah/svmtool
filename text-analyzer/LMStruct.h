/* 
 * Contents:  Declarations of parameter records
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

#ifndef LMSTRUCT_H_BDBBDB9A_650C_11E3_95D0_525400DA3F0D
#define LMSTRUCT_H_BDBBDB9A_650C_11E3_95D0_525400DA3F0D

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Collection of input parameters for fit control. */
typedef struct {
    double ftol;      /* Relative error desired in the sum of squares.
                         Termination occurs when both the actual and
                         predicted relative reductions in the sum of squares
                         are at most ftol. */
    double xtol;      /* Relative error between last two approximations.
                         Termination occurs when the relative error between
                         two consecutive iterates is at most xtol. */
    double gtol;      /* Orthogonality desired between fvec and its derivs.
                         Termination occurs when the cosine of the angle
                         between fvec and any column of the Jacobian is at
                         most gtol in absolute value. */
    double epsilon;   /* Step used to calculate the Jacobian, should be
                         slightly larger than the relative error in the
                         user-supplied functions. */
    double stepbound; /* Used in determining the initial step bound. This
                         bound is set to the product of stepbound and the
                         Euclidean norm of diag*x if nonzero, or else to
                         stepbound itself. In most cases stepbound should lie
                         in the interval (0.1,100.0). Generally, the value
                         100.0 is recommended. */
    int patience;     /* Used to set the maximum number of function evaluations
                         to patience*(number_of_parameters+1). */
    int scale_diag;   /* If 1, the variables will be rescaled internally.
                         Recommended value is 1. */
    FILE* msgfile;    /* Progress messages will be written to this file. */
    int verbosity;    /* OR'ed: 1: print some messages; 2: print Jacobian. */
    int n_maxpri;     /* -1, or max number of parameters to print. */
    int m_maxpri;     /* -1, or max number of residuals to print. */
} lm_control_struct;

/* Collection of output parameters for status info. */
typedef struct {
    double fnorm;     /* norm of the residue vector fvec. */
    int nfev;         /* actual number of iterations. */
    int outcome;      /* Status indicator. Nonnegative values are used as index
                         for the message text lm_infmsg, set in lmmin.c. */
    int userbreak;    /* Set when function evaluation requests termination. */
} lm_status_struct;

/* Preset (and recommended) control parameter settings. */
extern const lm_control_struct lm_control_double;
extern const lm_control_struct lm_control_float;

/* Preset message texts. */

extern const char *lm_infmsg[];
extern const char *lm_shortmsg[];

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LMSTRUCT_H_BDBBDB9A_650C_11E3_95D0_525400DA3F0D */
