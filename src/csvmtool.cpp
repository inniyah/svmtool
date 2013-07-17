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

#include "hash.h"
#include "list.h"
#include "dict.h"
#include "weight.h"
#include "swindow.h"
#include "tagger.h"
#include "regexp.h"

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace SVMTool {

  extern int verbose;

  class SVMTool_ResultNode {
    public:
      SVMTool_ResultNode() {
        this->pos = NULL;
        this->wrd = NULL;
        this->scores = NULL;
      }

      ~SVMTool_ResultNode() {
        delete[] this->wrd;
        delete[] this->pos;
        delete[] this->scores;
        index = 0;
      }

      inline void pushIndex(int iNum) {
        this->index = iNum;
      }
      inline void pushPOS(char * text) {
        this->pos = new char[strlen(text)+1];
        strcpy(this->pos,text);
      }
      inline void pushWord(char * text) {
        this->wrd = new char[strlen(text)+1];
        strcpy(this->wrd,text);
      }
      inline void pushScores(char * text) {
        this->scores = new char[strlen(text)+1];
        strcpy(this->scores,text);
      }
      inline int getIndex() {
        return this->index;
      }
      inline char * getWord() {
        return this->wrd;
      }
      inline char * getPOS() {
        return this->pos;
      }
      inline char * getScores() {
        return this->scores;
      }

    private:
      int index;
      char *wrd;
      char *pos;
      char *scores;
  };

  /*****************************************************************/

  class SVMTool_Result {
    public:
      SVMTool_Result(int iNum) {
        numItems = iNum;
        array = new SVMTool_ResultNode[iNum];
      }
      ~SVMTool_Result() {
        delete[] array;
      }

      inline char * getWord(int iPos) {
        return array[iPos].getWord();
      }
      inline char * getPOS(int iPos) {
        return array[iPos].getPOS();
      }
      inline char * getScores(int iPos) {
        return array[iPos].getScores();
      }

      inline int pushWord(char *text, int iPos) {
        if (iPos >= this->numItems) return -1;
        array[iPos].pushWord(text);
        return 0;
      }
      inline int pushPOS(char *text, int iPos) {
        if (iPos >= this->numItems) return -1;
        array[iPos].pushPOS(text);
        return 0;
      }
      inline int pushScores(char *text, int iPos) {
        if (iPos >= this->numItems) return -1;
        array[iPos].pushScores(text);
        return 0;
      }

      inline void print() {
        for (int i=0; i < this->numItems; i++) {
          fprintf(stdout, "%s %s %s\n",this->getWord(i),array[i].getPOS(),array[i].getScores());
        }
      }

    private:
      SVMTool_ResultNode * array;
      int numItems;
  };

  /*****************************************************************/


} // namespace SVMTool



extern "C" {

  int SVMToolInsertSentence(SVMToolTagger * pTagger, const char * szSentence) {
    SVMTool::tagger * t = (SVMTool::tagger *)pTagger;

    char wrd[200];
    FILE *f = fopen ("stdin.tmp","w");
    unsigned int i = 0;

    if ( t == NULL ) return -1;

    for (i=0; szSentence[i] !='\0' && i < strlen(szSentence); i++) {
      strcpy(wrd,"");
      int ret = sscanf(szSentence+i,"%s ",wrd);

      if (ret > 0) {
        fprintf(f,"%s\n",wrd);
        i = i + strlen(wrd);
      }
      else break;

    }

    fclose(f);
    return i;
  }

  SVMToolResult * SVMToolRunTagger(SVMToolTagger * pTagger, const char * szSentence, int iNumWords) {
    SVMTool::tagger * t = (SVMTool::tagger *)pTagger;

    if ( t == NULL ) return NULL;

    int ret = SVMToolInsertSentence (pTagger, szSentence);

    if ( ret == -1 ) return NULL;

    char tmp1[500],tmp2[500], tmp3[2000];
    char aux[3000];

    fflush(stdin);
    std::ifstream in("stdin.tmp");
    std::ofstream fout("stdout.tmp");

    t->taggerShowNoComments();
    t->taggerActiveShowScoresFlag();

    t->taggerInit(in,fout);
    t->taggerRun();
    fout.close();

    FILE *f = fopen("stdout.tmp","r");

    int index = 0;
    SVMTool::SVMTool_Result *out = new SVMTool::SVMTool_Result(iNumWords);

    while (!feof(f) && index < iNumWords) {
      strcpy(aux,"");
      strcpy(tmp1,""); strcpy(tmp2,""); strcpy(tmp3,"");
      char *ret = fgets(aux,3000,f);
      if (ret != NULL ) {
        sscanf(aux,"%s %s",tmp1, tmp2);
        if ((strlen(tmp1)+strlen(tmp2)+2) < strlen(aux)) {
          strcpy(tmp3,aux+strlen(tmp1)+1+strlen(tmp2)+1);
          tmp3[strlen(tmp3)-1] = '\0';
        }
        out->pushWord(tmp1, index);
        out->pushPOS (tmp2, index);
        out->pushScores (tmp3,index);
        index++;
      }

    }

    remove ( "stdin.tmp" );
    remove ( "stdout.tmp" );
    return (SVMToolResult *)out;
  }

  SVMToolTagger * SVMToolCreateTagger(char * szModelName) {
    if ( strcmp(szModelName,"") == 0 ) return NULL;
    SVMTool::tagger * t = new SVMTool::tagger(szModelName);
    return (SVMToolTagger *)t;
  }


  int SVMToolResultPushWord(SVMToolResult * result, char * text, int iPos) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
    return res->pushWord(text, iPos);
  }

  char * SVMToolResultGetWord(SVMToolResult * result, int iPos) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
    return res->getWord(iPos);
  }

  int SVMToolResultPushPOS(SVMToolResult * result, char * text, int iPos) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
    return res->pushPOS(text, iPos);
  }

  char * SVMToolResultGetPOS(SVMToolResult * result, int iPos) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
    return res->getPOS(iPos);
  }

  int SVMToolResultPushScores(SVMToolResult * result, char * text, int iPos) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
    return res->pushScores(text, iPos);
  }

  char * SVMToolResultGetScores(SVMToolResult * result, int iPos) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
    return res->getScores(iPos);
  }

  void SVMToolResultPrint(SVMToolResult * result) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
    res->print();
  }

  extern void SVMToolResultDelete(SVMToolResult * result) {
    SVMTool::SVMTool_Result * res = (SVMTool::SVMTool_Result *) result;
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

    if ( t == NULL ) return -1;
    SVMTool::verbose = 0;
    SVMTool::erCompRegExp();

    if ( fWFKnown != -1 )
      t->taggerPutKWeightFilter(fWFKnown);
    if ( fWFUnk != -1 )
      t->taggerPutUWeightFilter(fWFUnk);
    if ( iStrategy != -1 )
      t->taggerPutStrategy(iStrategy);
    if ( strcmp (szSense,"") == 0 )
      t->taggerPutFlow(szSense);
    if ( iWinLength != -1 )
      t->taggerPutWinLength(iWinLength);
    if ( iWinIndex != -1 )
      t->taggerPutWinIndex(iWinIndex);

    t->taggerLoadModelsForTagging();
    return 0;
  }

  void SVMToolDestroyTagger(SVMToolTagger * pTagger) {
    SVMTool::tagger * t = (SVMTool::tagger *)pTagger;
    SVMTool::erFreeRegExp();
    if ( t != NULL ) delete t;
  }

}
