#ifndef __MUISTIN_JAKO__
#define __MUISTIN_JAKO__
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KOKO_DATA 2044

enum viesti_e {
  ipcAnna_sekoitus = 1,
  ipcTarkastelu,
  ipcAloita,
  ipcLopeta,
  ipcJatka
};

typedef struct {
  enum viesti_e viesti;
  char data[SHM_KOKO_DATA];
} shm_tietue;

shm_tietue* ipc;
int viimeViesti = 0;

shm_tietue* liity_muistiin() {
  static char luotu = 0;
  int avain = ftok("/tmp", 55);
  if(avain < 0) {
    perror("\033[31mVirhe (liity_muistiin()->ftok)\033[0m");
    return NULL;
  }
  int shmid = shmget( avain, sizeof(shm_tietue), (IPC_CREAT | 0666)*(!!luotu) );
  luotu = 1;
  if(shmid < 0) {
    perror("\033[31mVirhe (liity_muistiin()->shmget)\033[0m");
    return NULL;
  }
  shm_tietue *r = shmat(shmid, NULL, 0);
  r->viesti = 0;
  if(r < 0) {
    perror("\033[31mVirhe (liity_muistiin()->shmat)\033[0m");
    return NULL;
  }
  return r;
}
#endif
