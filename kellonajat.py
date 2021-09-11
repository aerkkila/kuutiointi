#!/usr/bin/python3

#Tämä kutsuu c-ohjelmaa tiedostossa kellonajat.c
#Piirtää kuvaajan montako tulosta on kullakin vuorokauden minuutilla

import matplotlib.pyplot as plt
import numpy as np
import sys, time
from ctypes import *

if len(sys.argv) < 2:
    print("Käyttö ./kellonajat.py tulostiedosto.txt")
    exit()

c_ajat = CDLL("./kellonajat.so")
c_ajat.kellonajat.restype = POINTER(c_float*1440)

tnimi = sys.argv[1].encode('utf-8')
ctulos = c_ajat.kellonajat(c_char_p(tnimi))

ajat = np.zeros(1440)
for i,arvo in enumerate(ctulos.contents):
    ajat[i] = arvo

c_ajat.vapauta(ctulos)

plt.plot(np.linspace(0,24,len(ajat)), ajat)
plt.xlim([0,24])
plt.xticks(np.arange(0,25,2))
plt.xlabel("Vuorokauden tunti")
plt.show()
