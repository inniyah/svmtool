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
#include "hash.h"
#include "list.h"
#include "dict.h"
#include "swindow.h"
#include "common.h"
#include "er.h"


extern int verbose;

/**************************************************/

char *dictionary::getMFT(int w)
{
	if (w==HASH_FAIL) return NULL;

	int max=0,ret = 1;
	char *mft = new char[TAM_POS];
	simpleList *l = this->getElementMaybe(w);
	infoDict *ptr;

	l->setFirst();
	while (ret>=0)
	{
		ptr = (infoDict *)l->getIndex();
		if (max<ptr->num)
		{ strcpy(mft,ptr->txt);
			max = ptr->num;
		}
		ret = l->next();
	}
	l->setFirst();
return mft;
}

/**************************************************/

char *dictionary::getAmbiguityClass(int w)
{
	char *amb = new char[200];
	if (w==HASH_FAIL)
	{
	 sprintf(amb,"UNKNOWN");
	 return amb;
	}

	int ret = 1;
	strcpy(amb,"");
	simpleList *l = this->getElementMaybe(w);
	int numMaybe = this->getElementNumMaybe(w);
	infoDict *ptr;

	l->setFirst();
	while (ret>=0)
	{
	  numMaybe--;
  	  ptr = (infoDict *)l->getIndex();
	  // fprintf(stderr," %s %d",ptr->txt,ptr->num);
	  if (numMaybe>0) sprintf(amb,"%s%s_",amb,ptr->txt);
	  else sprintf(amb,"%s%s",amb,ptr->txt);
	  ret = l->next();
	}
	l->setFirst();
	return amb;
}

/**************************************************/

void dictionary::dictIncInfo(dataDict *elem, char *pos)
{
  int ret=1;
  infoDict *pInfoDict;

  elem->numWrd++;
  while (ret>=0)
    {
      pInfoDict = (infoDict *) elem->maybe.getIndex();
      if (strcmp(pInfoDict->txt,pos)==0)
	{
	  pInfoDict->num++;
	  elem->maybe.setFirst();
	  return;
	}
      ret=elem->maybe.next();
    }
  pInfoDict = new infoDict;
  strcpy(pInfoDict->txt,pos);
  pInfoDict->num=1;
  elem->maybe.add(pInfoDict);
  elem->numMaybe++;
  elem->maybe.setFirst();
}

/**************************************************/

void dictionary::dictWrite(char *outName)
{
  int ret=0;
  infoDict *data;
  dataDict *aux;
  int cont=0,contWords=0;
  char stringPOS[1000];

  FILE *f = openFile(outName,"w");

  hash_t *tptr = &d;

  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size, h, i;

  old_bucket=tptr->bucket;
  old_size=tptr->size;


  for (i=0; i<old_size; i++)
    {
      old_hash=old_bucket[i];
      while(old_hash)
	{
	  tmp=old_hash;
	  old_hash=old_hash->next;

	  aux = (dataDict *) tmp->data;
  	  //fprintf(stderr,"\n%s %d %d",aux->wrd,aux->numWrd,aux->numMaybe);
	  fprintf(f,"%s %d %d",aux->wrd,aux->numWrd,aux->numMaybe);

	  cont++;
	  contWords = aux->numWrd+contWords;
	  ret  = 1;
	  strcpy(stringPOS,"");
	  while (aux->numMaybe>0 && ret>=0)
	    {
	      data = (infoDict *) aux->maybe.getIndex();
	      //fprintf(stderr," [ %s %d ]",data->txt,data->num);

	      if (strlen(stringPOS)==0) sprintf(stringPOS,"%s %d",data->txt,data->num); 
	      else sprintf(stringPOS,"%s %s %d",stringPOS,data->txt,data->num);
	       
	      ret=aux->maybe.next();
	    }
	  
	  char *szOut = new char[strlen(stringPOS)+1];
	  ordenarStringPorParejas(stringPOS, szOut, 0, stringPOS);
	  fprintf(f," %s\n",szOut); 
	  
	  delete szOut;
	} /* while */
    } /* for */

  fclose(f);
  return;
}

/**************************************************/

void dictionary::dictCreate(FILE *f,int limitInf,int limitSup)
{
  int retW=0,retP=0,contWords=0,cont=0,contWordsAdded=0;
  infoDict *data;
  dataDict *aux,*aux2;
  nodo *elem;
  char wrd[200],pos[10];
  int no_chunk = FALSE;

  if ( verbose == TRUE ) fprintf(stderr,"\nCreating Dictionary");

  if (limitInf == 0 && limitSup == 0) no_chunk = TRUE;

  while (retP>=0 && retW>=0)
    {
      if ( verbose  == TRUE) showProcessDone(contWordsAdded, 1000, FALSE,"");
      retW = readString(f, wrd);
      char *real  = new char [strlen(wrd)+1];
      strcpy(real,wrd);
      retP = readString(f, pos);
      
      if (retW>=0 && retP>=0)
	{
	  int erRet=erLookRegExp(wrd);
	  switch (erRet)
  	   {
		case CARD: strcpy(wrd,"@CARD"); break;
		case CARDSEPS: strcpy(wrd,"@CARDSEPS"); break;
		case CARDPUNCT: strcpy(wrd,"@CARDPUNCT"); break;
		case CARDSUFFIX: strcpy(wrd,"@CARDSUFFIX"); break;
	   }

	  int is_valid_for_limit_inf =  ( (contWords < limitInf)  || contWords == 0 );
	  int is_valid_for_limit_sup =  (contWords >= limitSup );

	  if ( no_chunk == TRUE || is_valid_for_limit_inf || is_valid_for_limit_sup )
	    {      
	      if ((uintptr_t)(aux=(dataDict *)hash_lookup(&d,wrd)) == HASH_FAIL)
		{
		  aux= new dataDict;
		  strcpy(aux->wrd,wrd);
		  aux->numMaybe = 1;
		  aux->numWrd = 1;
		  data = new infoDict;
		  strcpy(data->txt,pos);
		  data->num=1;
		  aux->maybe.add(data);
		  hash_insert(&d,aux->wrd,(uintptr_t) aux);
		  cont++;
		}
	      else dictIncInfo(aux,pos);         
	      contWordsAdded++;
	           
	      if (strcmp(wrd,"@CARD")==0 || strcmp(wrd,"@CARDPUNCT")==0
		  || strcmp(wrd,"@CARDSEPS")==0 ||  strcmp(wrd,"@CARDSUFFIX")==0) 
		{
		  if ((uintptr_t)(aux2=(dataDict *)hash_lookup(&d,real)) == HASH_FAIL)
		  {
		    aux2 = new dataDict;
		    strcpy(aux2->wrd,real);
		    aux2->numMaybe = 1;
		    aux2->numWrd = 1;
		    data = new infoDict;
		    strcpy(data->txt,pos);
		    data->num = 1;
		    aux2->maybe.add(data);
		    hash_insert(&d,aux2->wrd,(uintptr_t) aux2);
		    cont++;		    
		  }
		  else dictIncInfo(aux2,pos);
		}	      
	    } 
	} 
	contWords++;
	delete real; 	  
    }

   if ( verbose == TRUE ) fprintf(stderr,"[ %d words ]",cont);
}

/**************************************************/

void dictionary::dictRepairFromFile(char *fileName)
{
  if ( verbose == TRUE ) fprintf(stderr,"\nReparing Dictionary with file < %s >",fileName);
  FILE *f = openFile(fileName,"r");

  char wrd[250],pos[10];
  int numWrd,numMaybe,numWrdxPOS;
  dataDict *aux;


  // Bucle para leer lista de palabras
  while (!feof(f))
    {
      fscanf(f,"%s %d %d",wrd,&numWrd,&numMaybe);
      int w = hash_lookup(&d,wrd);	
      if (w!=HASH_FAIL)
      {
	 aux = new dataDict;
	 strcpy(aux->wrd,wrd);
	 aux->numWrd = getElementNumWord(w); 
	 aux->numMaybe = 0; 
	 
	 simpleList *l = getElementMaybe(w);
		    
	 for (int i=0;i<numMaybe;i++)
	 {
		fscanf(f,"%s %d",pos,&numWrdxPOS);
		int ret=0;

		while (ret>=0)
		{
		  infoDict *ptr = (infoDict *)l->getIndex();
		  if (strcmp(pos,ptr->txt)==0) 
		     { 
		       //Copiamos  elemento a añadir
		       infoDict *tmpInfoDict = new infoDict;
		       strcpy(tmpInfoDict->txt,ptr->txt);
		       tmpInfoDict->num = ptr->num;
		             
		       aux->maybe.add(tmpInfoDict);
		       aux->numMaybe++;
		       ret = -1;
		       delete ptr; //Borrar substituido
		      }
		  else ret = l->next();
     
		}
	    l->setFirst();
	 }

	delete (dataDict *) hash_delete (&d,wrd); 
	hash_insert(&d,aux->wrd,(uintptr_t) aux);
      }
   }
   fclose(f);
}

/**************************************************/

void dictionary::dictRepairHeuristic(float dratio)
{
  hash_t *tptr = &d;
  hash_node_t *node, *last;
  int i;
 
  for (i=0; i<tptr->size; i++)
	{
		node = tptr->bucket[i];
		while (node != NULL)
		{
		   last = node;
		   node = node->next;

		   int ret=0;
		   dataDict *dd = (dataDict *)last->data;
		   
		   simpleList *l = &dd->maybe;
	           l->setFirst();
		   int iNumWrdsAfterDelete = dd->numWrd;
	           while (ret>=0)
	            {
		      infoDict *ptr = (infoDict *)l->getIndex();
		   
		      float fRange = (float) ptr->num / (float) dd->numWrd ;
		   
			if (fRange < dratio) 
			{ 			
			  dd->numMaybe--;
			  iNumWrdsAfterDelete =  iNumWrdsAfterDelete - ptr->num;
			  l->delIndex(); //Eliminar pos
			}
		      ret = l->next();
	            }
		  dd->numWrd =  iNumWrdsAfterDelete; 
	          l->setFirst();
		}
	}
}

/**************************************************/

int dictionary::readInt(FILE *in)
{	int i=0;
 char value[10];
 char c=' ';

 strcpy(value,"");

 while ((c==' ') && (!feof(in))) c=fgetc(in);
 while ((i<10) && (c!=' ') && (c!='\n') && (!feof(in)))
   {
     sprintf(value,"%s%c",value,c); 
     c=fgetc(in); i++;
   }
 return atoi(value);
}

/**************************************************/

infoDict *dictionary::readData(FILE *in)
{
  infoDict *data = new infoDict;
  char c=fgetc(in);
  int i = 0;

  strcpy(data->txt,"");

  while ( (i<TAMTXT) && (c!=' ' && c!='\n' && c!='\t') && (!feof(in)) )
    {
      sprintf(data->txt,"%s%c",data->txt,c); c=fgetc(in); i++;
    }
  data->num  = readInt(in);
  return data;
}

/**************************************************/

void dictionary::dictAddBackup(char *name)
{
  FILE *f = openFile(name,"r");
  
  char wrd[250],pos[10];
  int ret,i;
  dataDict *aux;
  infoDict *data;
  
  // Bucle para leer lista de palabras
  while (!feof(f))
    {
      data = readData(f);
      i = readInt(f);
      uintptr_t w = hash_lookup(&d,wrd);
      if (w==HASH_FAIL)
      {
	 aux = new dataDict;
	 strcpy(aux->wrd,data->txt);
	 aux->numWrd = 0; 
	 aux->numMaybe = 0;
      }		
      else  aux = (dataDict *) w;
	
      aux->numWrd += data->num;
      delete data;
      while (i>0)
      {
	  data = readData(f);
	  ret=1;
	  //Buscamos si ja existe en la lista.
	  for (int j=aux->numMaybe;ret>=0 &&  j>0; j--)
	    {
	      infoDict *element = (infoDict *)aux->maybe.getIndex();
	      if (strcmp(data->txt,element->txt)==0)
		{
		  ret = -1;
		  element->num += data->num; 
		}
	      else ret = aux->maybe.next();
	    }
	  //Si no encontrado lo añadimos a la lista
	  if (ret!=-1) 
	    { 
	      aux->maybe.add(data);
	      aux->numMaybe++;
	    }
	  else delete data;
	  i--;
	}
       if (w==HASH_FAIL) hash_insert(&d,aux->wrd,(uintptr_t) aux);
 
    } //End while not eof
   fclose(f);
}

/**************************************************/

void dictionary::dictLoad(FILE *in)
{
  char c='\0';
  char wrd[25]="";
  int i=0,number;
  dataDict *aux;
  infoDict *data;

  while (!feof(in))
    {
      data = readData(in);
      i = readInt(in);
      aux = new dataDict;

      strcpy(aux->wrd,data->txt);

      aux->numWrd = data->num;
      aux->numMaybe = i;    
      delete data;

      //Si leemos una línea con numMaybe = 0, es decir, sin
      //ninguna etiqueta no la cargaremos.
      if ( aux->numMaybe > 0) 
	{
	  while (i>0)
	    {
	      data = readData(in);
	      aux->maybe.add(data);

	      i--;
	    }
	  hash_insert(&d,aux->wrd,(uintptr_t) aux);
	}
      else delete aux;
    }
}

/**************************************************/

int dictionary::getElement(char *key)
{
  return hash_lookup(&d,key);
}

/**************************************************/

char *dictionary::getElementWord(uintptr_t ptr)
{
  dataDict *aux = (dataDict *) ptr;
  return aux->wrd;
}

/**************************************************/

int dictionary::getElementNumWord(uintptr_t ptr)
{
  dataDict *aux = (dataDict *) ptr;
  return aux->numWrd;
}

/**************************************************/

int dictionary::getElementNumMaybe(uintptr_t ptr)
{
  dataDict *aux = (dataDict *) ptr;
  return aux->numMaybe;
}

/**************************************************/

simpleList *dictionary::getElementMaybe(uintptr_t ptr)
{
  dataDict *aux = (dataDict *) ptr;
  return &aux->maybe;
}

/**************************************************/

dictionary::dictionary(char *name,char *backup)
{
  FILE *in = openFile(name,"r");
  hash_init(&d,1000);
  dictLoad(in);
  fclose(in);
  dictAddBackup(backup);
}

/**************************************************/

dictionary::dictionary(char *name)
{
  FILE *in = openFile(name,"r");
  hash_init(&d,1000);
  dictLoad(in);
  fclose(in);
}

/**************************************************/

dictionary::dictionary(char *name,int limInf, int limSup)
{
  FILE *in = openFile(name,"r");
  char str[200];
  hash_init(&d,1000);
  dictCreate(in,limInf,limSup);
  fclose(in);
}

/**************************************************/

void dictionary::dictCleanListInfoDict(simpleList *l, int num)
{
  infoDict *data = NULL;

	  l->setFirst();
  	  if ( num >= 1 ) //Si tiene mas de un maybe es ambigua
	  {
		int ret  = 1;
		while (ret>=0)
		{
		  data = (infoDict *) l->getIndex();
		  delete data;
		  ret=l->next();
		}
		l->setFirst();
	  } /* if */
}

/**************************************************/

dictionary::~dictionary()
{
  infoDict *data;
  dataDict *aux;
 
  //Recorrer cada entrada del diccionario eliminando el contenido de las listas
  hash_t *tptr = &d;

  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size, h, i;

  old_bucket=tptr->bucket;
  old_size=tptr->size;

  for (i=0; i<old_size; i++)
    {
      old_hash=old_bucket[i];
      while(old_hash)
	{
	  tmp=old_hash;
	  old_hash=old_hash->next;

	  aux = (dataDict *) tmp->data;
	  
	  dictCleanListInfoDict(&aux->maybe,aux->numMaybe);
	  delete aux;
	  aux = NULL;
	} /* while */
    } /* for */

  //Destruir hashing
  hash_destroy(&d);
}

/**************************************************/

hash_t *dictionary::dictFindAmbP(int *numPOS)
{
  int ret=0;
  infoDict *data;
  dataDict *aux;

  hash_t *ambp = new hash_t;
  hash_t *tptr = &d;
  hash_init(ambp,30);

  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size, h, i;

  old_bucket=tptr->bucket;
  old_size=tptr->size;

  *numPOS = 0;

  for (i=0; i<old_size; i++)
    {
      old_hash=old_bucket[i];
      while(old_hash)
	{
	  tmp=old_hash;
	  old_hash=old_hash->next;

	  aux = (dataDict *) tmp->data;
	  aux->maybe.setFirst();
  	  if (aux->numMaybe>1) //Si tiene mas de un maybe es ambigua
	  {
		ret  = 1;
		while (ret>=0)
		{
		data = (infoDict *) aux->maybe.getIndex();
		infoDict * tmp = new infoDict;
		strcpy(tmp->txt,data->txt);
		tmp->num = data->num;
		hash_insert(ambp,tmp->txt,(uintptr_t) tmp);

		*numPOS++;
		ret=aux->maybe.next();
		}
		aux->maybe.setFirst();
	  } /* if */
	} /* while */
    } /* for */
  return ambp;
}

/**************************************************/

hash_t *dictionary::dictFindUnkP(int *numPOS)
{
  int ret=0;
  infoDict *data;
  dataDict *aux;

  hash_t *unkp = new hash_t;
  hash_t *tptr = &d;
  hash_init(unkp,30);

  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size, h, i;

  old_bucket=tptr->bucket;
  old_size=tptr->size;

	*numPOS = 0;

  for (i=0; i<old_size; i++)
    {
      old_hash=old_bucket[i];
      while(old_hash)
	{
	  tmp=old_hash;
	  old_hash=old_hash->next;

	  aux = (dataDict *) tmp->data;
	  aux->maybe.setFirst();
	  if (aux->numWrd==1) //Si solo aparece una vez desconocida
	  {
		ret  = 1;
		while (ret>=0)
		{
		data = (infoDict *) aux->maybe.getIndex();
		infoDict * tmp = new infoDict;
		strcpy(tmp->txt,data->txt);
		tmp->num = data->num;
		hash_insert(unkp,tmp->txt,(uintptr_t) tmp);
		*numPOS++;
		ret=aux->maybe.next();
		}
		aux->maybe.setFirst();
	  } /* if */
	} /* while */
    } /* for */
  return unkp;
}

