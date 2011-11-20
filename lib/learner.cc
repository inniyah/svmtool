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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "marks.h"
#include "hash.h"
#include "list.h"
#include "dict.h"
#include "stack.h"
#include "swindow.h"
#include "mapping.h"
#include "weight.h"
#include "learner.h"
#include "common.h"

/**************************************************************/

extern int verbose;
double time_svmlight = 0;

/**************************************************************/

stack_t DO;
int KERNEL=0;
int DEGREE=0;
float CK = 0;
float CU = 0;
float X  = 3;
int MAX_MAPPING_SIZE = 100000;
int COUNT_CUT_OFF = 2;
int WINDOW_SIZE = 5;
int CORE_POSITION = 2;
char *TRAINSET = NULL;
char *SVMDIR = NULL;
char *NAME = NULL;
char *BLEX = NULL;
char *R = NULL;
float DRATIO = 0.001;
float ERATIO=0;
float KFILTER = 0;
float UFILTER = 0;
int   REMOVE_FILES = TRUE;

/**************************************************************/

char *UP = NULL;
char *AP = NULL;

/**************************************************************/

//ambiguous-right [default]
const char *A0 = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(-2,-1)\np(-1,1)\np(1,2)\np(-2,-1,1)\np(-1,1,2)\nk(0)\nk(1)\nk(2)\nm(0)\nm(1)\nm(2)\n";
const char *A0UNK = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(-2,-1)\np(-1,1)\np(1,2)\np(-2,-1,1)\np(-1,1,2)\nk(0)\nk(1)\nk(2)\nm(0)\nm(1)\nm(2)\na(2)\na(3)\na(4)\nz(2)\nz(3)\nz(4)\nca(1)\ncz(1)\nL\nSA\nAA\nSN\nCA\nCAA\nCP\nCC\nCN\nMW\n";

/**************************************************************/

//unambiguous-right
const char *A1 =  "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(1)\np(2)\np(-2,-1)\np(-1,0)\np(-1,1)\np(0,1)\np(1,2)\np(-2,-1,0)\np(-2,-1,1)\np(-1,0,1)\np(-1,1,2)\nk(0)\nk(1)\nk(2)\nm(0)\nm(1)\nm(2)\n";
const char *A1UNK =  "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(1)\np(2)\np(-2,-1)\np(-1,0)\np(-1,1)\np(0,1)\np(1,2)\np(-2,-1,0)\np(-2,-1,1)\np(-1,0,1)\np(-1,1,2)\nk(0)\nk(1)\nk(2)\nm(0)\nm(1)\nm(2)\na(1)\na(2)\na(3)\na(4)\nz(1)\nz(2)\nz(3)\nz(4)\nL\nSA\nAA\nSN\nCA\nCAA\nCP\nCC\nCN\nMW\n";

/**************************************************************/

//no-right
const char *A2 = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(-2,-1)\nk(0)\nm(0)\n";
const char *A2UNK = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(-2,-1)\nk(0)\nm(0)\na(1)\na(2)\na(3)\na(4)\nz(1)\nz(2)\nz(3)\nz(4)\nL\nSA\nAA\nSN\nCA\nCAA\nCP\nCC\nCN\nMW\n";

/**************************************************************/

//unsupervised-learning
const char *A3 = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2))\np(-2))\np(-1))\np(-2,-1))\np(-1,1))\np(1,2))\np(-2,-1,1))\np(-1,1,2))\nk(-2))\nk(-1))\nk(1))\nk(2)\n)\nm(-2))\nm(-1))\nm(1))\nm(2)\n";
const char *A3UNK = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2))\np(-2))\np(-1))\np(-2,-1))\np(-1,1))\np(1,2))\np(-2,-1,1))\np(-1,1,2))\nk(-2))\nk(-1))\nk(1))\nk(2)\n)\nm(-2))\nm(-1))\nm(1))\nm(2)\na(1)\na(2)\na(3)\na(4)\nz(1)\nz(2)\nz(3)\nz(4)\nL\nSA\nAA\nSN\nCA\nCAA\nCP\nCC\nCN\nMW\n";

/**************************************************************/

//ambiguous-right ++ unknown words on training
const char *A4 = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(-2,-1)\np(-1,1)\np(1,2)\np(-2,-1,1)\np(-1,1,2)\nk(0)\nk(1)\nk(2)\nm(0)\nm(1)\nm(2)\n";
const char *A4UNK = "w(-2)\nw(-1)\nw(0)\nw(1)\nw(2)\nw(-2,-1)\nw(-1,0)\nw(0,1)\nw(-1,1)\nw(1,2)\nw(-2,-1,0)\nw(-2,-1,1)\nw(-1,0,1)\nw(-1,1,2)\nw(0,1,2)\np(-2)\np(-1)\np(-2,-1)\np(-1,1)\np(1,2)\np(-2,-1,1)\np(-1,1,2)\nk(0)\nk(1)\nk(2)\nm(0)\nm(1)\nm(2)\na(1)\na(2)\na(3)\na(4)\nz(1)\nz(2)\nz(3)\nz(4)\nL\nSA\nAA\nSN\nCA\nCAA\nCP\nCC\nCN\nMW\n";

/**************************************************************/

char *learner::read_feature_list_from_config_file(FILE *f, char *first_feature)
{
  char tmp[1000],str[500],*out;
  strcpy(tmp,"");
  sprintf(tmp,"%s\n",first_feature);

  int cont=1;
  int ret=1;

  while (ret>0)
    {
      strcpy(str,"");
      ret = readTo(f,' ','\n',str);
      if (ret>=0)
	{
	  sprintf(tmp,"%s%s\n",tmp,str);
	  cont++;
	}
    }
  out = new char[strlen(tmp)+1];
  strcpy(out,tmp);
  return out;
}

/**************************************************************/

void learner::read_config_file(const char *config_file)
{
  int ret1=1,ret2=1,ret3=1;
  char str1[500],str2[500],str3[500];

  FILE *f = openFile(config_file,"r");
  FILE *tmp = openFile("tmp_config.svmt","w+");
  char c,ant;
  ant = '~';
  while (!feof(f))
    {
      c = fgetc(f);
      if (c=='#') readTo(f,'\n','\n',str1);
      else if (!(c == ' ' && ant == ' ')) fprintf(tmp,"%c",c);
      ant = c;
    }
  fclose(f);

  fseek(tmp,0,SEEK_SET);
  while (!feof(tmp))
    {
      strcpy(str1,""); strcpy(str2,""); strcpy(str3,"");
      ret1 = readTo(tmp,' ','=',str1);
      if (ret1>=0 && strcmp(str1,"do")==0)
	{
	  ret2 = readTo(tmp,' ',' ',str2);
	  ret3 = readTo(tmp,' ','\n',str3);
	  char *p = strstr(str2,"M");
	  int *modelo = new int;
	  int *direction = new int;
	  *modelo = atoi(&p[1]);
	  if (strcmp(str3,"LR")==0) *direction = LEFT_TO_RIGHT;
	  else if (strcmp(str3,"RL")==0) *direction = RIGHT_TO_LEFT;
	  else if (strcmp(str3,"LRL")==0) *direction = LR_AND_RL;
	  push(&DO,direction);
	  push(&DO,modelo);
	}
      else if (ret1>=0 && strcmp(str1,"")!=0)
	{
	  ret2 = readTo(tmp,' ',' ',str2);
	  ret2 = readTo(tmp,' ','\n',str2);
	  if (ret1>=0 && ret2>=0)
	    {
	      if (strcmp(str1,"A0k")==0 || strcmp(str1,"A0")==0)	A0 = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A1k")==0 || strcmp(str1,"A1")==0) A1 = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A2k")==0 || strcmp(str1,"A2")==0) A2 = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A3k")==0 || strcmp(str1,"A3")==0) A3 = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A4k")==0 || strcmp(str1,"A4")==0) A4 = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A0u")==0 || strcmp(str1,"A0unk")==0) A0UNK = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A1u")==0 || strcmp(str1,"A1unk")==0) A1UNK = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A2u")==0 || strcmp(str1,"A2unk")==0) A2UNK = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A3u")==0 || strcmp(str1,"A3unk")==0) A3UNK = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"A4u")==0 || strcmp(str1,"A4unk")==0) A4UNK = read_feature_list_from_config_file(tmp,str2);
	      else if (strcmp(str1,"F")==0 && ret3>=0)
		{
		  ret3 = readTo(tmp,'\n','\n',str3);
		  MAX_MAPPING_SIZE = atoi(str3);
		  COUNT_CUT_OFF = atoi (str2);
		}
	      else if (strcmp(str1,"W")==0)
		{
		  ret3 = readTo(tmp,'\n','\n',str3);
		  WINDOW_SIZE = atoi(str2);
		  CORE_POSITION = atoi (str3);
		}
	      else if (strcmp(str1,"TRAINSET")==0)
		{
		  TRAINSET = new char[strlen(str2)+1];
		  strcpy(TRAINSET,str2);
		}
	      else if (strcmp(str1,"BLEX")==0)
		{
		  BLEX = new char[strlen(str2)+1];
		  strcpy(BLEX,str2);
		}
	      else if (strcmp(str1,"R")==0)
		{
		  R = new char[strlen(str2)+1];
		  strcpy(R,str2);
		}
	      else if (strcmp(str1,"SVMDIR")==0)
		{
		  SVMDIR = new char[strlen(str2)+1];
		  strcpy(SVMDIR,str2);
		}
	      else if (strcmp(str1,"NAME")==0)
		{
		  NAME = new char[strlen(str2)+1];
		  strcpy(NAME,str2);
		}
	      else if (strcmp(str1,"REMOVE_FILES")==0)
		{
		  REMOVE_FILES = atoi (str2);
		}
	      else if (strcmp(str1,"CK")==0)
		{
		  CK = atof (str2);
		}
	      else if (strcmp(str1,"CU")==0)
		{
		  CU = atof (str2);
		}
	      else if (strcmp(str1,"Dratio")==0)
		{
		  DRATIO = atof (str2);
		}
	      else if (strcmp(str1,"Eratio")==0)
		{
		  ERATIO = atof (str2);
		}
	      else if (strcmp(str1,"Kfilter")==0)
		{
		  KFILTER = atof (str2);
		}
	      else if (strcmp(str1,"Ufilter")==0)
		{
		  UFILTER = atof (str2);
		}
	      else if (strcmp(str1,"X")==0)
		{
		  X  = atof (str2);
		}
	      else if (strcmp(str1,"AP")==0)
		{
		  int ret  = 1;
		  learnerAMBP_H = new hash_t;
		  hash_init(learnerAMBP_H,30);

		  infoDict * etiq = new infoDict;
		  strcpy(etiq->txt,str2);
		  hash_insert(learnerAMBP_H,etiq->txt,(uintptr_t) etiq);

		  if (ret2>0)
		  {
		    memset(str2,0,strlen(str2));
		    fgets(str2,500,tmp);

		    for (int i=0; i<strlen(str2);i++)
		      {
			memset(str3,0,strlen(str3));
			sscanf(str2+i,"%s",str3);
			i = strlen(str3) + i;

			etiq = new infoDict;
			strcpy(etiq->txt,str3);
			hash_insert(learnerAMBP_H,etiq->txt,(uintptr_t) etiq);
		      } //EndFor
		  } //End if(ret2>0)
		}
	      else if (strcmp(str1,"UP")==0)
		{
		  int ret  = 1;
		  learnerUNKP_H = new hash_t;
		  hash_init(learnerUNKP_H,30);

		  infoDict * etiq = new infoDict;
		  strcpy(etiq->txt,str2);
		  hash_insert(learnerUNKP_H,etiq->txt,(uintptr_t) etiq);
		  if (ret2>0)
		    {
		      memset(str2,0,strlen(str2));
		      fgets(str2,500,tmp);

		      for (int i=0; ret2>0 && i<strlen(str2);i++)
			{
			  memset(str3,0,strlen(str3));
			  sscanf(str2+i,"%s",str3);
			  i = strlen(str3) + i;

			  etiq = new infoDict;
			  strcpy(etiq->txt,str3);
			  hash_insert(learnerUNKP_H,etiq->txt,(uintptr_t) etiq);
			} //Endfor
		    } //End if(ret2>0)
      		}
      	    }
	}
    }
  fclose (tmp);

  if (verbose==TRUE)
    {
	fprintf(stderr,"\n* ===================== SVMTlearn configuration ==========================");
	fprintf(stderr,"\n* config file   = [ %s ]\n* trainset      = [ %s ]\n* model name    = [ %s ]",config_file,TRAINSET,NAME);
	fprintf(stderr,"\n* SVM-light dir = [ %s ]",SVMDIR);
	fprintf(stderr,"\n* ========================================================================");
	fprintf(stderr,"\n* unknown words expected   = [ X  = %f  % ]",X);
	fprintf(stderr,"\n* C parameter for known    = [ CK = %f ]",CK);
	fprintf(stderr,"\n* C parameter for unknown  = [ CU = %f ]",CU);
	fprintf(stderr,"\n* D ratio                  = [ Dratio = %f ]",DRATIO);
	fprintf(stderr,"\n* E ratio                  = [ Eratio = %f ]",ERATIO);
	fprintf(stderr,"\n* Known weights filter     = [ Kfilter = %f ]",KFILTER);
	fprintf(stderr,"\n* Unknown weights filter   = [ Ufilter = %f ]",UFILTER);
	fprintf(stderr,"\n* sliding window settings  = [ WINDOW SIZE = %d , CORE POSITION = %d ]",WINDOW_SIZE,CORE_POSITION);
	fprintf(stderr,"\n* mapping settings         = [ COUNT CUT OFF = %d , MAX MAPPING SIZE = %d ]",COUNT_CUT_OFF,MAX_MAPPING_SIZE);
	fprintf(stderr,"\n* remove temporal files    = [ %d ] (1) TRUE, (0) FALSE",REMOVE_FILES);	
	fprintf(stderr,"\n* ========================================================================");
    }

  system("rm -f tmp_config.svmt");

  if (TRAINSET == NULL) { fprintf (stderr,"\nError: TRAINSET parameter not found in %s.\n",config_file); exit(-1); }
  if (NAME == NULL) { fprintf (stderr,"\nError: MODEL NAME parameter not found in %s.\n",config_file); exit(-1); }
  if (SVMDIR == NULL) {  fprintf (stderr,"\nError: SVM DIRECTORY parameter not found in %s.\n",config_file); exit(-1); }
}

/**************************************************************/

learner::learner()
{
  learnerAMBP_H = NULL ;
  learnerUNKP_H = NULL ;
}

learner::~learner()
{
 char str[1000];
 strcpy(str,"");
 if (REMOVE_FILES==TRUE)
 {
   removeFiles(NAME,RM_TEMP_FILES,0,0,verbose);
 }

 if ( verbose == TRUE ) fprintf(stderr,"\n\nTERMINATION ... ");

 if (TRAINSET!=NULL) delete TRAINSET;
 if (SVMDIR!=NULL) delete SVMDIR;
 if (NAME!=NULL) delete NAME;
 if (BLEX!=NULL) delete BLEX;
 if (R!=NULL) delete R;

 if ( verbose == TRUE ) fprintf(stderr,"[DONE]\n\n");
}

/***************************************************************/

void learner::learnerCreatePOSFile(char *modelName, int is_ambp, hash_t *h)
{
  char name[300];
  if (is_ambp==TRUE)
    {
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.AMBP",modelName);
      sprintf(name,"%s.AMBP",modelName);
      FILE *f = openFile (name,"w");
      hash_print(h,f);
      fclose(f);
    }
  else
    {
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.UNKP",modelName);
      sprintf(name,"%s.UNKP",modelName);
      FILE *f = openFile (name,"w");
      hash_print(h,f);
      fclose(f);
    }
}

/***************************************************************/

void learner::learnerCreateDefaultFile(const char *modelName, const char *str)
{
  char name[500];
  sprintf (name,"%s.%s",modelName,str);
  FILE *f = openFile(name, "w");

  if (strcmp(str,"A0")==0)
  {
      if ( verbose == TRUE )  fprintf(stderr,"\nStoring %s.A0",modelName); fprintf(f,"%s",A0);
      sprintf (name,"%s.%s.UNK",modelName,str);
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A0.UNK",modelName);
      FILE *funk = openFile(name, "w");
      fprintf(funk,"%s",A0UNK);
      fclose(funk);
  }
  else if (strcmp(str,"A1")==0)
  {
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A1",modelName); fprintf(f,"%s",A1);
      sprintf (name,"%s.%s.UNK",modelName,str);
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A1.UNK",modelName);
      FILE *funk = openFile(name, "w");
      fprintf(funk,"%s",A1UNK);
      fclose(funk);
  }
  else if (strcmp(str,"A2")==0)
  {
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A2",modelName); fprintf(f,"%s",A2);
      sprintf (name,"%s.%s.UNK",modelName,str);
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A2.UNK",modelName);
      FILE *funk = openFile(name, "w");
      fprintf(funk,"%s",A2UNK);
      fclose(funk);
  }
  else if (strcmp(str,"A3")==0)
  {
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A3",modelName); fprintf(f,"%s",A3);
      sprintf (name,"%s.%s.UNK",modelName,str);
      if ( verbose == TRUE )  fprintf(stderr,"\nStoring %s.A3.UNK",modelName);
      FILE *funk = openFile(name, "w");
      fprintf(funk,"%s",A3UNK);
      fclose(funk);
  }
  else if (strcmp(str,"A4")==0)
  {
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A4",modelName); fprintf(f,"%s",A4);
      sprintf (name,"%s.%s.UNK",modelName,str);
      if ( verbose == TRUE ) fprintf(stderr,"\nStoring %s.A4.UNK",modelName);
      FILE *funk = openFile(name, "w");
      fprintf(funk,"%s",A4UNK);
      fclose(funk);
  }
  else if (strcmp(str,"WIN")==0)
    {
      if ( verbose == TRUE )  fprintf(stderr,"\nStoring %s.WIN",modelName);
      fprintf(f,"%d\n%d\n",WINDOW_SIZE,CORE_POSITION);
    }
  fclose(f);
}

/***************************************************************/

/*
 * Return CHAR_NULL if end of file
 * Return w,p,k (if we readed a) or s (if we readed m) if it's found
 */
char learner::obtainAtrChar(FILE *channel)
{
  char c;
  while (!feof(channel))
    {
      c=fgetc(channel);
      if (c=='w' || c=='p' || c=='k' || c=='m')
	{
	  fgetc(channel);
	  switch (c)
	    {
	    case 'k': return 'k';
	    case 'm': return 's';
	    default:  return c;
	    }
	}
    }
  return CHAR_NULL;
}

/**************************************************************/

/*
 * Cada atributo de una lista de atributos tiene la forma
 * <Marca>(<cadena de enteros separados por comas>)
 * Usaremos esta función para leer cada uno de los enteros de
 * la cadena que indica las posiciones de la ventana a considerar.
 * Lee del fichero (channel) un entero entre '(' y coma, comas,
 * coma y ')’ o paréntesis. Devuelve el número leído como entero.
 */
int learner::obtainAtrInt(FILE *channel,int *endAtr)
{
  int i=0;
  char c=' ',num[5]="";

  while ( (!feof(channel)) && (c!='(') && (c!=',') && (c!=')') )
    {
      c=fgetc(channel);
      if ((c!='(') && (c!=')')) num[i]=c;
      i++;
    }
  if (c==')') *endAtr=1;
  num[i]='\0';
  return atoi(num);
}

/***************************************************************/

/* int learnerCount(char *name, int *nWords, int *nSentences)
 * Cuenta el numero de palabras y frases que tiene el corpus
 * contenido en el fichero con nombre <name>. El numero de
 * palabras y de frases es devuelto como parametro de salida
 * mediante <nWords> y <nSentences>.
 */
void learner::learnerCount(char* name, int *nWords, int *nSentences)
{
  int ret1=0,ret2=0,cont=0,contWords=0;
  char c=' ',str[300];

  FILE *f = openFile(name, "r");
  while (!feof(f) && ret1>=0 && ret2>=0)
    {
      ret1 = readTo(f,' ',0,str);
      if (strcmp(str,"!")==0 || strcmp(str,"?")==0 || strcmp(str,".")==0) cont++;
      ret2 = readTo(f,'\n',0,str);
      if ( !feof(f) && ret1 >= 0 ) contWords++;
    }
  fclose(f);

  if ( contWords <= 1 ) 
    {
      fprintf(stderr,"\n\nInput corpus too short to begin training!! Program stopped.\n\n"); 
      exit(0);
    }

  *nWords = contWords;
  *nSentences = cont;
}


/********************************************************/

void learner::learnerPrintMessage(int numModel, int K_or_U, int LR_or_RL, int is_fex)
{
  if (verbose==TRUE)
    {
      fprintf(stderr,"\n\n* ========================================================================");
      if (is_fex==FALSE) fprintf(stderr,"\n* TRAINING MODEL %d ",numModel);
      else fprintf(stderr,"\n* FEATURES EXTRACTION FOR MODEL %d ",numModel);
      if (is_fex==TRUE) fprintf(stderr,"[ KNOWN AND UNKNOWN WORDS  -  ");
      else if (K_or_U==KNOWN) 	fprintf(stderr,"[ KNOWN WORDS  -  ");
      else fprintf(stderr,"[ UNKNOWN WORDS  -  ");
      if (LR_or_RL==LEFT_TO_RIGHT) fprintf(stderr,"LEFT TO RIGHT ]");
      else if (LR_or_RL==RIGHT_TO_LEFT) fprintf(stderr,"RIGHT TO LEFT ]");
      else if (LR_or_RL==LR_AND_RL)fprintf(stderr,"LEFT TO RIGHT AND RIGHT TO LEFT ]");
      fprintf(stderr,"\n* ========================================================================");
    }
}

/**************************************************************/

/*
 * Parámetros:
 * wrd		char *	Palabra de ejemplo que se está tratando.
 * numModel	int	Modelo que estamos entrenando.
 * direction	int	Dirección en la cual se realiza el entrenamiento
 			(Derecha a izquierda o izquierda a derecha).
 * Known_or_Unknown	int	Si se está entrenando para palabras conocidas
 				o desconocidas.
 * pos	char *	La etiqueta morfosintáctica que estamos entrenando.
 * samplePos	char *	Etiqueta morfosintáctica del ejemplo  que estamos tratando.
 * features	char *	Lista de atributos generados para el ejemplo.
 * d	dictionary *	Diccionario que se está usando para el entrenamiento.
 * nNeg	int *	Apuntador al número de palabras seleccionadas como ejemplos negativos.
 * nPos	int *	Apuntador al número de palabras seleccionadas como ejemplos positivos.
 *
 * Este método puede usarse para seleccionar ejemplos para palabras conocidas
 */
void learner::learnerPushSample(char *wrd,int numModel,int direction, int Known_or_Unknown,char *pos, char *samplePos, char *features,dictionary *d, int *nNeg, int *nPos)
{
    char fileName[100];
    //Se abre el fichero donde se debe insertar el ejemplo.
    generateFileName(NAME,pos,numModel,direction, Known_or_Unknown, "POS", fileName);
  
    FILE *f = openFile(fileName,"a+");

    //Se obtiene la lista de posibles etiquetas morfosintácticas para
    //la palabra wrd.
    simpleList *l = learnerGetPotser(wrd,Known_or_Unknown,d);
    l->setFirst();
    for (int stop=1; stop>=0  ; stop = l->next())
    {
    	//Se busca la etiqueta pos en la lista obtenida.
    	//Si se encuentra y es igual a samplePos se selecciona el ejemplo
    	//como positivo, si no es igual a samplePos se selecciona como
    	//negativo.
	infoDict *pInfo = (infoDict *) l->getIndex();
	if (pInfo != NULL )
	{
	  if (strcmp(pInfo->txt,pos)==0)
	  {
	    if (strcmp(pInfo->txt,samplePos)==0)
	    {
	    	//Positive Sample
		*nPos=(*nPos)+1;
		fprintf (f,"+1 %s\n",features);
	    }
	    else
	    {
	    	//Negative Sample
		*nNeg=(*nNeg)+1;
		fprintf (f,"-1 %s\n",features);
	    }
	  }
	}
     }

   d->dictCleanListInfoDict(l,l->numElements());
   delete l;
   //Se cierra el fichero.
   fclose(f);
}

/**************************************************************/
/*
 * Parámetros:
 * wrd	Char *	Palabra del ejemplo que se está tratando.
 * numModel	Int	Modelo que estamos entrenando.
 * direction	Int	Dirección en la cual se realiza el entrenamiento
 			(Derecha a izquierda o izquierda a derecha).
 * Known_or_Unknown	Int	Si se está entrenando para palabras conocidas
 				o desconocidas.
 * pos	Char *	La etiqueta morfosintáctica que estamos entrenando.
 * samplePos	Char *	Etiqueta morfosintáctica del ejemplo  que estamos tratando.
 * features	Char *	Lista de atributos generadas para el ejemplo.
 * d	dictionary *	Diccionario que se está usando para el entrenamiento.
 * nNeg	int *	Apuntador al número de palabras seleccionadas como ejemplos negativos.
 * nPos	int *	Apuntador al número de palabras seleccionadas como ejemplos positivos.
 * Este método se encarga de seleccionar ejemplos para palabras desconocidas.
*/
void learner::learnerPushSampleUnk(char *wrd,int numModel,int direction, int Known_or_Unknown,char *pos, char *samplePos, char *features,dictionary *d, int *nNeg, int *nPos)
{
    char fileName[100];
    generateFileName(NAME,pos,numModel,direction, Known_or_Unknown, "POS", fileName);
    //Abrimos el fichero
    FILE *f = openFile(fileName,"a+");

    //Si pos es igual a samplePos se selecciona el ejemplo como positivo,
    //en cualquier otro caso se selecciona como negativo.
    if (strcmp(samplePos,pos)==0)
	   {
	   	//Positive Sample
		*nPos=(*nPos)+1;
		fprintf (f,"+1 %s\n",features);
	   }
    else
	    {
	    	//Negative Sample
		*nNeg=(*nNeg)+1;
		fprintf (f,"-1 %s\n",features);
	    }
   //Cerramos el fichero
   fclose(f);
}

/**************************************************************/
/*
 * Este método recibe como parámetros un apuntador a un fichero (f)
 * y el apuntador a un objeto del tipo mapping. El objetivo de este
 * método es leer los atributos generados para un ejemplo del fichero
 * f y devolver un string con la lista de atributos en el formato
 * esperado por SVM-light.
 */
char *learner::learnerCreateFeatureString(FILE *f,mapping *m)
{
	char *features = new char[1000];
	int array[learnerNumFeatures];
	int ret1=1,i = 0;
	char str[100];

	//Construimos un array de enteros con los
	//identificadores numéricos de cada atributo
	while (ret1>0 && !feof(f))
	{
	  ret1 = readTo(f,' ','\n',str);
	  int num = m->mappingGetNumberByFeature(str);
	  if (ret1>=0 && num>-1)
	    {
	      array[i]=num;
	      i++;
	    }
	}

	//qsort --> ordena ascendetemente un array de enteros
	qsort(array,0,i-1);
	sprintf(features,"");

	for (int j=0;j<i;j++)
	{
	  //Contruimos la cadena de caracteres con losidentificadores
	  //numéricos
	  if (j!=i-1) sprintf(features,"%s%d:1 ",features,array[j]);
	  else sprintf(features,"%s%d:1",features,array[j]);
	}
	return features;
}

/**************************************************************/

/*
 * Parámetros:
 * d	dictionary*	Diccionario que se está usando para el entrenamiento.
 * m	Mapping*	Mapping entre atributos e identificadores numéricos
 * f	FILE*	Apuntador al fichero de ejemplos
 * pos	char *	La etiqueta morfosintáctica que estamos entrenando.
 * numModel	int	Modelo que estamos entrenando.
 * direction	int	Dirección en la cual se realiza el entrenamiento (Derecha a
 			izquierda o izquierda a derecha).
 * K_or_U	int	Si se está entrenando para palabras conocidas o desconocidas.
 * nNeg	int *	Apuntador al número de palabras seleccionadas como ejemplos negativos.
 * nPos	int *	Apuntador al número de palabras seleccionadas como ejemplos positivos.
 *
 * Este método lee el fichero en el cual están contenidos los ejemplos seleccionados
 * para el entrenamiento.
*/
void learner::learnerDressNakedSetTrain(dictionary *d,mapping *m,FILE *f, char* pos,int numModel, int direction, int K_or_U,int *nPos,int *nNeg)
{
    char wrd[TAM_WORD], samplePos[TAM_POS];
    char *features/*[1000]*/, garbage[1000],str[100];
    int array[learnerNumFeatures],cont=0;

    *nNeg=0; *nPos=0;

    fseek(f,0,SEEK_SET);
    //Para cada ejemplo seleccionado
    while (!feof(f))
    {
      int ret1 = readTo(f,':',0,wrd);
      int ret2 = readTo(f,' ',0,samplePos);
      if (ret1>=0 && ret2>=0)
	{
	    int isPossiblePOS = learnerIsPossiblePOS(wrd,pos,K_or_U);
	    if (isPossiblePOS==TRUE)
	    {
	      //Preparamos la lista de features
	      features = learnerCreateFeatureString(f,m);

	      if (K_or_U==KNOWN) learnerPushSample(wrd,numModel,direction,K_or_U,pos,samplePos,features,d,nNeg,nPos);
	      else learnerPushSampleUnk(wrd,numModel,direction,K_or_U,pos,samplePos,features,d,nNeg,nPos);
	      
	      if ( verbose  == TRUE) showProcessDone(cont , 1000, FALSE,"samples");
	      
	      delete features;
	    }
	  else readTo(f,'\n','\n',garbage);
	  cont++;
	} //if
    } //While
}

/**************************************************************/

/*
 * Parámetros:
 * f	FILE*	Apuntador al fichero de ejemplos
 * numModel	Int	Modelo que estamos entrenando.
 * LR_or_RL	Int	Dirección en la cual se realiza el entrenamiento
 * 			(Derecha a izquierda o izquierda a derecha).
 * K_or_U	Int	Si se está entrenando para palabras conocidas o desconocidas.
 * d	dictionary*	Diccionario que se está usando para el entrenamiento.
 * lPosToTrain	simpleList *	Lista de etiquetas morfosintácticas a entrenar
 *
 * Este método es el encargado de realizar el aprendizaje. Prepara las opciones de
 * ejecución  para SVM-light.  Construye el mapping mediante los datos contenidos
 * en el fichero de entrada. Crea un depósito de pesos (weightRepository) y
 * un hashing en el que almacenar los sesgos obtenidos para cada etiqueta.
 * Y prepara los datos para que puedan ser procesados por SVM-light.
 * Para cada etiqueta morfosintáctica de la lista lPosToTrain, se
 * llama al método learnerDressNakedSetTrain para conseguir los ficheros
 * de entrada para SVM-light. Una vez hecho esto, se llama a learnerExecSVMlight,
 * para ejecutar SVM-light. Con el fichero de salida generado por la herramienta
 * de Joachims, se rellenan las estructuras de datos con los pesos
 * (learnerBuiltWeightRepository) y los sesgos (learnerBuiltBias)
 * Una vez procesadas todas las etiquetas morfosintácticas a entrenar, el depósito
 * de pesos y el hashing de sesgos contienen todos los datos del modelo y se escriben 
 * en disco mediante los métodos weightRepository.wrWriteHash para los sesgos y 
 * weightRepository.wrWrite para los pesos.
 */

void learner::learnerDoLearn(FILE *f,int numModel,int LR_or_RL,int K_or_U,dictionary *d,simpleList *lPosToTrain)
{
  char posFileName[100],svmFileName[100],mapFileName[100];

  //Preparamos las opciones con que se ejecutará svm-light
  char options[100]="";
  if (CK!=0 && K_or_U==KNOWN) sprintf(options," -c %.6f ",CK);
  else if (CU!=0 && K_or_U==UNKNOWN) sprintf(options," -c %.6f ",CU);
  if (KERNEL!=0) sprintf(options,"%s -t %d ",options,KERNEL);
  if (DEGREE!=0) sprintf(options,"%s -d %d ",options,DEGREE);

  learnerPrintMessage(numModel,K_or_U,LR_or_RL,FALSE);

  if ( verbose == TRUE )
    {
      fprintf(stderr,"\nBuilding MAPPING for MODEL %d [ ",numModel);
      if (K_or_U==KNOWN) fprintf(stderr," KNOWN WORDS - ");
      else fprintf(stderr," UNKNOWN WORDS - ");
      if (LR_or_RL==LEFT_TO_RIGHT) fprintf(stderr,"LEFT TO RIGHT ]\n");
      else  fprintf(stderr,"RIGHT_TO_LEFT ]");
    }

  //Construimos el mapping a partir de los ejemplos seleccionados
  mapping *m =  new mapping();
  m->mappingBuilt(f,MAX_MAPPING_SIZE,COUNT_CUT_OFF);
  generateFileName(NAME,"",numModel,LR_or_RL,K_or_U,"MAP",mapFileName);
  m->mappingWrite(mapFileName,FALSE);

  //Creamos el depósito de pesos y el hash de sesgos
  hash_t *b = new hash_t;
  hash_init(b,30);
  weightRepository *wr = new weightRepository;

  infoDict *pInfo;
  int nPositive=0,nNegative=0;

  //Nos situamos en el primer  elemento de lista
  lPosToTrain->setFirst();
  //Para cada elemento de la lista de etiquetas
  for (int ret=1; ret>=0; ret=lPosToTrain->next())
  {
      if ( verbose == TRUE ) fprintf(stderr,"\n-----------------------------------------------------------");

      //Obtenemos la etiqueta morfosintáctica
      pInfo = (infoDict *)lPosToTrain->getIndex();
      nPositive=0;
      nNegative=0;

      //Preparamos el entrenamiento
      generateFileName(NAME,pInfo->txt,numModel,LR_or_RL,K_or_U,"POS",posFileName);
      generateFileName(NAME,pInfo->txt,numModel,LR_or_RL,K_or_U,"SVM",svmFileName);

      //Seleccionamos los ejemplos para el entrenamiento para la POS que estamos viendo
      if ( verbose == TRUE ) fprintf(stderr,"\nPreparing training set for [ %s ] ..",pInfo->txt);
	      learnerDressNakedSetTrain(d,m,f,pInfo->txt,numModel,LR_or_RL, K_or_U,&nPositive,&nNegative);

      //Realizamos el entrenamiento llamando a SVM-light
      if ( verbose == TRUE ) fprintf(stderr,"\nTraining [ %s ] with %d samples: [+] = %d samples ; [-] = %d samples\n",pInfo->txt,nPositive+nNegative,nPositive,nNegative);
       learnerExecSVMlight(SVMDIR,options,posFileName,svmFileName);

      //Se insertan los valores obtenidos del entrenamiento en el depósito de pesos
      //y el el hashing de sesgos
      if ( verbose == TRUE ) fprintf(stderr,"\nAdding elements to MERGED MODEL from [ %s ]",posFileName);
      wr = learnerBuiltWeightRepository(wr,m,pInfo->txt,svmFileName);
      if ( verbose == TRUE ) fprintf(stderr," [DONE]");
      if ( verbose == TRUE ) fprintf(stderr,"\nAdding biases from [ %s ]",posFileName);
      b =  learnerBuiltBias(b,pInfo->txt,svmFileName);
      if ( verbose == TRUE ) fprintf(stderr," [DONE]");

      if (REMOVE_FILES == TRUE)
	{
	  char cmd[150];
	  sprintf(cmd,"rm -f %s",posFileName);
	  system(cmd);
	  sprintf(cmd,"rm -f %s",svmFileName);
	  system(cmd);
	}
  }
  lPosToTrain->setFirst();

  if ( verbose == TRUE ) fprintf(stderr,"\n-----------------------------------------------------------");
  char fileName[100];
  generateFileName(NAME,"",numModel,LR_or_RL,K_or_U,"MRG",fileName);
  if ( verbose == TRUE ) fprintf(stderr,"\nStoring MERGED MODEL [ %s ]",fileName);
  //Modificación 180705: Filtrado de pesos
  if ( K_or_U == KNOWN ) wr->wrWrite(fileName, KFILTER); //ADD 180705
  else wr->wrWrite(fileName, UFILTER); //ADD 180705
  //Escribir deposito de pesos en disco
  //wr->wrWrite(fileName); //DEL 180705
  if ( verbose == TRUE ) fprintf(stderr," [DONE]");

  FILE *fwr = openFile(fileName,"a+");
  fprintf (fwr,"BIASES ");
  wr->wrWriteHash(b,fwr,' ');  //Escribir biases en fichero de depósito de pesos
  fclose(fwr);

  generateFileName(NAME,"",numModel,LR_or_RL,K_or_U,"B",fileName);
  if ( verbose == TRUE ) fprintf(stderr,"\nStoring BIASES [ %s ]",fileName);
  FILE *fb =openFile(fileName,"w");
  wr->wrWriteHash(b,fb,'\n');  //Escribir biases en fichero de sesgos
  fclose(fb);

  if ( verbose == TRUE ) fprintf(stderr," [DONE]");

  delete m;
  delete wr;
  learnerDestroyBias(b);
}

/*******************************************************/
/*
 * Recibe como parámetro el nombre del fichero de entrenamiento
 * (trainingFileName), el diccionario para palabras conocidas (dKnown),
 * el número del modelo a entrenar (numModel), la dirección en la cual
 * se realiza el entrenamiento (direction), el número de frases del
 * corpus (numSent), el número de palabras del corpus (numWords) y
 * el número de fragmentos en que se ha de dividir el corpus para
 * entrenar palabras desconocidas (numChunks).
 *
 * Este método selecciona ejemplo y realiza el entrenamiento del modelo
 */
void learner::learnerTrainModel(char *trainingFileName, dictionary *dKnown,int numModel, int direction, int numSent,int numWords, int numChunks)
{
  FILE *fKnownRL,*fUnknownRL,*fUnknownLR,*fKnownLR;
  int contSentences=0,ret = 1;
  char name[200];

  //Carga las listas de atributos
  simpleList featureList,featureListUnk;
  sprintf(name,"%s.A%d",NAME,numModel);
  createFeatureList(name,&featureList);
  sprintf(name,"%s.A%d.UNK",NAME,numModel);
  createFeatureList(name,&featureListUnk);

  if (direction==LEFT_TO_RIGHT) learnerPrintMessage(numModel,-1,LEFT_TO_RIGHT,TRUE);
  else if (direction==RIGHT_TO_LEFT) learnerPrintMessage(numModel,-1,RIGHT_TO_LEFT,TRUE);
  else if (direction==LR_AND_RL) learnerPrintMessage(numModel,-1,LR_AND_RL,TRUE);

  //Abrimos los ficheros de ejemplos
  if (direction==RIGHT_TO_LEFT || direction==LR_AND_RL)
    {
      sprintf(name,"%s.M%d.RL.SAMPLES",NAME,numModel);
      fKnownRL=openFile(name,"w+");
      sprintf(name,"%s.UNK.M%d.RL.SAMPLES",NAME,numModel);
      fUnknownRL=openFile(name,"w+");
    }
  if (direction==LEFT_TO_RIGHT || direction==LR_AND_RL)
    {
      sprintf(name,"%s.M%d.LR.SAMPLES",NAME,numModel);
      fKnownLR=openFile(name,"w+");
      sprintf(name,"%s.UNK.M%d.LR.SAMPLES",NAME,numModel);
      fUnknownLR=openFile(name,"w+");
    }

  int chunkSize =  (numWords/numChunks) + 1;

  if (verbose == TRUE)
    {
      fprintf(stderr,"\n* X = %f :: CHUNKSIZE = %d :: CHUNKS = %d",X,chunkSize,numChunks);
      fprintf(stderr,"\n* ========================================================================");
    }

  int nWordsRL = 0, nWordsLR = 0;
  int inicioRL = -1, inicioLR = -1;

  //Para cada chunk
  for (int i=0; (ret>=0 && i<numChunks);i++)
    {
      ret=1;

      int is_end_of_chunk = FALSE;
      int is_end_of_sentence = FALSE;
      if ( verbose == TRUE ) fprintf(stderr,"\nChunk %d [ %d ] ",i+1,i*chunkSize);

      //Creamos  el diccionario para entrenar palabras desconocidas
      dictionary *dUnknown = new dictionary(trainingFileName,i*chunkSize, (i+1)*chunkSize - 1 );
      if (R!=NULL) dUnknown->dictRepairFromFile(R);
      else dUnknown->dictRepairHeuristic(DRATIO);
      char name[200];
      sprintf(name,"%s.DICT.%d",NAME,i);
      dUnknown->dictWrite(name);
      delete dUnknown;
      dUnknown  = new dictionary(name);

      //Si el modelo en 4 usaremos el mismo diccionario para conocidas y desconocidas
      dictionary *d_for_known = NULL;
      //if (numModel==4) dKnown = dUnknown;
      if ( numModel == 4 ) 
	{  d_for_known = dUnknown; }
      else { d_for_known = dKnown; }

      if ( verbose ==  TRUE ) fprintf(stderr,"\nExtracting features : ");

      nWordsLR = chunkSize;
      nWordsRL = chunkSize;

      //Mientras haya palabras por leer y no estemos al final del chunk
      while ( ret>=0 && is_end_of_chunk==FALSE )
	{
	  is_end_of_sentence = FALSE;
	  //Si es LR o LRL
	  if (direction==LEFT_TO_RIGHT || direction==LR_AND_RL)
	    {
	      fKnown = fKnownLR;  fUnknown = fUnknownLR;
	      //Recorremos el texto en sentido LR para seleccionar ejemplos
	      nWordsLR = learnerLeftToRight(&featureList,&featureListUnk, d_for_known, dUnknown, nWordsLR, inicioLR);
	      inicioLR = sw->getIndex()->ord;
	      if (sw->next() !=  0)
		{
		  is_end_of_sentence = TRUE;
		  inicioLR = -1;
		}
	    }
	  //Si es RL o LRL
	  if (direction==RIGHT_TO_LEFT || direction==LR_AND_RL)
	    {
	      fKnown = fKnownRL; fUnknown = fUnknownRL;
	      //Recorremos el texto en sentido RL para seleccionar ejemplos
	      nWordsRL = learnerRightToLeft(&featureList,&featureListUnk, d_for_known , dUnknown, nWordsRL, inicioRL);
	      inicioRL = sw->getIndex()->ord;
	      if ( sw->previous() != 0 )
		{
		  is_end_of_sentence = TRUE;
		  inicioRL = -1;
		}
	    }

	  contSentences++;

	  if ( verbose  == TRUE) showProcess(contSentences,0);
	  //Si es fin de frase recargamos la ventana
	  if ( is_end_of_sentence == TRUE )
	    {
	      sw->deleteList();
	      ret = sw->iniGeneric();
	    }
	  
	  //Si hemos recorrido todas la palabras del chunk es fin de chunk
	  if ( nWordsRL <= 0 || nWordsLR <= 0 )
	    {
	      is_end_of_chunk = TRUE;
	    }
	}
      if ( verbose  == TRUE) showProcess(contSentences,1);
      delete dUnknown;

      //Si es necesario borramos los ficheros temporales
      char cmd[200];
      sprintf(cmd, "rm -f %s",name);
      if (REMOVE_FILES == TRUE) system(cmd);
    }

  //Creamos una copia de la lista de etiquetas para palabras conocidas
  simpleList *copyOfUNKP_L = learnerTransformHashInList(learnerUNKP_H);
  //para cada tipo known o unk
  //para cada direccion
  if (direction==LEFT_TO_RIGHT || direction==LR_AND_RL)
    {
      //Realizamos el entrenamiento
      learnerDoLearn(fKnownLR,numModel,LEFT_TO_RIGHT,KNOWN,dKnown,learnerAMBP_L);
      learnerDoLearn(fUnknownLR,numModel,LEFT_TO_RIGHT,UNKNOWN,dKnown,copyOfUNKP_L);
      fclose (fKnownLR);
      fclose (fUnknownLR);
    }

  if (direction==RIGHT_TO_LEFT || direction==LR_AND_RL)
    {
      //Realizamos el entrenamiento
      learnerDoLearn(fKnownRL,numModel,RIGHT_TO_LEFT,KNOWN,dKnown,learnerAMBP_L);
      learnerDoLearn(fUnknownRL,numModel,RIGHT_TO_LEFT,UNKNOWN,dKnown,copyOfUNKP_L);
      fclose(fKnownRL);
      fclose(fUnknownRL);
    }
  //fin para cada direccion
  //fin para cada tipo
  
  destroyFeatureList(&featureList,featureList.numElements());
  destroyFeatureList(&featureListUnk,featureListUnk.numElements());
}

/**********************************************************/

/*
 * Recibe como parámetros: wrd un string con una palabra,  
 * pos un string con una etiqueta, Known_or_Unknown si la 
 * palabra es conocida o desconocida. Si la palabra es conocida 
 * devuelve TRUE si la etiqueta pos está en la lista de etiquetas 
 * morfosintácticas ambiguas. Si la palabra es desconocida 
 * devuelve TRUE si la etiqueta pos pertenece a lista de posibles 
 * categorías morfosintácticas para palabras desconocidas. En cualquier 
 * otro caso devuelve FALSE. 
 */
int learner::learnerIsPossiblePOS(char *wrd, char *pos,int Known_or_Unknown)
{
  if (Known_or_Unknown==KNOWN)
    {
        if (HASH_FAIL != hash_lookup(learnerAMBP_H,pos))
	    {
		return TRUE;
	    }
    }
  else if (Known_or_Unknown==UNKNOWN)
    {
      if (HASH_FAIL != hash_lookup(learnerUNKP_H,pos))
      {
	 return TRUE;
      }
    }
  return FALSE;
}

/**************************************************************/
/*
 * Recibe como parámetros: 
 *	wrd un string con una palabra, 
 *	Known_or_Unknown un entero que indica si la palabra 
 *			 es conocida o desconocida y 
 *	un apuntador d al diccionario. Devuelve un apuntador a una lista. 
 */
simpleList *learner::learnerGetPotser(char *wrd, int Known_or_Unknown, dictionary *d)
{
  int stop=FALSE,ret=1;
  int w = d->getElement(wrd);
  infoDict *pInfoDict;
  simpleList *lout = new simpleList;

  //Si es conocida recoge las posibles etiquetas que contiene el diccionario 
  //para la palabra wrd  y mira si existen en la lista de categorías
  //morfosintácticas ambiguas. Si existen en la lista de etiquetas ambiguas 
  //las añade a la lista de salida, en caso contrario si no esta es posible 
  //etiqueta ambigua y es la etiqueta más frecuente se devuelve una lista 
  //sólo con la etiqueta más frecuente.
  if (Known_or_Unknown==KNOWN)
    {
      if (w != HASH_FAIL)
	{
	  simpleList *list = (simpleList *) d->getElementMaybe(w);
	  list->setFirst();
	  while (ret >= 0 && stop == FALSE)
	    {
	      pInfoDict = (infoDict *) list->getIndex();

	      if (HASH_FAIL!=hash_lookup(learnerAMBP_H,pInfoDict->txt))
	        {
		  infoDict *ptr = new infoDict;
 		  strcpy(ptr->txt,pInfoDict->txt);
		  ptr->num = pInfoDict->num;
		  lout->add(ptr);
	        }
	        else if ( strcmp(d->getMFT(w),pInfoDict->txt)==0 ) //si es most frequent tag
	        {
		  d->dictCleanListInfoDict(lout,lout->numElements());
		  lout->deleteList();
		  infoDict *ptr = new infoDict;
		  strcpy(ptr->txt,pInfoDict->txt);
		  ptr->num = pInfoDict->num;
		  lout->add(ptr);
		  stop = TRUE;
	        }
	      ret=list->next();
	    }
	  list->setFirst();
	}
    }
  //Si la palabra es desconocida la lista de salida contiene todas las
  //categorías morfosintácticas posibles desconocidas. 
  else	if (Known_or_Unknown==UNKNOWN)
    {
	 learnerUNKP_L->setFirst();
         while (ret>=0)
         {
	   pInfoDict =(infoDict *) learnerUNKP_L->getIndex();
	   infoDict *ptr = new infoDict;
 	   strcpy(ptr->txt,pInfoDict->txt);
	   ptr->num = pInfoDict->num;
	   lout->add(ptr);

	   ret=learnerUNKP_L->next();
         }
         learnerUNKP_L->setFirst();
      }

  return lout;
}


/**********************************************************/
/*
 * Recibe como parámetros: 
 * un apuntador al depósito de pesos (wr), 
 * un apuntador a mapping (m), 
 * un string con la etiqueta (pos) que estamos entrenando y 
 * el nombre del fichero  (fileName) en el cual se almacenan las SVs
 * para la etiqueta indicada. 
 *
 * Partiendo de estos datos recorre el fichero fileName,  traduciendo
 * cada uno de los atributos leídos gracias al mapping (m). 
 * Y añadiendo la correspondiente pareja etiqueta/atributo 
 * al depósito de pesos wr mediante el método wrAdd.
 *
 * Se devuelve el apuntador al depósito de pesos (weightRepository) 
 * que ha sido modificado.
 */
weightRepository *learner::learnerBuiltWeightRepository(weightRepository *wr,mapping *m,char *pos,char *fileName)
{
  char str[200]="";
  char *key;
  FILE *f = openFile(fileName,"r");
  int ret=1,trobat=FALSE;

  while (!feof(f) && ret>=0)
    {
      if (trobat==FALSE)
	{
	  ret= readTo(f,'\n',0,str);
	  if (strstr(str,"threshold")!=NULL) trobat=TRUE;
	}
      else
	{
	  memset(str,0,strlen(str));
	  ret = readTo(f,' ',0,str);
	  long double ld;

	  ld  = atof(str);

	  while (ret>0)
	    {
	      ret = readTo(f,':','\n',str);
	      if (ret>0)
		{
		  key = m->mappingGetFeatureByNumber(str);
		  ret = readTo(f,' ','\n',str);
		  if ((uintptr_t)key != HASH_FAIL) wr->wrAdd(key,pos,ld);
		}
 	    }

	}
    }
  fclose (f);
  return wr;
}

/**************************************************************/

void learner::learnerDestroyBias(hash_t *h)
{
  weight_node_t *aux;

  hash_t *tptr = h;
  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size;

  old_bucket=tptr->bucket;
  old_size=tptr->size;

  //Recorre cada lista de sinónimos del hash eliminando su contenido
  for (int i=0; i<old_size; i++)
    {
      old_hash=old_bucket[i];
      while(old_hash)
	{
	  tmp=old_hash;
	  old_hash=old_hash->next;

	  aux = (weight_node_t *) tmp->data;

	  delete aux;
	  aux = NULL;
	} /* while */
    } /* for */

  hash_destroy(h);
}

/**************************************************************/

/* 
 * Recibe como parámetros: 
 * Un apuntador a hash_t (h) que es el lugar donde almacenaremos los sesgos, 
 * la etiqueta (pos) que estamos entrenando y el fichero (fileName) de salida
 * de SVM-light en el cual se encuentran los datos esperados.
 * Este método lee el sesgo del fichero y lo añade (hash_insert) 
 * al hashing h para la etiqueta pos indicada. 
 * Posteriormente devuelve el apuntador al hashing modificado. 
 */
hash_t *learner::learnerBuiltBias(hash_t *h,char *pos,char *fileName)
{
  char str[200]="";
  FILE *f = openFile(fileName ,"r");
  int ret=1,trobat=FALSE;

  while (!feof(f) && trobat==FALSE && ret>=0)
    {
      ret= readTo(f,'\n',0,str);
      if (strstr(str,"threshold")!=NULL) trobat=TRUE;
    }

  char bias[200];
  int sortir = FALSE;
  strcpy(bias,"");
  for (int i=0;(i<strlen(str) && sortir==FALSE) ;i++)
    {
      if (str[i]==' ' || str[i]=='#') sortir == TRUE;
      else sprintf(bias,"%s%c",bias,str[i]);
    }

  weight_node_t *w = new weight_node_t;
  strcpy(w->pos,pos);
  w->data = (long double)0;
  w->data = atof (bias);
  hash_insert(h,w->pos,(uintptr_t)w);

  fclose(f);
  return h;
}

/********************************************************/

/*
 * Recibe como parámetro el nombre del fichero de configuración (train).
 * Ejecuta el aprendizaje 
 */
void learner::learnerRun(char *train)
{
  int iWrd=0,iSent=0;

  init_stack(&DO);
  //Leemos el fichero de configuración
  read_config_file(train);
  //obtener tamaño del corpus
  learnerCount(TRAINSET,&iWrd,&iSent);

  if ( verbose == TRUE )
    {
      fprintf(stderr,"\n* trainset # words        = [ %d ]",iWrd);
      fprintf(stderr,"\n* trainset # sentences    = [ %d ]",iSent);
      fprintf(stderr,"\n* ========================================================================");
      fprintf(stderr,"\n\n* ========================================================================");
      fprintf(stderr,"\n* PREPARING TRAINING");
      fprintf(stderr,"\n* ========================================================================");
    }
  struct  tms tbuff1,tbuff2;
  clock_t start,end;
  start = times(&tbuff1);

  //Creamos el diccionario
  char name[200];
  sprintf(name,"%s.DICT",NAME);
  dictionary *d = new dictionary(TRAINSET, 0,0);
  if (R!=NULL) d->dictRepairFromFile(R); // "/mnt/hda4/pfc/WSJ.200");
	else d->dictRepairHeuristic(DRATIO);
  d->dictWrite(name);
  delete d;
  d  = new dictionary(name);

  //Obtenemos las listas de etiquetas
  if (learnerAMBP_H == NULL)
    learnerAMBP_H = d->dictFindAmbP(&learnerNumAMBP);
  if (learnerUNKP_H == NULL)
    learnerUNKP_H = d->dictFindUnkP(&learnerNumUNKP);
  //Creamos los ficheros de etiquetas
  learnerCreatePOSFile(NAME,TRUE,learnerAMBP_H);
  learnerCreatePOSFile(NAME,FALSE,learnerUNKP_H);
  //Creamos unos hashings con las listas de etiquetas
  learnerAMBP_L = learnerTransformHashInList(learnerAMBP_H);
  learnerUNKP_L = learnerTransformHashInList(learnerUNKP_H);

  //creamos el fichero de configuración de la ventana
  learnerCreateDefaultFile(NAME,"WIN");

  //Calculamos en numero de chunks
  int chunks = learnerNumChunks(TRAINSET,X,iSent);
  //Mientras haya modelos por entrenar
  while (!empty(&DO))
    {
      int *numModel = (int *) pop(&DO);
      int *direction = (int *) pop(&DO);
      
	//Eliminamos los ficheros anteriores
      removeFiles(NAME, RM_MODEL_FILES,*numModel, *direction, verbose);
      removeFiles(NAME, RM_TEMP_FILES ,*numModel, *direction, verbose);

      sprintf(name,"A%d",*numModel);
      learnerCreateDefaultFile(NAME,name);

	//Creamos la ventana
      FILE *f = fopen(TRAINSET, "r");
      sw = new swindow(f,WINDOW_SIZE,CORE_POSITION);
      //ejecutamos el entrenamiento para el modelo
      learnerTrainModel(TRAINSET,d,*numModel,*direction,iSent,iWrd,chunks);
      delete sw;
    }
  delete d;

  end = times(&tbuff2);
  if ( verbose == TRUE )
    {
      fprintf(stderr,"\n\n* ========================================================================\n");
      showTime ("* SVM-light Time",  time_svmlight, time_svmlight, 0);
      showTime ("* SVMTlearn Time",
		((double)(end-start))/CLOCKS_PER_SECOND - time_svmlight, //CLK_TCK,
		((double)tbuff2.tms_utime-(double)tbuff1.tms_utime)/CLOCKS_PER_SECOND,
		((double)tbuff2.tms_stime-(double)tbuff1.tms_stime)/CLOCKS_PER_SECOND);
      showTime ("* Total Learning Time",
		time_svmlight + ((double)(end-start))/CLOCKS_PER_SECOND,
		time_svmlight + ((double)tbuff2.tms_utime-(double)tbuff1.tms_utime)/CLOCKS_PER_SECOND,
		((double)tbuff2.tms_stime-(double)tbuff1.tms_stime)/CLOCKS_PER_SECOND);
      fprintf(stderr,"* ========================================================================\n\n");
    }
}

/**************************************************************/

/*
 * Recorre el texto de izquierda a derecha seleccionando ejemplos
 */
int learner::learnerLeftToRight(simpleList *featureList, simpleList *featureListUnk, dictionary *dKnown, dictionary *dUnknown, int numWrds, int inicio)
{
  int ret = 0;

  if ( inicio == -1 ) while (sw->previous()==0);
  else if (sw->getIndex()->ord!=inicio)
    {
      while(ret == 0 && sw->getIndex()->ord != inicio)
      {
	if (inicio < sw->getIndex()->ord ) ret = sw->previous();
	if (inicio > sw->getIndex()->ord ) ret = sw->next();
      }
    }
  nodo *elem = sw->getIndex();
  numWrds--;
  learnerGenerateFeatures(elem,featureList,dKnown, LEFT_TO_RIGHT);
  learnerGenerateFeaturesUnk(elem,featureListUnk,dKnown, dUnknown, LEFT_TO_RIGHT);

  while(numWrds>=0)
    {
      if ( sw->next() != 0 ) return numWrds;

      elem = sw->getIndex();
      numWrds--;
      learnerGenerateFeatures(elem,featureList,dKnown,LEFT_TO_RIGHT);
      learnerGenerateFeaturesUnk(elem,featureListUnk,dKnown, dUnknown, LEFT_TO_RIGHT);
    }

   return (numWrds);
}

/**************************************************************/

/*
 * Recorre el texto de derecha a izquierda seleccionando ejemplos
 */
int learner::learnerRightToLeft(simpleList *featureList, simpleList *featureListUnk, dictionary *dKnown, dictionary *dUnknown, int numWrds, int inicio)
{
  int ret = 0;

  if ( inicio == -1 ) while (sw->next()==0);
  else  if ( sw->getIndex()->ord != inicio )
    {
     while(ret == 0 && sw->getIndex()->ord != inicio)
      {
	if (inicio < sw->getIndex()->ord ) ret = sw->previous();
	if (inicio > sw->getIndex()->ord ) ret = sw->next();
      }
    }

  nodo *elem = sw->getIndex();
  numWrds--;
  learnerGenerateFeatures(elem,featureList,dKnown,RIGHT_TO_LEFT);
  learnerGenerateFeaturesUnk(elem,featureListUnk,dKnown,dUnknown,RIGHT_TO_LEFT);

  while( numWrds>=0 )
    {
      if ( sw->previous() != 0 ) return numWrds;

      elem = sw->getIndex();
      numWrds--;
      learnerGenerateFeatures(elem,featureList,dKnown, RIGHT_TO_LEFT);
      learnerGenerateFeaturesUnk(elem,featureListUnk,dKnown, dUnknown, RIGHT_TO_LEFT);
    }

  return numWrds;
}

/**************************************************************/
/*
 * Esta función recibe como parámetros: 
 * el apuntador a un nodo de la ventana (elem),
 * una pila donde apilara los atributos generados (stk), 
 * la lista de atributos que debe generar (featureList), 
 * el diccionario con la información necesaria para el cálculo de features (d) 
 * y  la dirección en que se recorre el corpus (direction). 	
 * Recorre la lista featureList y ejecuta los métodos necesarios 
 * de la ventana (swindow) para generar los atributos y que al final 
 * de la ejecución de este método esten apilados en stk.
*/
void learner::learnerGetFeatures(nodo *elem, stack_t *stk,dictionary *d, simpleList *featureList, int direction)
{
  nodo_feature_list *aux = NULL;
  int ret = 1;
  //Recorre la lista de atributos y crea los atributos correspondientes
  while (ret>=0)
    {
      aux = (nodo_feature_list *) featureList->getIndex();
      if (strcmp(aux->mark,SLASTW)==0)  sw->winPushSwnFeature(stk);
      else if (strcmp(aux->mark,WMARK)==0)  sw->winPushWordFeature((void *)aux,d,stk,direction);
      else if (strcmp(aux->mark,KMARK)==0)  sw->winPushAmbiguityFeature((void *)aux,d,stk,direction);
      else if (strcmp(aux->mark,MMARK)==0)  sw->winPushMaybeFeature((void *)aux,d,stk,direction);
      else if (strcmp(aux->mark,PMARK)==0)  sw->winPushPosFeature((void *)aux,d,stk,direction);
      else if (strcmp(aux->mark,MFTMARK)==0)  sw->winPushMFTFeature((void *)aux,d,stk,direction);
      else
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
}

/**************************************************************/

/*
 * El objetivo de este método es seleccionar o descartar una palabra para
 * realizar en el entrenamiento de palabras desconocidas, calcular respectivos
 * atributos e insertar esta información en el fichero de ejemplos correspondiente. 
 */
void learner::learnerGenerateFeaturesUnk(nodo *elem, simpleList *featureList,dictionary *d, dictionary *dUnk, int direction)
{
  stack_t stk;
  nodo_feature_list *aux;
  int is_selected = FALSE;
  char *feature = NULL;
  char buffer[1000];

  strcpy(buffer,"");

  if (d==NULL || elem==NULL || featureList==NULL) return;

  init_stack(&stk);

  sprintf(buffer,"%s:%s",elem->wrd,elem->comment);

  int i = dUnk->getElement(elem->wrd);
  if (i!=HASH_FAIL)
    {
      /*
      int i2 = d->getElement(elem->wrd);
      if (dUnk->getElementNumMaybe(i) == 1 && hash_lookup(learnerUNKP_H,d->getMFT(i2))!=HASH_FAIL )
      {
            fprintf(fUnknown,buffer,strlen(buffer));
	    is_selected = TRUE;
      }
    */
    }
  else
    {
 	  fprintf(fUnknown,buffer,strlen(buffer));
	  is_selected = TRUE;
    }

  if ( is_selected == TRUE) learnerGetFeatures(elem, &stk,dUnk, featureList, direction );

  while (!empty(&stk) && is_selected == TRUE)
    {
      feature = (char *) pop(&stk);
      sprintf(buffer," %s",feature);
      fprintf(fUnknown,buffer,strlen(buffer)+1);
      delete feature;
    }

  if (is_selected == TRUE) fprintf(fUnknown,"\n");

  strcpy(elem->pos,elem->comment);
 }

/**************************************************************/

/*
 * El objetivo de este método es seleccionar o descartar una palabra para
 * realizar en el entrenamiento de palabras conocidas, calcular respectivos
 * atributos e insertar esta información en el fichero de ejemplos correspondiente. 
 */
void learner::learnerGenerateFeatures(nodo *elem, simpleList *featureList,dictionary *d, int direction)
{
  stack_t stk;
  nodo_feature_list *aux;
  int is_selected = FALSE;
  int is_unk = FALSE;
  char *feature,buffer[1000]="";

  if (d==NULL || elem==NULL || featureList==NULL) return;

  init_stack(&stk);

  sprintf(buffer,"%s:%s",elem->wrd,elem->comment);

  int i = d->getElement(elem->wrd);
  if (i!=HASH_FAIL)
    {

      if ( d->getElementNumMaybe(i)>1 && hash_lookup(learnerAMBP_H,d->getMFT(i))!=HASH_FAIL )
	{
	  fprintf(fKnown,buffer,strlen(buffer));
	  is_selected = TRUE;
	}

    }

  if ( is_selected == TRUE) learnerGetFeatures(elem, &stk,d, featureList, direction );

  while (!empty(&stk) && is_selected == TRUE)
    {
      feature = (char *) pop(&stk);
      sprintf(buffer," %s",feature);
      /*  if (i!=HASH_FAIL)
	  {*/
	  fprintf(fKnown,buffer,strlen(buffer)+1);

      delete feature;
    }

  sprintf(buffer,"\n");

  if ( is_selected == TRUE)     fprintf(fKnown,buffer,strlen(buffer));

  strcpy(elem->pos,elem->comment);
}

/************************************************************/
/*
 * Ejecuta SVM-light. Recibe como parámetros cuatro cadenas de 
 * caracteres: svmdir es el directorio en el que se encuentra SVM-light,  
 * options son las opciones con que se lanzará SVM-light, posFile el nombre
 * del fichero de ejemplos usado como entrada para la herramienta 
 * de Joachims y, por último, outFile que es el nombre del fichero 
 * de salida.  Esta función devuelve 0.
*/
int learner::learnerExecSVMlight(char *svmdir, char *options, char *posFile, char *outFile)
{
  time_t begin, finish;

  begin = time (0);

  char command[500]="";
  float c;
  strcpy(command,"");
  sprintf(command,"%s/svm_learn -v 0  %s %s %s",svmdir,options,posFile,outFile);  
  
  if ( verbose == TRUE ) fprintf(stderr,"Executing Joachims svm_light [ with options: %s ]  ",options);
  system(command);
  if ( verbose == TRUE ) fprintf(stderr," [DONE]");

  finish = time(0);

  time_svmlight = difftime(finish,begin) +  time_svmlight;

  return 0;
}

/**************************************************************/

simpleList *learner::learnerTransformHashInList(hash_t *tptr)
{
  hash_node_t *node, *last;
  int i;
  simpleList *l = new simpleList;

  for (i=0; i<tptr->size; i++)
    {
      node = tptr->bucket[i];
      while (node != NULL)
	{
	  last = node;
	  node = node->next;
	  infoDict *p = (infoDict *) last->data;
	  l->add( (void *)last->data);
	}
    }
  l->setFirst();
  return l;
}

/**************************************************************/

/*
 * Calcularemos el número de fragmentos en el que se ha de dividir 
 * el corpus para conseguir un porcentaje de palabras desconocidas 
 * determinado. Lo parámetros de entrada son el nombre del fichero 
 * de entrenamiento (trainingFileName), el porcentaje de palabras 
 * desconocidas deseado (percentage) y el número de frases del corpus 
 * (nSentences). El valor devuelto es un entero indicando el número de
 * fragmentos. Si el número de fragmentos calculado es mayor que el número
 * de frases que contiene el corpus de entrenamiento se devolverá como el 
 * número de frases (nSentences).
 */
int learner::learnerNumChunks(char *trainingFileName,float percentage,int nSentences)
{
  int ret=0,ndwords=0,nwords=0;
  char wrd[500];

  FILE *f = openFile (trainingFileName,"r");
  hash_t h1;
  hash_init(&h1,10000);
  while (!feof(f))
    {
      ret = readTo(f,' ','\n',wrd);
      if (ret>=0)
	{
	  nwords++;
	  char *w = new char[strlen(wrd)+1];
	  strcpy(w,wrd);
	  if ((uintptr_t)hash_insert(&h1,w,(uintptr_t) w)==HASH_FAIL) ndwords++;
	  if (ret>0) readTo(f,'\n','\n',wrd);
	}
    }

   //Read again until a certain point where X is met   --> $ndwords * (100 - $X) / 100;
   float meeting = ndwords * (100 - X ) /100;
   int nwords2=0,ndwords2=0;
   fseek(f,0,SEEK_SET);
   hash_t h2;
   hash_init(&h2,10000);
   while (!feof(f) && ndwords2<meeting)
    {
      ret = readTo(f,' ','\n',wrd);
      if (ret>=0)
	{
	  nwords2++;
	  char *w = new char[strlen(wrd)+1];
	  strcpy(w,wrd);
	  if (hash_insert(&h2,w,(uintptr_t) w)==HASH_FAIL) ndwords2++;
	  if (ret>0) readTo(f,'\n','\n',wrd);
	}
    }

   int chunks = nwords/(nwords - nwords2);
   if (nSentences<=chunks) chunks = nSentences;

   fclose (f);

   hash_destroy(&h2);
   hash_destroy(&h1);
   return chunks;
}

/**************************************************************/

int learner::learnerIsInsideList(simpleList *l, char *key)
{
  if (l==NULL || key==NULL || strcmp(key,"")==0) return FALSE;
  int ret = 0;
  while (ret>=0)
    {
      infoDict *ptr = (infoDict *)l->getIndex();
      if (strcmp(key,ptr->txt)==0)
       {
	 return TRUE;
       }
    }
  l->setFirst();
  return FALSE;
}
