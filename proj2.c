// proj2.c
// Řešení IOS-DU2, 28.4.2019
// Autor: Ludek Burda, FIT
// Přeloženo: gcc 7.4.0

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include "fun.h" // knihovna s obecne vyuzitelnymi funkcemi pro zjednoduseni a prehlednost kodu
#include "errors.h" // knihovna pro reseni chybovych stavu

#define SERF "SERF"
#define HACK "HACK"

sem_t *boardQueue, *mutexAboard, *mutexActions, *mutexRow, *SerfQueue, *HackQueue, *TheEnd; // deklarace semaforu
int *hackers, *serfs, *actions, *wholeSquad, *hackOnDockside, *serfOnDockside; // deklarace sdilenych promennych
pid_t wpid; // pomocna promenna pro cekani na child procesy
int status = 0; //
FILE *soubor; // soubor pro zapis vystupu

int main(int argc, char *argv[]) {
  ArgHandle(argc,argv); // osetreni argumentu funkci
  int Persons = atoi(argv[1]); // argument P
  int HackMax = atoi(argv[2]); // arg H
  int SerfMax = atoi(argv[3]); // arg S
  int RowMax = atoi(argv[4]); // arg R
  int WakeMax = atoi(argv[5]); // arg W
  int Capacity = atoi(argv[6]); // arg C
  int whichSerf = 0; // unikatni identifikatory jednotlivych procesu
  int whichHack = 0; //
  setbuf(stdout,NULL);
  setbuf(stderr,NULL);
  hackers = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // alokace sdilene pameti pro pocitadla
  serfs = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  actions = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  wholeSquad = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  hackOnDockside = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  serfOnDockside = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  *hackers = 0, *serfs = 0, *actions = 0, *wholeSquad = 0, *hackOnDockside = 0, *serfOnDockside = 0; // inicializace pocitadel sdilene pameti na 0
  boardQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // alokace sdilene pameti pro semafory
  mutexAboard = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  mutexActions = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  mutexRow = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  SerfQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  HackQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  TheEnd = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //
  sem_init(mutexAboard,1,1); // inicializace semaforu
  sem_init(mutexActions,1,1); //
  sem_init(mutexRow,1,1); //
  sem_init(boardQueue,1,0); //
  sem_init(SerfQueue,1,0); //
  sem_init(HackQueue,1,0); //
  sem_init(TheEnd,1,0); //


  soubor = fopen("proj2.out","w"); // zalozeni noveho souboru
  if (soubor == NULL) MyError(CLO); // kontrola

  if (fork() == 0){ // serfs
    for (int i = 0; i < Persons; i++){
      usleep(1000*(rand()%(SerfMax + 1))); // uspat na nahodny pocet milisekund
      if (fork() == 0){             // serf create
        sem_wait(mutexAboard);
        sem_wait(mutexActions);
        *actions = *actions + 1; // pocitadlo akci
        *serfs = *serfs + 1; // pocitadlo celkoveho poctu serfu
        whichSerf = *serfs;
        fprintf(soubor,"%d\t: %s %d\t: starts\n",*actions,SERF,whichSerf);
        fflush(soubor);
        sem_post(mutexActions);
        while (1){                // pristup na molo
          sem_wait(mutexActions);
          *actions = *actions + 1;

          if (*wholeSquad < Capacity){ // podminka pristupu na molo
            *serfOnDockside = *serfOnDockside + 1;
            *wholeSquad = *wholeSquad + 1;
            fprintf(soubor,"%d\t: %s %d\t: waits\t \t \t: %d\t: %d\n",*actions,SERF,whichSerf,*hackOnDockside,*serfOnDockside);
            fflush(soubor);
            sem_post(mutexActions);
            if ((*serfOnDockside == 4) || (*serfOnDockside > 1 && *hackOnDockside > 1)) { // nastoupi na lod jako kapitan
              sem_wait(mutexRow);
              if (*serfOnDockside == 4) {

                for (int i = 0; i < 3; i++) {
                  sem_post(SerfQueue);
                }
                *serfOnDockside = *serfOnDockside - 4;
              }
              else {
                for (int i = 0; i < 2; i++){
                  sem_post(HackQueue);
                }
                sem_post(SerfQueue);
                *serfOnDockside = *serfOnDockside - 2;
                *hackOnDockside = *hackOnDockside - 2;
              }
              *wholeSquad = *wholeSquad - 4;
              *actions = *actions + 1;
              fprintf(soubor,"%d\t: %s %d\t: boards\t\t: %d\t: %d\n",*actions,SERF,whichSerf,*hackOnDockside,*serfOnDockside);
              fflush(soubor);

              usleep(1000*(rand()%(RowMax + 1))); // uspani vytvoreno podle vzorce, konstanta tisic protoze usleep je v mikrosekundach, (1000*(rand()%(RangeMax-RangeLow+1)+RangeLow)
              sem_wait(mutexActions);

              for (int i = 0; i < 3; i++) {
                sem_post(TheEnd);
              }
              *actions = *actions + 1;
              fprintf(soubor,"%d\t: %s %d\t: captain exits\t\t: %d\t: %d\n",*actions,SERF,whichSerf,*hackOnDockside,*serfOnDockside);
              fflush(soubor);
              sem_post(mutexAboard);
              sem_post(mutexActions);
              sem_post(mutexRow);
              exit(EXIT_SUCCESS);
            }
            else { // nenalodi se jako kapitan
              sem_post(mutexAboard);
              sem_wait(SerfQueue);
              sem_wait(TheEnd);
              sem_wait(mutexActions);
              *actions = *actions + 1;
              fprintf(soubor,"%d\t: %s %d\t: member exits\t\t: %d\t: %d\n",*actions,SERF,whichSerf,*hackOnDockside,*serfOnDockside);
              fflush(soubor);
              sem_post(mutexActions);
              exit(EXIT_SUCCESS);
            }
          }

          fprintf(soubor,"%d\t: %s %d\t: leaves queue\t\t: %d\t: %d\n",*actions,SERF,whichSerf,*hackOnDockside,*serfOnDockside);
          fflush(soubor);
          sem_post(mutexActions);
          sem_post(mutexAboard);
          usleep(1000*((rand()%(WakeMax - 20 + 1)) + 20)); // uspani - 20 je ze zadani nejmensi pocet prospanych milisekund

          sem_wait(mutexActions); // probuzeni
          *actions = *actions + 1;
          fprintf(soubor,"%d\t: %s %d\t: is back\n",*actions,SERF,whichSerf);
          fflush(soubor);
          sem_post(mutexActions);
          sem_wait(mutexAboard);
        }
      }
    }
    while ((wpid = wait(&status)) > 0);
    exit(EXIT_SUCCESS);
  }

  else { // hackers
    if (fork()==0){
      for (int i = 0; i < Persons; i++){
        usleep(1000*(rand()%(HackMax + 1)));
        if (fork()==0){
          sem_wait(mutexAboard);
          sem_wait(mutexActions); // hack create
          *hackers = *hackers + 1;
          whichHack = *hackers;
          *actions = *actions + 1;
          fprintf(soubor,"%d\t: %s %d\t: starts\n",*actions,HACK,whichHack);
          fflush(soubor);
          sem_post(mutexActions);
          while (1){ // pristup na molo

            sem_wait(mutexActions);
            if (*wholeSquad < Capacity){ // podminka pristupu na molo
              *hackOnDockside = *hackOnDockside + 1;
              *wholeSquad = *wholeSquad + 1;
              *actions = *actions + 1;
              fprintf(soubor,"%d\t: %s %d\t: waits\t\t\t: %d\t: %d\n",*actions,HACK,whichHack,*hackOnDockside,*serfOnDockside);
              fflush(soubor);
              sem_post(mutexActions);
              if ((*hackOnDockside == 4) || (*hackOnDockside > 1 && *serfOnDockside > 1)) { // nastoupi na lod jako kapitan

                sem_wait(mutexRow);
                if (*hackOnDockside == 4) {
                  for (int i = 0; i < 3; i++) sem_post(HackQueue);
                  *hackOnDockside = *hackOnDockside - 4;
                }
                else {
                  for (int i = 0; i < 2; i++) sem_post(SerfQueue);
                  sem_post(HackQueue);
                  *serfOnDockside = *serfOnDockside - 2;
                  *hackOnDockside = *hackOnDockside - 2;
                }
                *wholeSquad = *wholeSquad - 4;
                *actions = *actions + 1;
                fprintf(soubor,"%d\t: %s %d\t: boards\t\t: %d\t: %d\n",*actions,HACK,whichHack,*hackOnDockside,*serfOnDockside);
                fflush(soubor);

                usleep(1000*(rand()%(RowMax + 1))); // uspani - 20 je ze zadani nejmensi pocet prospanych milisekund
                sem_wait(mutexActions);

                for (int i = 0; i < 3; i++){
                  sem_post(TheEnd);
                }
                *actions = *actions + 1;
                fprintf(soubor,"%d\t: %s %d\t: captain exits\t\t: %d\t: %d\n",*actions,HACK,whichHack,*hackOnDockside,*serfOnDockside);
                fflush(soubor);
                sem_post(mutexAboard);
                sem_post(mutexActions);
                sem_post(mutexRow);
                exit(EXIT_SUCCESS);
              }
              else { // nenalodi se jako kapitan
                sem_post(mutexAboard);
                sem_wait(HackQueue);
                sem_wait(TheEnd);
                sem_wait(mutexActions);
                *actions = *actions + 1;
                fprintf(soubor,"%d\t: %s %d\t: member exits\t\t: %d\t: %d\n",*actions,HACK,whichHack,*hackOnDockside,*serfOnDockside);
                fflush(soubor);
                sem_post(mutexActions);
                exit(EXIT_SUCCESS);
              }
            }

            fprintf(soubor,"%d\t: %s %d\t: leaves queue\t\t: %d\t: %d\n",*actions,HACK,whichHack,*hackOnDockside,*serfOnDockside);
            fflush(soubor);
            sem_post(mutexActions);
            sem_post(mutexAboard);
            usleep(1000*((rand()%(WakeMax - 20 + 1)) + 20)); // uspani - 20 je ze zadani nejmensi pocet prospanych milisekund

            sem_wait(mutexActions); // probuzeni
            *actions = *actions + 1;
            fprintf(soubor,"%d\t: %s %d\t: is back\n",*actions,HACK,whichHack);
            fflush(soubor);
            sem_post(mutexActions);
            sem_wait(mutexAboard);
          }

        }
      }
      while ((wpid = wait(&status)) > 0);
      exit(EXIT_SUCCESS);
    }
    while ((wpid = wait(&status)) > 0); // pocka na vsechny child procesy
  }
  munmap(hackers, sizeof(int)); // vycisteni pameti od zdroju
  munmap(serfs, sizeof(int));
  munmap(actions, sizeof(int));
  munmap(wholeSquad, sizeof(int));
  munmap(hackOnDockside, sizeof(int));
  munmap(serfOnDockside, sizeof(int));
  sem_destroy(mutexAboard);
  munmap(mutexAboard, sizeof(sem_t));
  sem_destroy(mutexActions);
  munmap(mutexActions, sizeof(sem_t));
  sem_destroy(mutexRow);
  munmap(mutexRow, sizeof(sem_t));
  sem_destroy(boardQueue);
  munmap(boardQueue, sizeof(sem_t));
  sem_destroy(SerfQueue);
  munmap(SerfQueue, sizeof(sem_t));
  sem_destroy(HackQueue);
  munmap(HackQueue, sizeof(sem_t));
  sem_destroy(TheEnd);
  munmap(TheEnd, sizeof(sem_t));
  if(fclose(soubor) == EOF) MyError(CLO);

  return 0;
}
