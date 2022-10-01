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

  aanireuna_valinta=1,
  äänireuna_tallenna_takaa,
  äänireuna_tallenna_tästä,
  aanireuna_valitse_molemmat,
};
#endif
