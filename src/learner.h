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
 

#ifndef LEARNER_H

struct samples_counter_t
{
    char *key;
    int positive;
    int negative;
};

class learner
{
private:
	int learnerNumAMBP,learnerNumUNKP;
	hash_t *learnerAMBP_H,*learnerUNKP_H;
	swindow *sw;
	simpleList learnerFeatureList,*learnerAMBP_L,*learnerUNKP_L;
	FILE *fKnown,*fUnknown;
	int learnerNumFeatures;
	char obtainAtrChar(FILE *channel);
	int  obtainAtrInt(FILE *channel,int *endAtr);
	void learnerCreateFeatureList(char *name, simpleList *featureList);
	simpleList *learnerTransformHashInList(hash_t *tptr);
	void learnerCreateDefaultFile(const char *modelName, const char *str);
	void learnerCreatePOSFile(char *modelName, int is_ambp, hash_t *h);
	void learnerCount(char *name, int *sentences, int *words);
	int learnerExecSVMlight(char *svmdir, char *options, char *posFile, char *outFile);
	int learnerLeftToRight(simpleList *, simpleList *, dictionary *dKnown, dictionary *dUnknown,/*mapping *mKnown, mapping *mUnknown,*/int numWrds, int inicio);
	int learnerRightToLeft(simpleList *,simpleList *,   dictionary *dKnown, dictionary *dUnknown,/*mapping *mKnown, mapping *mUnknown,*/int numWrds, int inicio);
	void learnerGetFeatures(nodo *elem, stack_t *stk,dictionary *d, simpleList *featureList, int direction);
	void learnerGenerateFeatures(nodo *elem,simpleList *featureList, dictionary *d, int direction);
	void learnerGenerateFeaturesUnk(nodo *elem,simpleList *featureList, dictionary *d, dictionary *dUnk, int direction);
	void learnerTrainModel(char *trainingFileName,dictionary *d, int numModel,int direction,  int numSent, int numWords, int numChunks);
 	weightRepository *learnerBuiltWeightRepository(weightRepository *wr,mapping *m,char *pos,char *fileName);
	hash_t *learnerBuiltBias(hash_t *,char *pos,char *fileName);
	void learnerDestroyBias(hash_t *h);
   	int learnerIsPossiblePOS(char *wrd, char *pos, int Known_or_Unknown);
  	simpleList *learnerGetPotser(char *wrd, int Known_or_Unknown, dictionary *d);
  	void learnerTraining(FILE *f,char *modelName, int numModel,int LR_or_RL,int K_or_U,dictionary *d,simpleList *lpos);
   	void learnerPrintMessage(int numModel, int K_or_U, int LR_or_RL,int is_fex);
  	int  learnerNumChunks(char *trainingFileName,float percentage,int nSentences);
  	int learnerIsInsideList(simpleList *l, char *key);
  	void learnerDoLearn(FILE *f,int numModel,int LR_or_RL,int K_or_U,dictionary *d,simpleList *lPosToTrain);
  	void learnerDressNakedSetTrain(dictionary *d,mapping *m,FILE *f, char* pos, int numModel, int direction, int K_or_U,int *nPos,int *nNeg);
  	void learnerPushSample(char *wrd,int numModel,int direction, int Known_or_Unknown, char *pos,char *samplePos,char *features,dictionary *d, int *nNeg, int *nPos);
  	void learnerPushSampleUnk(char *wrd,int numModel,int direction, int Known_or_Unknown,char *pos, char *samplePos, char *features,dictionary *d, int *nNeg, int *nPos);
  	char *learnerCreateFeatureString(FILE *f,mapping *m);

  	char *read_feature_list_from_config_file(FILE *f, char *first_feature);
  	void read_config_file(const char *config_file);

public:
	learner(char *modelName);
	learner();
	~learner();
	void learnerRun(char *train);
};

#define LEARNER_H
#endif
