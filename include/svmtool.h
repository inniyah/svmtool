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

  class ResultNode;
  class tagger;

  class Result {
    public:
      Result(int);
      ~Result();

      char * getWord(int iPos);
      char * getPOS(int iPos);
      char * getScores(int iPos);

      int    pushWord(char * text, int iPos);
      int    pushPOS(char * text, int iPos);
      int    pushScores(char * text, int iPos);

      void   print();

    private:
      ResultNode * array;
      int numItems;
  };

  int InsertSentence(tagger * t, const char * szSentence);

  Result * RunTagger(tagger * t, const char * szSentence, int iNumWords);

  tagger * CreateTagger(char * szModelName);

  int InitializeTagger (
    tagger     * t,
    int          iStrategy,
    const char * szSense,
    int          iWinLength,
    int          iWinIndex,
    float        fWFKnown,
    float        fWFUnk
  );

  void DestroyTagger(tagger * t);

} // namespace SVMTool

#endif // SVMTOOL_H_7CC43D0A_EC7E_11E2_8F37_525400DA3F0D
