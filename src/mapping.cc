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
#include "common.h"
#include "hash.h"
#include "mapping.h"

/**************************************************/

extern int verbose_svmtool;

/**************************************************/

/*
 * La clase mapping tiene como principal objetivo realizar 
 * la conversión de atributo a identificador  numérico y viceversa.
 * Este objeto se utiliza principalmente en tiempo de aprendizaje. 
 *
 * Los atributos que contiene la clase mapping son:
 * mapByKey	hash_t *	Hashing para permitir el acceso por atributo
 * mapByNumber	hash_t *	Hash que  nos permite acceso por identificador
 *				numérico o clave numérica (Numeric key)
 * mapping_counter	int	Contador de  elementos que contiene el mapping
 */

/**************************************************/

/*
 * Cada elemento insertado en el mapping es del tipo mapping_node_t. 
 * Este objeto está compuesto por:
 * 
 * feature	char *	Atributo almacenado
 * number	char *	Número identificativo de el atributo en formato alfanumérico
 * num	Int	Número de veces que aparece
 */
class mapping_node_t{
public:
  char *feature;
  char *number;
  int num;
  
  ~mapping_node_t()
  { 
    delete feature;
    delete number;
  }
};

/**************************************************/

mapping::mapping()
{
        mapByKey = new hash_t;
	mapByNumber = new hash_t;
	hash_init(mapByKey,100000);
	hash_init(mapByNumber,100000);
	mapping_counter = 1;
}

/**************************************************/

mapping::~mapping()
{
  // Se elimina hash de mapeo por Id. numerico
  hash_destroy(mapByNumber);
  delete mapByNumber;
  
  // se eliminan todos los elementos insertados
  int ret=0,numdel=0;
  mapping_node_t *aux;

  hash_t *tptr = mapByKey;
  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size, h, i;

  old_bucket=tptr->bucket;
  old_size=tptr->size;

  //Recorremos todas las listas de sinonimos el hash 
  //eliminando el contenido  
  for (i=0; i<old_size; i++)
    {
      old_hash=old_bucket[i];
      while(old_hash)
      {
	tmp=old_hash;
	old_hash=old_hash->next;
	aux = (mapping_node_t *) tmp->data;
	delete aux;
	
      	} /* while */
    } /* for */

  hash_destroy(mapByKey);
  delete mapByKey;
}

/**************************************************/
/*
 * Esta función añade un atributo al hashing. Recibe como parámetro
 * el atributo, key. Devuelve un entero, que es el número que 
 * identifica una palabra. 
 *
 * Para añadir un atributo al hash se comprueba si ya existe en  
 * el mapByKey. Si ya existe se incrementa el número de veces que 
 * aparece el atributo. Si no existe se asigna al atributo un número
 * secuencial y se inserta en el mapByKey  y en mapByNumber. 
 */
int mapping::mappingAddByKey(const char *key)
{
 	char strTmp[100];

 	mapping_node_t *tmp = (mapping_node_t *) hash_lookup(mapByKey,key);

 	if ((uintptr_t)tmp == HASH_FAIL)
	{
		tmp = new mapping_node_t;
		strcpy(strTmp,"");
		tmp->feature = new char[strlen(key)+1];
		strcpy(tmp->feature,key);
		sprintf(strTmp,"%d",mapping_counter); //bynum
		tmp->number = new char[strlen(strTmp)+1];
		strcpy(tmp->number,strTmp);
		mapping_counter++; //endbynum
		tmp->num=1;
		hash_insert(mapByKey, tmp->feature, (uintptr_t) tmp);
		hash_insert(mapByNumber, tmp->number, (uintptr_t) tmp); //add by num
	}
	else tmp->num++;
	return atoi(tmp->number);
}

/**************************************************/
/*
 * Parámetro de entrada el atributo, key, para la que se desea asignar 
 * e insertar un identificador numérico.  
 * Mira si existe la clave recibida como parámetro en mapByKey. 
 * Si no existe  se le asigna un identificador numérico  y se  almacena en mapByNumber. 
 * Devuelve -1 si no existe key, identificador numérico asignado en caso contrario. 
 */
int mapping::mappingAddNumber(const char *key)
{
 	char strTmp[100];
	int  ret = -1;
 	mapping_node_t *tmp = (mapping_node_t *) hash_lookup(mapByKey,key);

 	if ((uintptr_t)tmp != HASH_FAIL)
	{
	    	strcpy(strTmp,"");
	  	sprintf(strTmp,"%d",mapping_counter);
		tmp->number = new char[strlen(strTmp)+1];		
		strcpy(tmp->number,strTmp);
		mapping_counter++;
	  	hash_insert(mapByNumber, tmp->number, (uintptr_t) tmp);
		ret = atoi(tmp->number);
	}
	
	return ret; 
}

/****************************************************************************/
/* 
 * Busca el atributo, key, en el hashing correspondiente.
 * Devuelve HASH_FAIL si no se encuentra key. En caso contrario,
 * devuelve el identificador numérico. 
 */
int mapping::mappingGetNumberByFeature(const char *key)
{
	mapping_node_t *tmp = (mapping_node_t *)hash_lookup(mapByKey,key);
	if ( ((uintptr_t) tmp) == HASH_FAIL) return HASH_FAIL;
	return atoi(tmp->number);
}

/****************************************************************************/

/*
 * Busca el identificador numérico, key, en el hashing correspondiente.
 * Devuelve HASH_FAIL si no se encuentra key. En caso contrario, 
 * devuelve el atributo correspondiente. 
 */
char *mapping::mappingGetFeatureByNumber(const char *key)
{
	mapping_node_t *tmp = (mapping_node_t *) hash_lookup(mapByNumber,key);
	if ( ((uintptr_t) tmp) == HASH_FAIL) return (char *) HASH_FAIL;
	return tmp->feature;
}

/****************************************************************************/

/*
 * Esta función recibe como parámetros el tamaño máximo de entradas 
 * que puede tener el mapping (max_mapping_size) y el número mínimo 
 * de veces que ha de aparecer cada entrada de la tabla (count_cut_off). 
 * Se recorren los objetos insertados siempre i cuando el número de
 * elementos contenidos por este objeto sea mayor a max_mapping_size.
 * Se eliminan del mapping todas aquellas entradas que hayan aparecido 
 * count_cut_off veces o menos. 
 * Tras recorrer todas las entradas del mapping si el número de 
 * entradas del hashing sigue siendo mayor al tamaño máximo permitido,
 * max_mapping_size, se llama a  
 *
 *		mappingRepair(max_mapping_size,count_cut_off+1)
 *
 * Devuelve el número de entradas borradas. 
 */
int mapping::mappingRepair(int max_mapping_size, int count_cut_off)
{
  int ret=0,numdel=0;
  mapping_node_t *aux;

  hash_t *tptr = mapByKey;

  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size, h, i;

  if ( verbose_svmtool == TRUE)
    fprintf(stderr,"\n\tReducing Mapping (%d) --> size (%d)",count_cut_off,this->mappingNumElements());

  old_bucket=tptr->bucket;
  old_size=tptr->size;
  if (max_mapping_size<this->mappingNumElements())
  {
  for (i=0; i<old_size; i++)
    {
      old_hash=old_bucket[i];
      while(old_hash)
	{
	  tmp=old_hash;
	  old_hash=old_hash->next;
	  aux = (mapping_node_t *) tmp->data;
	  
	  if (aux->num <  count_cut_off)
	    {
	     hash_delete(mapByKey,aux->feature);
	     
	     hash_delete(mapByNumber,aux->number);
	     delete aux;
	     aux = NULL;
	     mapping_counter--; numdel++;
	  }
	} /* while */
    } /* for */
  }/* if */

  if ( verbose_svmtool == TRUE)
    fprintf(stderr," - deleted (%d) = Final size (%d)",numdel,this->mappingNumElements());

  int number=0;
  if (max_mapping_size<this->mappingNumElements()) number = mappingRepair(max_mapping_size,count_cut_off+1);
  return (number+numdel); 
}

/****************************************************************************/
/*
 * Devuelve el número de elementos insertados en el objeto.
 */
int mapping::mappingNumElements()
{
return mapping_counter-1;
}

/****************************************************************************/
/*
 * Este método recibe el apuntador al fichero (f) desde el cual 
 * se crea el mapping, el número mínimo de veces que puede aparecer 
 * una palabra en el mapping (count_cut_off) i el tamaño máximo del 
 * mapping (max_mapping_size).
 * Se recorre el fichero añadiendo todos los atributos  encontrados.
 * Después se repara el mapping llamando a mappingRepair. 
 */
void mapping::mappingBuilt(FILE *f,int max_mapping_size, int count_cut_off)
{
  char str[200]="";
  int contador=0;

  fseek(f,0,SEEK_SET);
  while (!feof(f))
    {
      int ret1 = readTo(f,' ',0,str);
      while (ret1>0)
	{
	  ret1 = readTo(f,' ','\n',str);
	  this->mappingAddByKey(str);
	  contador++;
	}
        if ( verbose_svmtool == TRUE ) showProcessDone(contador,300,FALSE,"features");
    }
  if ( verbose_svmtool == TRUE ) showProcessDone(contador,300,TRUE,"features");

  if (this->mappingNumElements()>max_mapping_size)
  {
    if ( verbose_svmtool == TRUE ) fprintf(stderr,"REDUCING MAPPING: ");
    int numdel = this->mappingRepair(max_mapping_size,count_cut_off);
  }
}

/****************************************************************************/

/* 
 * Guarda los datos del mapping en el fichero con nombre, fName.
 * Si el parámetro onlyFeatures vale TRUE escribe sólo los atributos. 
 * En otro caso, escribe el identificador numérico, el atributo i 
 * el número de veces que aparece.
 */ 
void  mapping::mappingWrite(char *fName,int onlyFeatures)
{
    FILE *f = openFile(fName,"w");
  int ret=0,numdel=0;
  mapping_node_t *aux;

  hash_t *tptr = mapByKey;
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
	aux = (mapping_node_t *) tmp->data;
	
	if (onlyFeatures == TRUE)  fprintf(f,"%s\n",aux->feature);
	else fprintf(f,"%s %s %d\n",aux->number,aux->feature, aux->num);
	
      	} /* while */
    } /* for */
  
  fclose (f);
}



