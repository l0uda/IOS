// fun.h
// Hlavickovy soubor s uzitecnymi funkcemi, 28.4.2019
// Autor: Ludek Burda, FIT
// Přeloženo: gcc 7.4.0

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "errors.h"

#ifndef MYFNCS
#define MYFNCS

void *MyMalloc(int size){
  void *p = malloc(size);
  if (p == NULL) {
    fprintf(stderr,"Alokace se nezdarila.\n");
    exit(1);
  }
  return p;
}

int IsNumber(char *pole){
  unsigned int i;
  if (pole[0] != '+' && pole[0] != '-' && !isdigit(pole[0])) return 0;
  for (i = 1; i < strlen(pole); i++){
    if (!isdigit(pole[i])) return 0;
  }
  return 1;
}

void ArgHandle(int argc, char *argv[]){ // only for IOS
  int i;
  if (argc != 7) MyError(ARG);
  for (i = 1; i < argc; i++){
    if (!IsNumber(argv[i])) MyError(ARG);
  }
  if (atoi(argv[1]) < 2 || atoi(argv[1])%2 != 0) MyError(ARG);
  for (i = 2; i < 5; i++) if (atoi(argv[i]) < 0 || atoi(argv[i]) > 2000) MyError(ARG);
  if (atoi(argv[5]) < 20 || atoi(argv[5]) > 2000) MyError(ARG);
  if (atoi(argv[6]) < 5) MyError(ARG);
}

#endif
