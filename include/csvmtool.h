/*
 * Copyright (C) 2013, Miriam Ruiz <miriam@debian.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef CSVMTOOL_H_B6732B2E_ECEC_11E2_9D64_525400DA3F0D
#define CSVMTOOL_H_B6732B2E_ECEC_11E2_9D64_525400DA3F0D

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    /* Nothing to see here */
  } SVMToolResult;

  extern int SVMToolInsertSentence(const char * szSentence);

  extern SVMToolResult * SVMToolTaggerRun(const char * szSentence, int iNumWords);

  extern int SVMToolResultPushWord(SVMToolResult * result, char * text, int iPos);
  extern char * SVMToolResultGetWord(SVMToolResult * result, int iPos);

  extern int SVMToolResultPushPOS(SVMToolResult * result, char * text, int iPos);
  extern char * SVMToolResultGetPOS(SVMToolResult * result, int iPos);

  extern int SVMToolResultPushScores(SVMToolResult * result, char * text, int iPos);
  extern char * SVMToolResultGetScores(SVMToolResult * result, int iPos);

  extern void SVMToolResultPrint(SVMToolResult * result);

  extern void SVMToolResultDelete(SVMToolResult * result);

  extern int SVMToolTaggerCreate( char * szModelName );

  extern int SVMToolTaggerInitialize (
    int          iStrategy,
    const char * szSense,
    int          iWinLength,
    int          iWinIndex,
    float        fWFKnown,
    float        fWFUnk
  );

  extern void SVMToolTaggerDestroy();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSVMTOOL_H_B6732B2E_ECEC_11E2_9D64_525400DA3F0D
