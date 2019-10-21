// errors.h
// Hlavickovy soubor pro osetreni chybovych stavu, 28.4.2019
// Autor: Ludek Burda, FIT
// Přeloženo: gcc 7.4.0

#include <stdio.h>
#include <stdlib.h>

#ifndef ERR
#define ERR

#define ARG 1
#define CLO 2

void MyError(int code){
  assert(code>0 && code<2);
  switch(code){
    case 1:
      fprintf(stderr,"Nespravne zadany argument.\n");
      exit(1);
    case 2:
      fprintf(stderr,"Nepodarilo se otevrit/zavrit soubor.\n");
      exit(1);
  }
}

#endif
