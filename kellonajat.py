#!/usr/bin/python

#Tämä kutsuu c-ohjelmaa tiedostossa kellonajat.c
#Piirtää kuvaajan montako tulosta on kullakin vuorokauden minuutilla
#Gaussin alipäästösuodattimen herkkyyttä voi säätää liu'ulla

import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, CheckButtons
import numpy as np
import sys, time
from ctypes import *

if len(sys.argv) < 2:
    print("Käyttö skellonajat tulostiedosto.txt")
    exit()

c_ajat = CDLL("/usr/share/skello/kellonajat.so")
c_ajat.suodata.restype = POINTER(c_float*1440)
c_ajat.tarkat_maarat.restype = POINTER(c_int*1440)

tnimi = sys.argv[1].encode('utf-8')
ctulos = c_ajat.alusta(c_char_p(tnimi))

alkusuod = 60
ajat = c_ajat.suodata(alkusuod).contents
tarkat = c_ajat.tarkat_maarat().contents
suurin = np.max(tarkat)

fig,ax = plt.subplots(figsize=(14,9))
xakseli = np.linspace(0,24,len(ajat))
pisteet, = ax.plot(xakseli, tarkat, '.', color='r')
viiva, = ax.plot(xakseli, ajat, color='k')
plt.xlim((0,24))
plt.ylim((0,suurin))
plt.xticks(np.arange(0,25,2))
plt.xlabel("Vuorokauden tunti")
plt.ylabel("Tuloksia minuutilla");

plt.subplots_adjust(top=0.9)
gaussliuku = Slider(
    ax = plt.axes([0.1, 0.964, 0.78, 0.026]),
    label = "$3\sigma$",
    valmin=0,
    valmax=720,
    valinit=alkusuod
)
rajausliuku = Slider(
    ax = plt.axes([0.1, 0.93, 0.78, 0.026]),
    label = 'alku',
    valmin=0,
    valmax=c_ajat.pituus(),
    valinit=0
)
vntanapit = CheckButtons(
    ax = plt.axes((0.0, 0.7, 0.08, 0.2)),
    labels = ('viiva', 'pisteet'),
    actives = (True, True)
)

ei_paivitusta = False
def paivita_gauss(arvo):
    global ei_paivitusta
    if ei_paivitusta:
        ei_paivitusta = False
        return
    arvo = np.floor(gaussliuku.val)
    ei_paivitusta = True
    gaussliuku.set_val(arvo) #laitetaan kokonaisluvuksi, mutta ei enää päivitetä
    ctulos = c_ajat.suodata(c_int(int(arvo)));
    for i,arvo in enumerate(ctulos.contents):
        ajat[i] = arvo
    viiva.set_ydata(ajat)
    plt.draw()

def paivita_rajaus(arvo):
    c_ajat.rajaa(c_int(int(arvo)), c_ajat.pituus())
    paivita_gauss(gaussliuku.val)

def painettaessa(tapaht):
    global ax
    if tapaht.key == 'left':
        gaussliuku.set_val(gaussliuku.val-1)
    elif tapaht.key == 'right':
        gaussliuku.set_val(gaussliuku.val+1)
    elif tapaht.key == 'enter':
        ax.set_ylim(np.min(ajat),np.max(ajat))
        plt.draw()
    elif tapaht.key == 'backspace':
        ax.set_ylim(0,suurin)
        plt.draw()

def vntapainallus(nimio):
    if nimio == 'viiva':
        viiva.set_visible(not viiva.get_visible())
    elif nimio == 'pisteet':
        pisteet.set_visible(not pisteet.get_visible())
    plt.draw()

gaussliuku.on_changed(paivita_gauss)
rajausliuku.on_changed(paivita_rajaus)
fig.canvas.mpl_connect('key_press_event', painettaessa)
vntanapit.on_clicked(vntapainallus)

plt.show()

c_ajat.vapauta()
