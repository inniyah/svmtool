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

#ifndef TAGGER_H

struct models_t{
    weightRepository *wr,*wr2,*wrUnk,*wrUnk2;
    //hash_t *bias,*bias2,*biasUnk,*biasUnk2;
    simpleList featureList;
    simpleList featureListUnk;
};


class tagger
{
 private:
    //Flow Control
    char flow[10];
    int  taggerStrategy,taggerNumLaps,taggerWinIndex,taggerWinLength;
    float  taggerKFilter,taggerUFilter;
    char taggerBackupDict[150],taggerModelName[150];

    struct stack_t *stk;
    models_t  *taggerModelList;
    models_t  *taggerModelRunning;
    dictionary *d;
    swindow *sw;
    weight_node_t *weightUnk;

    int taggerRightSense();
    int taggerLeftSense();

    void taggerSumWeight(weightRepository *wRep,hash_t *bias,weight_node_t *weight,int numMaybe, int *max);
    void taggerGenerateScore(nodo *elem,int direction);

    weight_node_t *taggerCreateWeightNodeArray(int numMaybe,int index);
    weight_node_t *taggerInitializeWeightNodeArray(int numMaybe,weight_node_t *w);
    weight_node_t *taggerCreateWeightUnkArray(char *name);
    hash_t *taggerCreateBiasHash(char *name);
    void taggerLoadModels(models_t *model, int taggerNumModel);

    void taggerStadistics(int numWords, int numSentences, double realTime,double usrTime, double sysTime);
    void taggerShowVerbose(int num,int isEnd);


  int taggerRightSenseSpecialForUnknown();
  int taggerLeftSenseSpecialForUnknown();
  void taggerDoNormal(int *numWords, int *numSentences);
  void taggerDoSpecialForUnknown(int *numWords, int *numSentences);
  void taggerDoNTimes(int *numWords, int *numSentences,int laps);

 public:
    void taggerRun();
    void taggerLoadModelsForTagging();
    void taggerPutFlow(char *inFlow);
    void taggerPutBackupDictionary(char *dictName);
    void taggerPutStrategy(int num);
    void taggerPutWinLength(int l);
    void taggerPutWinIndex(int i);
    void taggerPutKWeightFilter(float kfilter);
    void taggerPutUWeightFilter(float ufilter);
    void taggerInit();
    tagger(char *model);
    ~tagger();
};


#define TAGGER_H
#endif
