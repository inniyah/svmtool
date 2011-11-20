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
#include "common.h"

int verbose_svmtool = FALSE;

/**************************************************/

/*
 * FILE *openFile(const char *name, const char mode[])
 * Abre un fichero con el nombre <name> y en el modo <mode> 
 * (r lectura, w escritura, a actualización ...).
 * Devuelve el puntero al fichero
 * En caso de no poder abrir un fichero, termina la ejecucion
 */
FILE *openFile(const char *name, const char mode[])
{
    FILE *f;
    if ((f = fopen(name, mode))== NULL)
    {
		fprintf(stderr, "Error opening file: %s\n",name);
		exit(0);
    }
    return f;
}

/**************************************************/

void generateFileName(const char *name, const char *added,int numModel, int direction, int what, const char *type, char *out)
{
	strcpy(out,name);
	if (strcmp(added,"")!=0 && added!=NULL)
	  { 
	    sprintf(out,"%s.",out);
	    for (int i=0; i<strlen(added); i++)
	      {
		if (added[i]==':') sprintf(out,"%s%s",out,"DOSPUNTS");
		else if (added[i]=='`')  sprintf(out,"%s%s",out,"COMETA2");
		else if (added[i]=='\'') sprintf(out,"%s%s",out,"COMETA");
		else sprintf(out,"%s%c",out,added[i]);
		
	      }

	}
	if (what==UNKNOWN) 	sprintf(out,"%s.UNK",out);
	else sprintf(out,"%s",out);
	if (numModel>=0) sprintf(out,"%s.M%d",out,numModel);
	if (direction==LEFT_TO_RIGHT) sprintf(out,"%s.LR",out);
	else if (direction==RIGHT_TO_LEFT) sprintf(out,"%s.RL",out);
	if (type!=NULL) sprintf(out,"%s.%s",out,type);
}

/**************************************************/

void showProcess(int num,int isEnd)
{
  if (isEnd) { fprintf(stderr,".%d sentences [DONE]\n\n",num); return; }
  else if (num%100==0) fprintf(stderr,"%d",num);
  else if (num%10==0) fprintf(stderr,".");
}

/**************************************************/

void showProcessDone(int num,int freq, int isEnd, const char *what)
{
  if (isEnd) { fprintf(stderr,".%d %s [DONE]\n",num,what); return; }
  else if (num%freq==0) fprintf(stderr,".");
}

/**************************************************/

/* 
 * int goToWord(FILE *f, int offset) 
 * Lee <offset> lineas del canal o fichero <f>
 * Retorna -1 si encuentra eof
 * retorna el numero de lineas leidas si todo va bien         
 */
int goToWord(FILE *f, int offset)
{
	int cont=0;

	while (!feof(f) && cont<offset)
	{
	   if (fgetc(f)=='\n') cont++;
	}
	
	if (feof(f)) return -1;
	return cont;
}

/**************************************************/

/*
 * int readString(FILE *f, char *out)
 * Lee un String entre espacios o espacios y fines de linea
 *    FILE *f - es fichero o canal de donde leeremos el String
 *    char *out - es un parametro de salida, contendra el 
 *                String que leeremos
 * Retorna -1 si encuentra eof
 * retorna  0 si todo va bien
 */
int readString(FILE *f, char *out)
{
  if (feof(f)) return -1;
  
  char c = fgetc(f);
  strcpy(out,"");
  
  while (!feof(f) && c==' ' && c=='\n') c=fgetc(f);
  
  while (!feof(f) && c!=' ' && c!='\n')
   { 
     sprintf(out,"%s%c",out,c);
     c=fgetc(f);
   }
   if (feof(f) && strlen(out)==0) return -1;
   return 0;	
}

/**************************************************/

/*
 * int readTo(FILE *f, char endChar, chas endLine, char *out)
 * Lee un del canal o fichero <f>, el String leido sera devuelto como el
 * parametro de salida <out>. Para leer el String se leera hasta encontrar
 * el <endChar> o el caracter <endLine>
 * Retorna 0 si encuentra <endLine> 
 * retorna -1 si eof
 * retorn 1 si todo va bien y encuentra <endChar>
 */
int readTo(FILE *f, char endChar, char endLine, char *out)
{
  strcpy(out,"");
  char c = endChar+1;
  while (!feof(f) && c!=endChar && (endLine==0 || c!=endLine))
    { 
      c=fgetc(f);      
      if (c!=endChar && c!=endLine) sprintf(out,"%s%c",out,c);
    }
  if (feof(f)) return -1;
  if (c==endLine) return 0;
  return 1;  
}

/*******************************************************/

void qsort(int a[], int lo, int hi) {
      int h, l, p, t;

      if (lo < hi) {
         l = lo;
         h = hi;
         p = a[hi];

         do {
            while ((l < h) && (a[l] <= p))
               l = l+1;
            while ((h > l) && (a[h] >= p))
               h = h-1;
            if (l < h) {
               t = a[l];
               a[l] = a[h];
               a[h] = t;
            }
         } while (l < h);

         t = a[l];
         a[l] = a[hi];
         a[hi] = t;

         qsort(a, lo, l-1);
         qsort(a, l+1, hi);
      } // if
   }

/**************************************************/

void showTime(const char *what, double real, double utime, double stime)
{ 
  char message[200]="";
  sprintf(message,"%s: [ Real Time = %5.3lf secs.( %5.3lf usr + %5.3lf sys = %5.3lf CPU Time) ]\n",what,real,utime,stime,utime+stime);
  fprintf(stderr,"%s",message);
}

/**************************************************/

int buscarMenorEnString(char *szIn,char *szMenor,int *iMenor)
{
  char szString[10],szTemp[strlen(szIn)+1];
  int  iString;

  if (strcmp(szIn,"")==0 || szIn==NULL) return 1;

  strcpy(szTemp,szIn);
  if (*iMenor==-1)
      sscanf(szIn,"%s%d",szMenor,iMenor);
  else 
    {
      sscanf(szIn,"%s%d",szString,&iString);
      if (strcmp(szString,szMenor)<0)
	{
	  strcpy(szMenor,szString);
	  *iMenor = iString;
	}
    }

  int cont=0;
  int i;
  for (i=0; cont<2 && i<strlen(szTemp) ;i++)
    {
      if (szTemp[i]==' ') cont++;
    }

  return buscarMenorEnString(szTemp+i,szMenor,iMenor);
}

/**************************************************/

int ordenarStringPorParejas(char *szIn, char *szOut, int depth, char *szInicial)
{
	char szMenor[10],*p;
	char szTempMenor[10],szTemp[strlen(szIn)+1];
	int  i=0,iMenor = -1;
	
	// fprintf(stderr,"1 in: %s  out: %s\n",szIn,szOut);
	if (strcmp(szIn,"")==0 || szIn==NULL
	    || szInicial==NULL || szIn>(szInicial+strlen(szInicial))) return depth;
	if (depth==0) strcpy(szOut,"");
	
	buscarMenorEnString(szIn,szMenor,&iMenor);  
	sprintf(szTempMenor,"%s %d",szMenor,iMenor);
	p = strstr(szIn,szTempMenor);
	
	strcpy(szTemp,"");
	// Copiamos string szIn sin pareja menor
	while (i<strlen(szIn) && p!=NULL)
	{
	  if (&szIn[i]<p || &szIn[i]>(p+strlen(szTempMenor)))
	    {
	      sprintf(szTemp,"%s%c",szTemp,szIn[i]);
	    }
	  i++;
	}
	
	if (strlen(szOut)==0) sprintf(szOut,"%s %d",szMenor,iMenor);
	else  sprintf(szOut,"%s %s %d",szOut,szMenor,iMenor);
	
	return   ordenarStringPorParejas(szTemp,szOut,depth+1,szInicial);
}

/**************************************************/

int obtainMark(FILE *channel,char *mark, int es_primera)
{  
    int ret;
    strcpy(mark,"");
    while (strlen(mark)==0) ret = readTo(channel,'(','\n',mark);
 
    if (ret==-1) return -1;
    else return ret;    
}

/**************************************************/

int obtainAtrInt(FILE *channel,int *endAtr)
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

/**************************************************/

void destroyFeatureList(simpleList *fl,int nf)
{
  nodo_feature_list *data = NULL;

  fl->setFirst();
  if ( nf >= 1 ) //Si tiene mas de un maybe es ambigua
  {
	int ret  = 1;
	while (ret>=0)
	{
	  data = (nodo_feature_list *) fl->getIndex();
	  
	  data->l.setFirst();
	  if (data->l.numElements() >= 1)
	    {
	      int ret2 = 1;
	      while (ret2>=0)
		{
		  int *pInt = (int *)  data->l.getIndex();
		  delete pInt;
		  ret2 = data->l.next();
		}
	      data->l.setFirst();
	    } 
	  delete data;
	  ret=fl->next();
	}
	fl->setFirst();
  } /* if */
}

/**************************************************/

void createFeatureList(char *name,simpleList *featureList)
{
   int *i,endAtr,cont=0;
   char c;
   int ret = 1;
   //char temp[100];
   nodo_feature_list *data;

   FILE *f;
    if ((f = fopen(name, "rt"))== NULL)
    {
	fprintf(stderr, "Error opening file %s!!",name);
	exit(0);
    }

    //Insert feature Swn
    data = new nodo_feature_list; 
    strcpy(data->mark,"Swn"); 
    data->n = 0;
    featureList->add(data);

    char temp[10];
    ret = obtainMark(f,temp,TRUE);
    while (ret!=-1)
    {
        data = new nodo_feature_list;
	strcpy(data->mark,temp);
		
	endAtr=0;
	cont=0;
	
	while (endAtr==0 && ret!=0)
	{
	    i = new int;
	    *i = obtainAtrInt(f,&endAtr);
	    data->l.add(i);
	    cont++;
	}
	data->n = cont;
	featureList->add(data);
	strcpy(temp,"");
	ret = obtainMark(f,temp,FALSE);
    }
    fclose(f);      
}

/**************************************************/

void removeFiles(char *path, int type,int numModel, int direction, int verbose)
{
  char remove[200];
  switch (type)
    {
      case RM_TEMP_FILES: 
	if (verbose==TRUE) fprintf(stderr,"\nErasing temporal files.",numModel);
	sprintf(remove,"rm -f %s.M%d*.SVM",path,numModel);
	system(remove);
	sprintf(remove,"rm -f %s*M%d*.POS",path,numModel);
	system(remove);
	sprintf(remove,"rm -f %s*M%d*.SAMPLES",path,numModel);
	system(remove);
	sprintf(remove,"rm -f %s*M%d*.MAP",path,numModel);
	system(remove);
	sprintf(remove,"rm -f %s*DICT.*",path);
	system(remove);
	break;
     case RM_MODEL_FILES:
        if (direction==LEFT_TO_RIGHT || direction==LR_AND_RL)
	  {
	    if (verbose==TRUE) fprintf(stderr,"\nErasing old files for MODEL %d in LEFT TO RIGHT sense.",numModel);
	    sprintf(remove,"rm -f %s*M%d.LR.*",path,numModel);
	    system(remove);
	  }
        if (direction==RIGHT_TO_LEFT || direction==LR_AND_RL)
	  {
	    if (verbose==TRUE) 
	      fprintf(stderr,"\nErasing old files for MODEL %d in RIGHT TO LEFT sense.",numModel);
	    sprintf(remove,"rm -f %s*M%d.RL.*",path,numModel);
	    system(remove);
	  }
	    sprintf(remove,"rm -f %s*A%d.*",path,numModel);
	    system(remove);
        break;
    }
}

