#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "muistin_jako.h"

shmRak_s* liity_muistiin() {
  static char luotu = 0;
  const char shmTied[] = "shmTied.txt";
  int avain = ftok(shmTied, PROJ_ID);
  if(avain < 0) {
    perror("Virhe, ftok");
    return NULL;
  }
  int shmid = shmget(avain, SHM_KOKO, (!luotu)? (IPC_CREAT | 0666) : 0);
  luotu = 1;
  if(shmid < 0) {
    perror("Virhe, shmget");
    return NULL;
  }
  shmRak_s *r = shmat(shmid, NULL, 0);
  if(r < 0) {
    perror("Virhe, shmat");
    return NULL;
  }
  return r;
}
