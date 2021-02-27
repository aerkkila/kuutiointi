#!/usr/bin/python3

import numpy as np
import sounddevice as sd
import time
import sys
import matplotlib.pyplot as plt

taajuus = 16000;
nuku0 = 0; #aika ennen kuin tehdään mitään
kohinaa = 1; #montako sekunta kuunnellaan taustakohinaa
tPak = 0.1; #yhden äänityspaketin kesto
varKerr = 10; #monikokertainen varianssi aloittaa ja lopettaa
tLoppu = 1; #kuinka kauan oltava hiljaista, että loppuu
nLoppu = np.ceil(tLoppu/tPak);

nPist = int(taajuus*tPak); #äänityspaketin pituus: N
kohinaPit = int(taajuus*kohinaa); #kohinakuuntelun pituus: N

sd.default.samplerate = taajuus;
sd.default.channels = (1, 2);
sd.default.dtype = 'float32'; #pienempi aiheuttaa ylivuodon, käsittely liukulukuina

aani = [np.array([0]*nPist)]*2

class Odottaja(object):
    def __init__(self, var=1, alkukuuntelu=1):
        self.var = var;
        self.aInd = 0;
        if not alkukuuntelu:
            return;
        print("odota", end="");
        sys.stdout.flush();
        data = sd.rec(int(taajuus*kohinaa), blocking=1)[:,0];
        self.var = np.var(data);
        print("\r       \r", end="");
    def __enter__(self):
        time.sleep(nuku0);
        self.aInd = 0;
        bInd = 1;
        aani[self.aInd] = sd.rec(nPist)[:,0];
        while 1:
            sd.wait();
            aani[bInd] = sd.rec(nPist)[:,0];
            var = np.var(aani[self.aInd]);
            if(var > self.var*varKerr):
                print("hep");
                return;
            self.aInd = bInd;
            bInd = (bInd+1) % 2;
    def __exit__(self, exc_type, exc_value, traceback):
        pass;


od = Odottaja();

def yksiAani():
    with od:
        nauhoite = np.array([]);
        aInd = od.aInd;
        bInd = (aInd+1) % 2;
        laskuri = 0;
        nHiljaa = 0;
        while nHiljaa < nLoppu:
            nauhoite = np.append(nauhoite, aani[aInd]);
            laskuri += 1;
            sd.wait();
            aani[aInd] = sd.rec(nPist)[:,0];
            var = np.var(aani[bInd]);
            if(var < od.var*varKerr):
                nHiljaa += 1;
            else:
                nHiljaa = 0;
            aInd = bInd;
            bInd = (bInd+1) % 2;
    return nauhoite, laskuri;

nauhoite, laskuri = yksiAani();
plt.plot(np.arange(0,len(nauhoite))/taajuus, nauhoite);
plt.title("laskuri = %i" %laskuri);
plt.show();
