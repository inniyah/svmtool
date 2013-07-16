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

#include "csvmtool.h"

#include "svmtool.h"

extern "C" {

  int SVMToolInsertSentence(SVMToolTagger * pTagger, const char * szSentence) {
    SVMTool::tagger * t = (SVMTool::tagger *)pTagger;
    return SVMTool::InsertSentence(t, szSentence);
  }

  SVMToolResult * SVMToolRunTagger(SVMToolTagger * pTagger, const char * szSentence, int iNumWords) {
    SVMTool::tagger * t = (SVMTool::tagger *)pTagger;
    SVMTool::Result * res = SVMTool::RunTagger(t, szSentence, iNumWords);
    return (SVMToolResult *) res;
  }

  SVMToolTagger * SVMToolCreateTagger(char * szModelName) {
    return (SVMToolTagger *)SVMTool::CreateTagger(szModelName);
  }


  int SVMToolResultPushWord(SVMToolResult * result, char * text, int iPos) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    return res->pushWord(text, iPos);
  }

  char * SVMToolResultGetWord(SVMToolResult * result, int iPos) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    return res->getWord(iPos);
  }

  int SVMToolResultPushPOS(SVMToolResult * result, char * text, int iPos) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    return res->pushPOS(text, iPos);
  }

  char * SVMToolResultGetPOS(SVMToolResult * result, int iPos) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    return res->getPOS(iPos);
  }

  int SVMToolResultPushScores(SVMToolResult * result, char * text, int iPos) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    return res->pushScores(text, iPos);
  }

  char * SVMToolResultGetScores(SVMToolResult * result, int iPos) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    return res->getScores(iPos);
  }

  void SVMToolResultPrint(SVMToolResult * result) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    res->print();
  }

  extern void SVMToolResultDelete(SVMToolResult * result) {
    SVMTool::Result * res = (SVMTool::Result *) result;
    delete res;
  }


  int SVMToolInitializeTagger (
    SVMToolTagger * pTagger,
    int             iStrategy,
    const char    * szSense,
    int             iWinLength,
    int             iWinIndex,
    float           fWFKnown,
    float           fWFUnk
  ) {
    SVMTool::tagger * t = (SVMTool::tagger *)pTagger;
    return SVMTool::InitializeTagger (t, iStrategy, szSense, iWinLength, iWinIndex, fWFKnown, fWFUnk);
  }

  void SVMToolDestroyTagger(SVMToolTagger * pTagger) {
    SVMTool::tagger * t = (SVMTool::tagger *)pTagger;
    SVMTool::DestroyTagger(t);
  }

}
