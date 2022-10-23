#ifndef __AANIREUNA_H__
#define __AANIREUNA_H__
#define TAAJ_Hz 48000
#define TAAJ_kHz 48
/*negatiiviset ovat viestejä äänireuna-ohjelmalta
  positiiviset ovat viestejä äänireuna-ohjelmalle*/
enum {
  seuraavaksi_lopun_unixaika = -0xff,
  seuraavaksi_alun_unixaika,
  valinnan_erotin,
  havaittiin_reuna,

  äänireuna_tallenna_takaa=1,
  äänireuna_tallenna_tästä,
  äänireuna_valitse_molemmat,
};
#endif
