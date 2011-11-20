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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "weight.h"

float absolut(float f)
{
	if (f < 0) return (-1)*f;
	else return f;
}


/***********************************************************/

/*
 * El objeto WeightRepository es el encargado de contener los pesos
 * para cada pareja POS-feature. Un depsito de pesos est formado por
 * un hash de objetos weight_struct_t, conteniendo los atributos (key).
 * Cada uno de estos hash es poseedor de un nuevo hash con todas las POS
 * para las cuales se ha encontrado el atributo y su respectivo peso.
 * (weight_node_t).
 */

/***********************************************************/

//Definicin de ewight_struct_t
class weight_struct_t
{
	public:
		char key[150];
		hash_t *hash;

		//El destructot de este objeto eliminar el contenido del hash
		~weight_struct_t()
		{
			weight_node_t *aux;

			hash_t *tptr = this->hash;
			hash_node_t **old_bucket, *old_hash, *tmp;
			int old_size;

			old_bucket=tptr->bucket;
			old_size=tptr->size;

			//Recorremos todas las entradas de la tabla de hash
			//Eliminando todos no objetos que encontremos
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
				}				 /* while */
			}					 /* for */

			hash_destroy(hash);
		}
};

/***********************************************************/

char weightRepository::wrSaltarBlancs(FILE *in, char c,int jmp)
{
	while ((c==':') || (c==' ') || (c=='\n' && jmp==1)) c=fgetc(in);
	return c;
}


/***********************************************************/

/*
 * void weightRepository::wrReadMergeModel(FILE *in,float filter)
 * Parmetros:
 * 	FILE *in : apuntador al fichero que ha de leer
 *	float filter: Valor para filtrar los pesos que se lean
 * Este mtodo carga un depsito de pesos de un fichero (f), filltrando
 * los pesos que esten por debajo del lmite marcado (filter)
 */
void weightRepository::wrReadMergeModel(FILE *in,float filter)
{
	char c=fgetc(in),key[200],value[100],*endptr;
	weight_struct_t *obj;
	char garbage[512];

	strcpy(key,"");
	strcpy(value,"");
	while  (!feof(in))
	{
		//c = fgetc(in);
		if (c!='#')
		{
			obj = new weight_struct_t;
			strcpy(obj->key,"");

			while (c!=' ')
			{
				sprintf(obj->key,"%s%c",obj->key,c);
				c=fgetc(in);
			}

			obj->hash  = new hash_t;
			hash_init(obj->hash,10);

			while ((c!='\n') && (!feof(in)))
			{
				weight_node_t *w = new weight_node_t;

				c = wrSaltarBlancs(in,c,0);
				strcpy(w->pos,""); strcpy(value,"");
				while ((c!=':') && (!feof(in)))
				{
					sprintf(w->pos,"%s%c",w->pos,c);
					c=fgetc(in);
				}

				c = wrSaltarBlancs(in,c,0);

				while ((c!=' ') && (c!='\n') && (!feof(in)) )
				{
					sprintf(value,"%s%c",value,c);
					c=fgetc(in);
				}

				w->data=atof(value);
				if ( absolut(w->data) > absolut(filter) )
					hash_insert(obj->hash,w->pos,(uintptr_t) w);
				else delete w;
			}

			c = wrSaltarBlancs(in,c,1);

			hash_insert(&wr,obj->key, (uintptr_t) obj);
		}
		else
		{						 //while(c=fgetc(in)!='\n');
			fgets(garbage,512,in);
			c = fgetc(in);
		}
	}
}


/***********************************************************/

/*
 * long double weightRepository::wrGetWeight(const char *feature,char *pos)
 * Parmetros:
 *	char *feature: Atributo
 *	char *pos: Etiqueta morfosintctica
 * Lee el peso para el atributo y la etiqueta recibidos como parmetro.
 */
long double weightRepository::wrGetWeight(const char *feature,char *pos)
{
	uintptr_t h = hash_lookup(&wr,feature);
	if (h!=HASH_FAIL)
	{
		weight_struct_t *obj = (weight_struct_t *)h;
		uintptr_t w = hash_lookup(obj->hash,pos);

		if (w!=HASH_FAIL)
		{
			weight_node_t *ret = (weight_node_t *)w;
			return ret->data;
		}
	}
	return 0;
}


/***********************************************************/

/*
 * weightRepository(char *fileName,float filter)
 * Parmetros:
 * 	char *fileName : Nombre del fichero
 *	float filter: Valor para filtrar los pesos que se lean
 * Constructor que carga el deposito de pesos del fichero llamado
 * fileName , filltrando los pesos que esten por debajo del lmite
 * marcado (filter)
 */
weightRepository::weightRepository(char *fileName,float filter)
{
	FILE *in;
	if ((in = fopen(fileName, "rt"))== NULL)
	{
		fprintf(stderr, "Error opening weightRepository: %s. It's going to work without it.\n",fileName);
		exit(0);
	}
	hash_init(&wr,10000);
	wrReadMergeModel(in,filter);
	fclose(in);
}


/***********************************************************/

/*
 * weightRepository()
 * Contructor
 */
weightRepository::weightRepository()
{
	hash_init(&wr,10000);
}


/***********************************************************/

/*
 * ~weightRepository()
 * Destructor
 */
weightRepository::~weightRepository()
{

	weight_struct_t *aux;

	hash_t *tptr = &wr;
	hash_node_t **old_bucket, *old_hash, *tmp;
	int old_size;

	old_bucket=tptr->bucket;
	old_size=tptr->size;
	//Recorre las listas de sinnimos de la tabla de hash
	//eliminando los datos
	for (int i=0; i<old_size; i++)
	{
		old_hash=old_bucket[i];
		while(old_hash)
		{
			tmp=old_hash;
			old_hash=old_hash->next;

			aux = (weight_struct_t *) tmp->data;

			delete aux;
			aux = NULL;
		}						 /* while */
	}							 /* for */

	hash_destroy(&wr);
}


/*******************************************************/

/*
 * void wrAddPOS(uintptr_t obj, char* pos, long double weight)
 * Parmetros:
 * 	int obj: Apuntador al objeto que contiene el atributo
 *	char *pos: Etiqueta a insertar:
 *	long double weight: Peso a asignar a la etiqueta
 * Insertamos un nuevo peso para la etiqueta pos, en el atributo indicado
 * por obj. Si la etiqueta ya existe se incrementa el peso con weight. Si
 * no existe se aade.
 */
void weightRepository::wrAddPOS(uintptr_t obj, char* pos, long double weight)
{
	weight_struct_t *wst = (weight_struct_t *)obj;
	uintptr_t x = hash_lookup( wst->hash, pos);

	if (x==HASH_FAIL)
	{
		//Insertamos Nueva POS
		weight_node_t *w = new weight_node_t;
		strcpy(w->pos,pos);
		w->data=weight;
		hash_insert( wst->hash,w->pos,(uintptr_t) w);
	}
	else
	{							 //Si POS ya esta, incrementamos el peso
		weight_node_t *wnt = (weight_node_t *)x;
		wnt->data = wnt->data + weight;
	}
}


/*******************************************************/

/*
 * void wrAdd(char *feature, char* pos, long double weight)
 * Parmetros:
 * 	char *feature: Atributo a insertar
 *	char *pos: Etiqueta a insertar
 *	long double weight: Peso a asignar a la etiqueta
 * Insertamos un nuevo peso para para el atributo feature y la etiqueta pos.
 */
void weightRepository::wrAdd(char *feature, char* pos, long double weight)
{
	weight_struct_t *obj = (weight_struct_t *)hash_lookup(&wr,feature);

	if ( (uintptr_t) obj == HASH_FAIL)
	{
		// Creamos nueva entrada en WeightRepository
		obj = new weight_struct_t;
		strcpy(obj->key,feature);
		obj->hash  = new hash_t;
		hash_init(obj->hash,10);
		//Aadimos el peso y la etiqueta
		wrAddPOS((uintptr_t)obj,pos,weight);
		hash_insert(&wr,obj->key, (uintptr_t) obj);
	}
	else
		//Aadimos el peso y la etiqueta
		wrAddPOS((uintptr_t)obj,pos,weight);
}


/*******************************************************/

/*
 * wrWrite(const char *outName)
 * Escribe el depsito de pesos  en el fichero con nombre outName.
 *
 * Modificacin 180705:
 *     Aadimos el parmetro "float filter", se utiliza para filtrar pesos
 */
void weightRepository::wrWrite(const char *outName, float filter)
{
	weight_struct_t *wst;
	FILE *f;

	if ((f = fopen(outName, "w"))== NULL)
	{
		fprintf(stderr, "Error opening file: %s\n",outName);
		exit(0);
	}

	hash_t *tptr = &wr;

	hash_node_t *node, *last;
	int i;

	//Recorremos el hash objeto a objeto
	for (i=0; i<tptr->size; i++)
	{
		node = tptr->bucket[i];
		while (node != NULL)
		{
			last = node;
			node = node->next;
			wst = (weight_struct_t *) last->data;

			//Modificacin 180705: aadimos filtrado de pesos
			//char *mrg = wrGetMergeInput(wst->hash); //DEL 180705
								 //ADD 180705
			char *mrg = wrGetMergeInput(wst->hash,filter);

			if (strcmp(mrg,"")!=0) fprintf(f,"%s%s\n",wst->key,mrg);
			delete mrg;
		}						 //while
	}							 //for
	fclose (f);
}


/*******************************************************/
/* 
 * char *wrGetMergeInput(hash_t *tptr)
 * Devuelve una cadena de caracteres con todas las parejas
 * POS/PESO contenidas en el hash (tptr) de un atributo.
 *
 * Modificacin 180705:
 *        Aadimos parmetro "float filter" para filtrado de pesos
 */
char *weightRepository::wrGetMergeInput(hash_t *tptr, float filter)
{
	char *out = new char[3000];
	weight_node_t *wnt;
	hash_node_t **old_bucket, *old_hash, *tmp;
	int old_size, h, i;

	old_bucket=tptr->bucket;
	old_size=tptr->size;
	strcpy (out,"");

	for (i=0; i<old_size; i++)
	{
		old_hash=old_bucket[i];
		while(old_hash)
		{
			tmp=old_hash;
			old_hash=old_hash->next;
			wnt = (weight_node_t *) tmp->data;

			if ((float)wnt->data!=0)
			{
				//Modificacin 180705: Filtrado de pesos
				//Comprobamos que el peso a insertar en el fichero
				//cumple con el filtrado de pesos.
								 //ADD 180705
				if ( absolut(wnt->data) > absolut(filter) )
					sprintf(out,"%s %s:%.18E",out,wnt->pos, (float) wnt->data);
			}
		}						 //while
	}							 //for
	return out;
}


/*******************************************************/

/*
 * void wrWriteHash(hash_t *tptr,FILE *f, char separador)
 * Escribe el contenido de un hash (tptr), en fichero apuntado por f.
 * Entre cada pareja POS/PESO pone el caracter separador.
 */
void weightRepository::wrWriteHash(hash_t *tptr,FILE *f, char separador)
{
	weight_node_t *wnt;
	hash_node_t **old_bucket, *old_hash, *tmp;
	int old_size, h, i;
	int cont=0;

	old_bucket=tptr->bucket;
	old_size=tptr->size;

	for (i=0; i<old_size; i++)
	{
		old_hash=old_bucket[i];
		while(old_hash)
		{
			tmp=old_hash;
			old_hash=old_hash->next;
			wnt = (weight_node_t *) tmp->data;
			if (separador == '\n' && cont==0) fprintf(f,"%s %2.10f",wnt->pos,(float)wnt->data);
			else  fprintf(f,"%c%s:%2.10f",separador,wnt->pos,(float)wnt->data);
			cont++;
		}						 /* while */
	}							 /* for */
}
