#!/usr/bin/python3

#Tämä kutsuu c-ohjelmaa tiedostossa kellonajat.c
#Piirtää kuvaajan montako tulosta on kullakin vuorokauden minuutilla
#Gaussin alipäästösuodattimen herkkyyttä voi säätää liu'ulla

import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import numpy as np
import sys, time
from ctypes import *

if len(sys.argv) < 2:
    print("Käyttö ./kellonajat.py tulostiedosto.txt")
    exit()

c_ajat = CDLL("./kellonajat.so")
c_ajat.suodata.restype = POINTER(c_float*1440)

tnimi = sys.argv[1].encode('utf-8')
ctulos = c_ajat.alusta(c_char_p(tnimi))
ajat = np.zeros(1440)

alkusuod = 60
ctulos = c_ajat.suodata(alkusuod);
for i,arvo in enumerate(ctulos.contents):
    ajat[i] = arvo

fig,ax = plt.subplots(figsize=(14,9))
viiva, = ax.plot(np.linspace(0,24,len(ajat)), ajat)
plt.xlim([0,24])
plt.xticks(np.arange(0,25,2))
plt.xlabel("Vuorokauden tunti")
plt.ylabel("Tuloksia minuutilla");

plt.subplots_adjust(top=0.9)
gauss_liuku = Slider(
    ax = plt.axes([0.1, 0.93, 0.78, 0.07]),
    label = "$3\sigma$",
    valmin=0,
    valmax=720,
    valinit=alkusuod
)

ei_paivitusta = False
def paivita(arvo):
    global ei_paivitusta
    if ei_paivitusta:
        ei_paivitusta = False
        return
    arvo = np.floor(gauss_liuku.val)
    ei_paivitusta = True
    gauss_liuku.set_val(arvo) #laitetaan kokonaisluvuksi, mutta ei enää päivitetä
    ctulos = c_ajat.suodata(c_int(int(arvo)));
    for i,arvo in enumerate(ctulos.contents):
        ajat[i] = arvo
    viiva.set_ydata(ajat)
    plt.draw()

def painettaessa(tapaht):
    global ax
    if tapaht.key == 'left':
        gauss_liuku.set_val(gauss_liuku.val-1)
    elif tapaht.key == 'right':
        gauss_liuku.set_val(gauss_liuku.val+1)
    elif tapaht.key == 'enter':
        ax.set_ylim(np.min(ajat),np.max(ajat))
        plt.draw()

gauss_liuku.on_changed(paivita)
fig.canvas.mpl_connect('key_press_event', painettaessa)

plt.show()

c_ajat.vapauta()
