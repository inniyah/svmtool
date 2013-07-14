/*
 * Copyright (C) 2004 Jesus Gimenez, Lluis Marquez and Senen Moya
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

#ifndef SVMTOOL_H_7CC43D0A_EC7E_11E2_8F37_525400DA3F0D
#define SVMTOOL_H_7CC43D0A_EC7E_11E2_8F37_525400DA3F0D

namespace SVMTool {

  class ResultNode {
    private:
      int index;
      char *wrd;
      char *pos;
      char *scores;

    public:
      void  pushIndex(int);
      void  pushPOS(char *);
      void  pushWord(char *);
      void  pushScores(char *);
      int   getIndex();
      char *getWord();
      char *getPOS();
      char *getScores();

      ResultNode();
      ~ResultNode();
  };

  class Result {
    private:
      ResultNode *array;
      int numItems;

    public:
      Result(int);
      ~Result();
      char *getPOS(int);
      char *getWord(int);
      char *getScores(int);
      int  pushWord(char *, int);
      int  pushPOS(char *, int);
      int  pushScores(char *, int);
      void print();
  };

  int InsertSentence(const char *szSentence);

  Result *TaggerRun(const char *szSentence, int iNumWords);

  int TaggerCreate( char *szModelName );

  int TaggerInitialize (
    int   iStrategy,
    const char  *szSense,
    int   iWinLength,
    int   iWinIndex,
    float fWFKnown,
    float fWFUnk
  );

  void TaggerDestroy();

} // namespace SVMTool

#endif // SVMTOOL_H_7CC43D0A_EC7E_11E2_8F37_525400DA3F0D
