#ifndef __MUISTIN_JAKO__
#define __MUISTIN_JAKO__
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KOKO 2048

enum viesti_e {
  ipcAnna_sekoitus = 1,
  ipcTarkastelu,
  ipcAloita,
  ipcLopeta,
  ipcJatka
};

typedef struct {
  enum viesti_e viesti;
  char data[SHM_KOKO];
} shmRak_s;

extern shmRak_s* ipc;
extern int viimeViesti;

static const int projId = 55;
static const char* shmTied = "/tmp";

shmRak_s* ipc;
int viimeViesti = 0;

shmRak_s* liity_muistiin() {
  static char luotu = 0;
  int avain = ftok(shmTied, projId);
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
  r->viesti = 0;
  if(r < 0) {
    perror("Virhe, shmat");
    return NULL;
  }
  return r;
}

#endif
