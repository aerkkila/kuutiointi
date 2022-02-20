#ifndef __AANIREUNA_H__
#define __AANIREUNA_H__
/*negatiiviset ovat viestejä äänireuna-ohjelmalta
  positiiviset ovat viestejä äänireuna-ohjelmalle*/
enum {
  seuraavaksi_kohdan_unixaika = -0xff,
  valinnan_erotin,
  havaittiin_reuna,

  aanireuna_valinta=1,
  aanireuna_tallenna,
  aanireuna_valitse_molemmat,
};
#endif
