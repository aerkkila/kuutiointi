#ifndef __KUUTION_KOMMUNIKOINTI__
#define __KUUTION_KOMMUNIKOINTI__

#define PROJ_ID 55
#define SHM_KOKO 2048

extern const char jaetTied[];

enum viesti_e {
  valmis,
  anna_sekoitus,
  tarkastelee,
  ratkaisee
};
  

typedef struct {
  enum viesti_e viesti;
  char data[SHM_KOKO];
} shmRak_s;

shmRak_s* liity_muistiin();
void lue_siirrot(shmRak_s*);
#endif
