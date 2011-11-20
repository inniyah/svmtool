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

#ifndef SWINDOW_H

#include "common.h"

#define CHAR_NULL	'~'
#define EMPTY_WORD	"_"
#define EMPTY_POS	"??"
#define EMPTY		""
#define LEFT_TO_RIGHT		1
#define RIGHT_TO_LEFT		2
#define PUT_MAX	0
#define RESET_VALUES 1
#define PUT_OLD 2

#define COUNTING_FROM_END   111
#define COUNTING_FROM_BEGIN 222

struct nodo{
  int ord;
  char wrd[TAM_WORD];
  char realWrd[TAM_WORD];
  char comment[TAM_LINE];
  char pos[TAM_POS],posOld[TAM_POS];
  long double weight,weightOld;
  struct stack_t *stackScores;
  nodo *next;
  nodo *previous;
};

class swindow
{
 private:
  FILE *input;
  //List Control
  nodo *first;
  nodo *last;
  int numObj;

  //Window Control
  nodo *index,*beginWin,*endWin;
  int  lengthWin,posIndex,posBegin,posEnd;

  void init();
  int iniList();

  int readSentence();
  int readInput();
  int readInput_old();
  int winAdd(char *wrd, char *pos);

  int winLookRegExp2(void *er,char *str);
  void winCompRegExp();
  void winFreeRegExp();

 public:
  int winLookRegExp(char *m);
  int winMaterializePOSValues(int action);

  ~swindow();
  swindow(FILE *input);
  swindow(FILE *input,int number, int position);
  swindow(FILE *input,int number);
  int next();
  int previous();
  nodo *getIndex();
  nodo *get(int position,int direction);
  int show();

  void putLengthWin(int l);
  void putIndex(int i);

  int sentenceLength();

  int winExistUnkWord(int direction, dictionary *d);

  void winPushWordFeature(void *ptr,dictionary *d, struct stack_t *pila,int direction);
  void winPushPosFeature(void *ptr,dictionary *d, struct stack_t *pila,int direction);
  void winPushAmbiguityFeature(void *ptr,dictionary *d, struct stack_t *pila,int direction);
  void winPushMFTFeature(void *ptr,dictionary *d, struct stack_t *pila,int direction);
  void winPushMaybeFeature(void *ptr,dictionary *d, struct stack_t *pila,int direction);
  void winPushSwnFeature(struct stack_t *pila);
  void winPushUnknownFeatures(char *str, struct stack_t *pila);

  void winPushSuffixFeature(char *wrd, struct stack_t *pila,int longitud);
  void winPushPrefixFeature(char *wrd, struct stack_t *pila,int longitud);
  //void winPushStartCapFeature(char *wrd, struct stack_t *pila);
  void winPushAllUpFeature(char *wrd,stack_t *pila);
  void winPushAllLowFeature(char *wrd,stack_t *pila);
  void winPushContainCapFeature(char *wrd, stack_t *pila);
  void winPushContainCapsFeature(char *wrd, stack_t *pila);
  void winPushContainPeriodFeature(char *wrd, stack_t *pila);
  void winPushContainCommaFeature(char *wrd, stack_t *pila);
  void winPushContainNumFeature(char *wrd, stack_t *pila);
  void winPushMultiwordFeature(char *wrd, stack_t *pila);
  void winPushLetterFeature(char *, stack_t *, int, int );
  void winPushLenghtFeature(char *wrd, stack_t *pila);
  void winPushStartWithCapFeature(char *,stack_t *);
  void winPushStartWithLowerFeature(char *,stack_t *);
  void winPushStartWithNumberFeature(char *,stack_t *);
  int iniGeneric();
  void deleteList();
};


#define SWINDOW_W
#endif
