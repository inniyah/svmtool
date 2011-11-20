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

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <string.h>
#include "marks.h"
#include "strategies.h"
#include "hash.h"
#include "list.h"
#include "weight.h"
#include "dict.h"
#include "stack.h"
#include "swindow.h"
#include "tagger.h"
#include "common.h"

/***************************************************************/

struct tms  tbuffStartUp,tbuffEndStartUp;
clock_t startUpTime,endStartUpTime;
double  sysFexTime=0, usrFexTime=0,realFexTime=0;
double  sysSVMTime=0, usrSVMTime=0,realSVMTime=0;

/***************************************************************/

extern int verbose;
int NUM_UNK_POS=0;


/***************************************************************/

hash_t *tagger::taggerCreateBiasHash(char *name)
{
  hash_t *bias = new hash_t;
  int i=0;
  char c=' ',weight[20];
  weight_node_t *w;
  FILE *f;

  hash_init(bias,40);

  if ((f = fopen(name, "rt"))== NULL)
    {
      fprintf(stderr, "Error opening file: %s",name);
      exit(0);
    }

  while (!feof(f))
    {
      c = fgetc(f);
      if (c!='#') 
	{  
	  w = new weight_node_t;
	  strcpy(weight,"");
	  i=0;
	  strcpy(w->pos,"");
	  while ((c!='\n') && (!feof(f)))
	    {
	      if (c!=' ' && c!='\n' && c!='\t' && i==1)  sprintf(weight,"%s%c",weight,c);
	      else if (c!=' ' && c!='\n' && c!='\t' && i==0)
		{
		  if (c!=':') sprintf(w->pos,"%s%c",w->pos,c);
		  else i=1;
		}
	      c = fgetc(f);
	    }
	  w->data = (long double)0;
	  w->data = atof (weight);
	  hash_insert(bias,w->pos,(uintptr_t)w);
	} //end if
      else while(c=fgetc(f)!='\n');
    }
  fclose(f);
  return bias;
}

/***************************************************************/

tagger::tagger(char *model)
{
  char name[150]="";
  strcpy(flow,"LR");
  taggerNumLaps = 1;
  taggerKFilter = 0;
  taggerUFilter = 0;
  taggerStrategy = STRA_1P_DEFAULT; //modstrat //0;
  taggerWinIndex = -1;
  taggerWinLength = -1;
  strcpy(taggerModelName,model);
  strcpy (taggerBackupDict,"");
  stk = new stack_t;
  init_stack(stk);
}

/***************************************************************/

void tagger::taggerLoadModels(models_t *model, int taggerNumModel)
{
  char name[150],flow2[5],flow1[5];

  //Cargamos la lista de "features" para palabras conocidas
  sprintf(name,"%s.A%d",taggerModelName,taggerNumModel);
  if (verbose)  fprintf(stderr,"\nLoading FEATURES FOR KNOWN WORDS from < %s >\n",name);
  createFeatureList(name,&model->featureList);
  //Cargamos la lista de "features" para palabras desconocidas
  sprintf(name,"%s.A%d.UNK",taggerModelName,taggerNumModel);
  if (verbose)  fprintf(stderr,"\nLoading FEATURES FOR UNKNOWN WORDS from < %s >\n",name);
  createFeatureList(name,&model->featureListUnk);

  if (strcmp(flow,"LRL")==0)
    {
      strcpy(flow1,"LR");	strcpy(flow2,"RL");

      sprintf(name,"%s (Right-to-Left)",flow1);
      if (verbose) fprintf(stderr,"\nREADING MODELS < direction =  %s >\n",name);
  
      sprintf(name,"%s.M%d.%s.MRG",taggerModelName,taggerNumModel,flow2);
      if (verbose) fprintf(stderr,"-. Loading MERGED MODEL FOR KNOWN WORDS from < %s >\n",name);
      model->wr2 = new weightRepository(name,taggerKFilter);

      sprintf(name,"%s.UNK.M%d.%s.MRG",taggerModelName,taggerNumModel,flow2);
      if (verbose) fprintf(stderr,"-. Loading MERGED MODEL FOR UNKKNOWN WORDS from < %s >\n\n",name);
      model->wrUnk2 = new weightRepository(name,taggerUFilter);
    }
  else strcpy(flow1,flow);

  if (strcmp(flow1,"RL")==0) sprintf(name,"%s (Right-to-Left)",flow1);
  else sprintf(name,"%s (Left-to-Right)",flow1);

  if (verbose) fprintf(stderr,"\nREADING MODELS < direction =  %s >\n",name);

  sprintf(name,"%s.M%d.%s.MRG",taggerModelName,taggerNumModel,flow1);
  if (verbose) fprintf(stderr,"-. Loading MERGED MODEL FOR KNOWN WORDS from < %s >\n",name);
  model->wr = new weightRepository(name,taggerKFilter);

  sprintf(name,"%s.UNK.M%d.%s.MRG",taggerModelName,taggerNumModel,flow1);
  if (verbose) fprintf(stderr,"-. Loading MERGED MODEL FOR UNKNOWN WORDS from < %s >\n",name);
  model->wrUnk = new weightRepository(name,taggerUFilter);

}

/***************************************************************/

void tagger::taggerLoadModelsForTagging()
{
  startUpTime = times(&tbuffStartUp);

  int modelsNeeded=1;
  char name[150];

  sprintf(name,"%s.DICT",taggerModelName);
  if (strcmp(taggerBackupDict,"")!=0)
    {
      if (verbose) fprintf(stderr,"Loading DICTIONARY from < %s > with BACKUP DICTIONARY from < %s >\n",name,taggerBackupDict);
      d  = new dictionary(name,taggerBackupDict);
    }
  else
    {
      if (verbose) fprintf(stderr,"Loading DICTIONARY from < %s >\n",name);
      d  = new dictionary(name);
    }

  sprintf(name,"%s.UNKP",taggerModelName);
  if (verbose) fprintf(stderr,"Loading UNKNOWN WORDS POS from < %s >\n",name);
  weightUnk = taggerCreateWeightUnkArray(name);

  if ( taggerStrategy == STRA_2P_RELABELING //modstrat 1 
       || taggerStrategy == STRA_1P_ROBUST_UNK /*modstrat 4*/ )  modelsNeeded = 2;

  taggerModelList = new models_t[modelsNeeded];
  taggerModelRunning = &taggerModelList[0];

  if (taggerStrategy == STRA_1P_DEFAULT ) //modstrat 0)  
  	taggerLoadModels(taggerModelRunning,0);
  else if (taggerStrategy == STRA_1P_UNSUPERVISED ) //modstrat 2)  
  	taggerLoadModels(taggerModelRunning,3);
  else if (taggerStrategy ==  STRA_1P_ROBUST_UNK ) //modstrat 4)
    { 
      	taggerLoadModels(taggerModelRunning,0);
      	taggerLoadModels(&taggerModelList[1],2);
    }
  else if (taggerStrategy == STRA_1P_VERY_ROBUST_UNK ) //modstrat 5)  
  	taggerLoadModels(taggerModelRunning,4);
	   else  if (taggerStrategy == STRA_2P_RELABELING )//modstrat 1)
    { 
      	taggerLoadModels(taggerModelRunning,2);
      	taggerLoadModels(&taggerModelList[1],1);
      	taggerNumLaps = 2;
    }
  else
    {  
      fprintf(stderr,"Execution error: Strategy %d doesn't exist!!\n\n",taggerStrategy);
      exit(0);
    }

  endStartUpTime = times(&tbuffEndStartUp);
}

void tagger::taggerInit()
{

  //  int modelsNeeded=1;
  char name[150];

  //Mirar si existe fichero .WIN  
  if (taggerWinIndex==-1 && taggerWinLength==-1)
    {
      sprintf(name,"%s.WIN",taggerModelName);
      FILE *f =  fopen (name,"r");
      if ( f == NULL ) sw = new swindow(stdin);
      else 
	{
	  fscanf(f,"%d %d",&taggerWinLength,&taggerWinIndex);
	  fclose(f);
	  sw = new swindow (stdin,taggerWinLength,taggerWinIndex);
	}
    }
  else if (taggerWinIndex==-1) sw = new swindow (stdin,taggerWinLength);
  else sw = new swindow (stdin,taggerWinLength,taggerWinIndex);
}


/***************************************************************/

tagger::~tagger()
{
  int modelsNeeded=1;

  if (taggerStrategy == STRA_2P_RELABELING ) //modstrat 1)  
    modelsNeeded = 2;

  delete stk;
  delete d;
  delete sw;
  delete[] weightUnk; //Mod

  for (int i=0;i<modelsNeeded;i++)
    {

      delete taggerModelList[i].wr;
      delete taggerModelList[i].wrUnk;
      
      if (strcmp(flow,"LRL")==0)
	{
	  delete  taggerModelList[i].wr2;
	  delete  taggerModelList[i].wrUnk2;
	}
    }
}

/***************************************************************/
/***************************************************************/

void tagger::taggerPutFlow(char *inFlow)
{
  strcpy(flow,inFlow);
}

/***************************************************************/

void tagger::taggerPutStrategy(int num)
{
   taggerStrategy = num;
}

/***************************************************************/

void tagger::taggerPutWinLength(int l)
{
  taggerWinLength = l;
}

/***************************************************************/

void tagger::taggerPutWinIndex(int i)
{
  taggerWinIndex = i;
}

/***************************************************************/

void tagger::taggerPutBackupDictionary(char *dictName)
{
  strcpy (taggerBackupDict,dictName);
}

/***************************************************************/

void tagger::taggerPutKWeightFilter(float kfilter)
{
  taggerKFilter = kfilter;
}

/***************************************************************/

void tagger::taggerPutUWeightFilter(float ufilter)
{
  taggerUFilter = ufilter;
}

/***************************************************************/
/***************************************************************/
/***************************************************************/
/***************************************************************/

int tagger::taggerRightSenseSpecialForUnknown()
{
  int cont=1;

  while(sw->previous()==0);
  nodo *elem = sw->getIndex();

  if (sw->winExistUnkWord(1,d)==-1)
    taggerModelRunning=&taggerModelList[1];
  else taggerModelRunning=&taggerModelList[0];

  taggerGenerateScore(elem,1);

  while(sw->next()==0)
    {
      elem = sw->getIndex();

      if (sw->winExistUnkWord(1,d)==-1)
	taggerModelRunning=&taggerModelList[1];
      else taggerModelRunning=&taggerModelList[0];

      taggerGenerateScore(elem,1);
      cont++;
    }

  if (strcmp(flow,"LRL")==0) sw->winMaterializePOSValues(1);

  return cont;
}

/***************************************************************/

int tagger::taggerLeftSenseSpecialForUnknown()
{
  int cont=1;
  while(sw->next()==0);
  nodo *elem = sw->getIndex();
  if (sw->winExistUnkWord(2,d)==-1)
    taggerModelRunning=&taggerModelList[1];
  else taggerModelRunning=&taggerModelList[0];

  taggerGenerateScore(elem,2);

  while(sw->previous()==0)
    {
      elem = sw->getIndex();

      if (sw->winExistUnkWord(2,d)==-1)
	taggerModelRunning=&taggerModelList[1];
      else taggerModelRunning=&taggerModelList[0];

      taggerGenerateScore(elem,2);
      cont++;
    }

  if (strcmp(flow,"LRL")==0) sw->winMaterializePOSValues(0);
  return cont;
}

/***************************************************************/

int tagger::taggerRightSense()
{
  int cont=1;

  while(sw->previous()==0);
  nodo *elem = sw->getIndex();
  taggerGenerateScore(elem,1);

  while(sw->next()==0)
    {
      elem = sw->getIndex();
      taggerGenerateScore(elem,1);
      cont++;
    }

  if (strcmp(flow,"LRL")==0) sw->winMaterializePOSValues(1);

  return cont;
}

/***************************************************************/

int tagger::taggerLeftSense()
{
  int cont=1;
  while(sw->next()==0);
  nodo *elem = sw->getIndex();
  taggerGenerateScore(elem,2);

  while(sw->previous()==0)
    {
      elem = sw->getIndex();
      taggerGenerateScore(elem,2);
      cont++;
    }

  if (strcmp(flow,"LRL")==0) sw->winMaterializePOSValues(0);
  return cont;
}

/***************************************************************/

void tagger::taggerRun()
{
  int contWords=0,contSentences=0;

  struct  tms     tbuff1,tbuff2;
  clock_t start,end;
  start = times(&tbuff1);

  switch(taggerStrategy)
    {
    case STRA_1P_DEFAULT/*modstrat 0*/: taggerDoNormal(&contWords,&contSentences); break;
    case STRA_2P_RELABELING/*modstrat 1*/: taggerDoNTimes(&contWords,&contSentences,taggerNumLaps); break;
    case STRA_1P_UNSUPERVISED/*modstrat 2*/: taggerDoNormal(&contWords,&contSentences); break;
    case STRA_1P_SENTENCE_LEVEL/*modstrat 3*/: /*taggerDoNTimes(&contWords,&contSentences,taggerNumLaps);*/ break;
    case STRA_1P_ROBUST_UNK/*modstrat 4*/: taggerDoSpecialForUnknown(&contWords,&contSentences); break;
    case STRA_1P_VERY_ROBUST_UNK/*modstrat 5*/: taggerDoNormal(&contWords,&contSentences); break;
    case STRA_1P_ROBUST_SENTENCE_LEVEL: break;  
    }
  end = times(&tbuff2);


  if (verbose)
    { taggerShowVerbose(contSentences,1);

    fprintf(stderr,"* -------------------------------------------------------------------\n");    
    showTime("Start Up Time",
	     ((double)(endStartUpTime-startUpTime))/CLOCKS_PER_SECOND, 
	     ((double)tbuffEndStartUp.tms_utime-(double)tbuffStartUp.tms_utime)/CLOCKS_PER_SECOND,
	     ((double)tbuffEndStartUp.tms_stime-(double)tbuffStartUp.tms_stime)/CLOCKS_PER_SECOND);
    fprintf(stderr,"* -------------------------------------------------------------------\n");
    showTime("Features Extraction Time",realFexTime,usrFexTime,sysFexTime);
    showTime("SVM Time",realSVMTime,usrSVMTime,sysSVMTime);
    showTime("Process Time",((double)(end-start))/CLOCKS_PER_SECOND - realFexTime - realSVMTime,
	     ((double)tbuff2.tms_utime-(double)tbuff1.tms_utime)/CLOCKS_PER_SECOND - usrFexTime -usrSVMTime,
	     ((double)tbuff2.tms_stime-(double)tbuff1.tms_stime)/CLOCKS_PER_SECOND - sysFexTime -sysSVMTime);
    fprintf(stderr,"* -------------------------------------------------------------------\n");
    fprintf(stderr,"[ Tagging Time = Feature Extraction Time + SVM Time + Process Time ]\n");
    showTime("Tagging Time",((double)(end-start))/CLOCKS_PER_SECOND, 
	     ((double)tbuff2.tms_utime-(double)tbuff1.tms_utime)/CLOCKS_PER_SECOND,
	     ((double)tbuff2.tms_stime-(double)tbuff1.tms_stime)/CLOCKS_PER_SECOND);
    fprintf(stderr,"* -------------------------------------------------------------------\n");
    fprintf(stderr,"[ Overall Time = Start up Time + Tagging Time ]\n");
    showTime("Overall Time",((double)(end-start+endStartUpTime-startUpTime))/CLOCKS_PER_SECOND,
	     ((double)tbuff2.tms_utime-(double)tbuff1.tms_utime+
	      (double)tbuffEndStartUp.tms_utime-(double)tbuffStartUp.tms_utime)/CLOCKS_PER_SECOND,
	     ((double)tbuff2.tms_stime-(double)tbuff1.tms_stime+
	      (double)tbuffEndStartUp.tms_stime-(double)tbuffStartUp.tms_stime)/CLOCKS_PER_SECOND);
    fprintf(stderr,"* -------------------------------------------------------------------\n");
    taggerStadistics(contWords,contSentences,
		     ((double)(end-start))/CLOCKS_PER_SECOND, 
		     ((double)tbuff2.tms_utime-(double)tbuff1.tms_utime)/CLOCKS_PER_SECOND,
		     ((double)tbuff2.tms_stime-(double)tbuff1.tms_stime)/CLOCKS_PER_SECOND);
    }
}

/***************************************************************/

void tagger::taggerDoNormal(int *numWords, int *numSentences)
{
  int contWordsLR=0,contWordsRL=0,contSentences=0,ret = 1;

  while ((ret>=0))
    {
      if (verbose) taggerShowVerbose(contSentences,0);

      if ((strcmp(flow,"LRL")==0) || (strcmp(flow,"LR")==0))
	contWordsLR = contWordsLR+taggerRightSense();
      if ((strcmp(flow,"LRL")==0) || (strcmp(flow,"RL")==0))
	contWordsRL = contWordsRL+taggerLeftSense();
      contSentences++;
      sw->show();
      sw->deleteList();
      ret = sw->iniGeneric();
    }
  if (contWordsRL==0) *numWords=contWordsLR/taggerNumLaps;
  else *numWords=contWordsRL/taggerNumLaps;
  *numSentences = contSentences;
}

/***************************************************************/

void tagger::taggerDoSpecialForUnknown(int *numWords, int *numSentences)
{
  int contWordsLR=0,contWordsRL=0,contSentences=0,ret = 1;

  while ((ret>=0))
    {
      if (verbose) taggerShowVerbose(contSentences,0);

      if ((strcmp(flow,"LRL")==0) || (strcmp(flow,"LR")==0))
	contWordsLR = contWordsLR+taggerRightSenseSpecialForUnknown();
      if ((strcmp(flow,"LRL")==0) || (strcmp(flow,"RL")==0))
	contWordsRL = contWordsRL+taggerLeftSenseSpecialForUnknown();

      contSentences++;
      sw->show();
      sw->deleteList();
      ret = sw->iniGeneric();
    }
  if (contWordsRL==0) *numWords=contWordsLR/taggerNumLaps;
  else *numWords=contWordsRL/taggerNumLaps;
  *numSentences = contSentences;
}

/***************************************************************/

void tagger::taggerDoNTimes(int *numWords, int *numSentences,int laps)
{
  int contWordsLR=0,contWordsRL=0,contSentences=0,ret = 1;

  while ((ret>=0))
    {

      if (verbose) taggerShowVerbose(contSentences,0);

      for (int pasadas=0;pasadas<laps;pasadas++)
	{

  	  taggerModelRunning = &taggerModelList[pasadas];
	  if ((strcmp(flow,"LRL")==0) || (strcmp(flow,"LR")==0))
	    contWordsLR = contWordsLR+taggerRightSense();
	  
	  if (strcmp(flow,"LRL")==0 && pasadas>0)
	    sw->winMaterializePOSValues(2);
	  if ((strcmp(flow,"LRL")==0) || (strcmp(flow,"RL")==0))
	    contWordsRL = contWordsRL+taggerLeftSense();

	}
   
      contSentences++;
      sw->show();
      sw->deleteList();
      ret = sw->iniGeneric();
    }
  if (contWordsRL==0) *numWords=contWordsLR/taggerNumLaps;
  else *numWords=contWordsRL/taggerNumLaps;
  *numSentences = contSentences;
}

/***************************************************************/
/***************************************************************/

void tagger::taggerGenerateScore(nodo *elem,int direction)
{

  struct  tms tbuffStartFex,tbuffEndFex;
  clock_t startFexTime,endFexTime;
  struct  tms tbuffStartSVM,tbuffEndSVM;
  clock_t startSVMTime,endSVMTime;

  weight_node_t *weight;
  nodo_feature_list *aux;
  weightRepository *weightRep;
  hash_t *bias;
  int i,numMaybe,ret=1,max=0;
  int is_unk=FALSE;
  simpleList *featureList;

  startFexTime = times(&tbuffStartFex);

  i = d->getElement(elem->wrd);
  if (i!=HASH_FAIL)
    {
      featureList = &taggerModelRunning->featureList;
      numMaybe = d->getElementNumMaybe(i);
      weight = taggerCreateWeightNodeArray(numMaybe,i);
      if ((strcmp(flow,"LRL")==0) && (direction==2))
	{
	  weightRep = taggerModelRunning->wr2;  //wr2;
	  //bias = taggerModelRunning->bias2;  //taggerBias2;
	}
      else
	{
	  weightRep = taggerModelRunning->wr; //wr;
	  //bias = taggerModelRunning->bias; //taggerBias;
	}
    }
  else
    {
      numMaybe = NUM_UNK_POS;
      weight = taggerInitializeWeightNodeArray(numMaybe,weightUnk);
      featureList = &taggerModelRunning->featureListUnk;
      is_unk = TRUE; 

      if ((strcmp(flow,"LRL")==0) && (direction==2))
	{
		weightRep = taggerModelRunning->wrUnk2;  //wrUnk2;
		//bias =	taggerModelRunning->biasUnk2; //taggerBiasUnk2;
	}
      else
	{	weightRep = taggerModelRunning->wrUnk; //wrUnk;
		//bias =taggerModelRunning->biasUnk; //taggerBiasUnk;
	}
    }

    
  if (numMaybe>1)
    {
      while (ret>=0)
	{
	  aux = (nodo_feature_list *) featureList->getIndex();
	  if (strcmp(aux->mark,SLASTW)==0)  sw->winPushSwnFeature(stk);
	  else if (strcmp(aux->mark,WMARK)==0)  sw->winPushWordFeature((void *)aux,d,stk,direction); 
	  else if (strcmp(aux->mark,KMARK)==0)  sw->winPushAmbiguityFeature((void *)aux,d,stk,direction);
	  else if (strcmp(aux->mark,MMARK)==0)  sw->winPushMaybeFeature((void *)aux,d,stk,direction);
	  else if (strcmp(aux->mark,PMARK)==0)  sw->winPushPosFeature((void *)aux,d,stk,direction);
	  else if (strcmp(aux->mark,MFTMARK)==0)  sw->winPushMFTFeature((void *)aux,d,stk,direction);
	  else if (is_unk==TRUE)
	    {
	      int *param;
	      if (aux->n>0)
		{
		  param = (int *) aux->l.getIndex();
		}
	      if (strcmp(aux->mark,PREFIX_MARK)==0)  sw->winPushPrefixFeature(elem->wrd, stk, *param);
	      else if (strcmp(aux->mark,SUFFIX_MARK)==0) sw->winPushSuffixFeature(elem->wrd, stk, *param);
	      else if (strcmp(aux->mark,CHAR_A_MARK)==0) sw->winPushLetterFeature(elem->wrd, stk, *param, COUNTING_FROM_BEGIN);
	      else if (strcmp(aux->mark,CHAR_Z_MARK)==0) sw->winPushLetterFeature(elem->wrd, stk, *param, COUNTING_FROM_END);
	      else if (strcmp(aux->mark,LENGTH_MARK)==0) sw->winPushLenghtFeature(elem->wrd,stk);
	      else if (strcmp(aux->mark,START_CAPITAL_MARK)==0) sw->winPushStartWithCapFeature(elem->wrd,stk);
	      else if (strcmp(aux->mark,START_LOWER_MARK)==0)  sw->winPushStartWithLowerFeature(elem->wrd,stk);
	      else if (strcmp(aux->mark,START_NUMBER_MARK)==0) sw->winPushStartWithNumberFeature(elem->wrd,stk);
	      else if (strcmp(aux->mark,ALL_UPPER_MARK)==0) sw->winPushAllUpFeature(elem->wrd,stk);
	      else if (strcmp(aux->mark,ALL_LOWER_MARK)==0) sw->winPushAllLowFeature(elem->wrd,stk);
	      else if (strcmp(aux->mark,CONTAIN_CAP_MARK)==0) sw->winPushContainCapFeature(elem->wrd, stk);
	      else if (strcmp(aux->mark,CONTAIN_CAPS_MARK)==0) sw->winPushContainCapsFeature(elem->wrd, stk);
	      else if (strcmp(aux->mark,CONTAIN_COMMA_MARK)==0) sw->winPushContainCommaFeature(elem->wrd, stk);
	      else if (strcmp(aux->mark,CONTAIN_NUMBER_MARK)==0) sw->winPushContainNumFeature(elem->wrd, stk);
	      else if (strcmp(aux->mark,CONTAIN_PERIOD_MARK)==0) sw->winPushContainPeriodFeature(elem->wrd, stk);
	      else if (strcmp(aux->mark,MULTIWORD_MARK)==0) sw->winPushMultiwordFeature(elem->wrd, stk);
	    }
	  ret = featureList->next();
	}
      featureList->setFirst();

      endFexTime = times(&tbuffEndFex);
      realFexTime = realFexTime + ((double)(endFexTime-startFexTime))/CLOCKS_PER_SECOND;
      usrFexTime = usrFexTime + (((double)tbuffEndFex.tms_utime-(double)tbuffStartFex.tms_utime)/CLOCKS_PER_SECOND);
      sysFexTime = sysFexTime + (((double)tbuffEndFex.tms_stime-(double)tbuffStartFex.tms_stime)/CLOCKS_PER_SECOND);
	
      startSVMTime = times(&tbuffStartSVM);
	
      taggerSumWeight(weightRep,bias,weight,numMaybe,&max);
	
      endSVMTime = times(&tbuffEndSVM);
      realSVMTime = realSVMTime + ((double)(endSVMTime-startSVMTime))/CLOCKS_PER_SECOND;
      usrSVMTime = usrSVMTime + (((double)tbuffEndSVM.tms_utime-(double)tbuffStartSVM.tms_utime)/CLOCKS_PER_SECOND);
      sysSVMTime = sysSVMTime + (((double)tbuffEndSVM.tms_stime-(double)tbuffStartSVM.tms_stime)/CLOCKS_PER_SECOND);
    }

  strcpy(elem->pos,weight[max].pos);
  elem->weight = weight[max].data;

  if (strcmp(flow,"LRL")==0)
    {	
    	weight_node_t *score = new weight_node_t;
	score->data = weight[max].data;
	strcpy(score->pos,weight[max].pos);
	push(elem->stackScores,score);
    }

  if (i!=HASH_FAIL) delete[] weight; //mod delete[] instead of delete
}

/***************************************************************/

weight_node_t *tagger::taggerCreateWeightNodeArray(int numMaybe,int index)
{
  int ret=1,j = numMaybe;
  weight_node_t *weight = new weight_node_t[numMaybe];
  simpleList *list = (simpleList *) d->getElementMaybe(index);
  
  while (ret>=0 && numMaybe > 0)
    {
      infoDict *pInfoDict = (infoDict *) list->getIndex();
      j--;
      sprintf(weight[j].pos,"%s",pInfoDict->txt);
      weight[j].data = 0;
      ret=list->next();
    }

  list->setFirst();
  return weight;
}

/***************************************************************/

weight_node_t *tagger::taggerInitializeWeightNodeArray(int numMaybe,weight_node_t *w)
{
  for (int i=0;i<numMaybe;i++) w[i].data=0;
  return w;
}

/***************************************************************/

void tagger::taggerSumWeight(weightRepository *wRep,hash_t *bias,weight_node_t *weight, int numMaybe, int *max)
{
  weight_node_t *aux;
  long double w,b = 0;
  char *feature;
  int putBias=1;

  while (!empty(stk))
    {
      *max=0;
      feature = (char *) pop(stk);
      for (int j=0; j<numMaybe;j++)
	{
	  if (putBias)
	    {
	      b = wRep->wrGetWeight("BIASES",weight[j].pos);
	      weight[j].data = weight[j].data - b;
	    }
	  w = wRep->wrGetWeight(feature,weight[j].pos);
	  weight[j].data=weight[j].data+w;
	  if (((float)weight[*max].data)<((float)weight[j].data)) *max=j;
	}
      delete[] feature; //mod delete[] instead of delete
      putBias=0;
    }
}

/***************************************************************/
/***************************************************************/

weight_node_t *tagger::taggerCreateWeightUnkArray(char *name)
{
  NUM_UNK_POS=0;
  int i=0;
  char c=' ';
  FILE *f;

  if ((f = fopen(name, "rt"))== NULL)
    {
      fprintf(stderr, "Error opening file: %s",name);
      exit(0);
    }

  while (!feof(f))
    { if (fgetc(f)=='\n') NUM_UNK_POS++;
    }
  
  fseek(f,0,SEEK_SET);

  weight_node_t *weight = new weight_node_t[NUM_UNK_POS];
  while (!feof(f) && (i<NUM_UNK_POS))
    {	strcpy(weight[i].pos,"");
    weight[i].data=0;
    c = fgetc(f);
    while ((c!='\n') && (!feof(f)))
      {
	if (c!=' ' && c!='\n' && c!='\t') sprintf(weight[i].pos,"%s%c",weight[i].pos,c);
	c = fgetc(f);
      }
    i++;
    }
  fclose(f);
  return weight;
}

/***************************************************************/
/***************************************************************/

void tagger::taggerStadistics(int numWords, int numSentences,  double realTime,double usrTime, double sysTime)
{
  char message[200]="";
  float media=0;
  if (time!=0)  media = (float) (((double) numWords)/(sysTime+usrTime));

  sprintf(message,"%s\n%d sentences were tagged.",message,numSentences);
  sprintf(message,"%s\n%d words were tagged.",message,numWords);
  sprintf(message,"%s\n%f words/second were tagged.\n",message,media);
  fwrite(message,strlen(message),1,stderr);
}

/***************************************************************/

void tagger::taggerShowVerbose(int num,int isEnd)
{
  if (isEnd) { fprintf(stderr,".%d sentences [DONE]\n\n",num); return; }
  else if (num%100==0) fprintf(stderr,"%d",num);
  else if (num%10==0) fprintf(stderr,".");
}


