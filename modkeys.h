#ifndef __MODKEYS__
#define __MODKEYS__
#define LVAIHTO (1<<0)
#define RVAIHTO (1<<1)
#define LCTRL   (1<<2)
#define RCTRL   (1<<3)
#define LALT    (1<<4)
#define RALT    (1<<5)
#define LWIN    (1<<6)
#define RWIN    (1<<7)
#define VAIHTO (LVAIHTO | RVAIHTO)
#define CTRL (LCTRL | RCTRL)
#define ALT (LALT | RALT)
#define WIN (LWIN | RWIN)
#endif

#ifdef _MODKEYS_SWITCH_KEYDOWN
#define _MODNAPPAIN(nap) modkey |= nap; break
#define _MODKEYS_SWITCH
#undef _MODKEYS_SWITCH_KEYDOWN
#endif
#ifdef _MODKEYS_SWITCH_KEYUP
#define _MODNAPPAIN(nap) modkey &= ~nap; break
#define _MODKEYS_SWITCH
#undef _MODKEYS_SWITCH_KEYUP
#endif
#ifdef _MODKEYS_SWITCH
#if 0
switch(0) { //auttaa automaattisisennystä
#endif
 case SDLK_LSHIFT:
   _MODNAPPAIN(LVAIHTO);
 case SDLK_RSHIFT:
   _MODNAPPAIN(RVAIHTO);
 case SDLK_LALT:
   _MODNAPPAIN(LALT);
 case SDLK_RALT:
   _MODNAPPAIN(RALT);
 case SDLK_LCTRL:
   _MODNAPPAIN(LCTRL);
 case SDLK_RCTRL:
   _MODNAPPAIN(RCTRL);
 case SDLK_LGUI:
   _MODNAPPAIN(LWIN);
 case SDLK_RGUI:
   _MODNAPPAIN(RWIN);
#if 0
 }
#endif
#undef _MODNAPPAIN
#undef _MODKEYS_SWITCH
#endif
