#!/usr/bin/python3

import scipy.stats as stat
import numpy as np
import sounddevice as sd
import time
import sys
import matplotlib.pyplot as plt

taajuus = 16000;
nuku0 = 0; #aika ennen kuin tehdään mitään
kohinaa = 1; #montako sekunta kuunnellaan taustakohinaa
tPak = 0.1; #yhden äänityspaketin kesto


nPist = int(taajuus*tPak); #äänityspaketin pituus: N
kohinaPit = int(taajuus*kohinaa); #kohinakuuntelun pituus: N

sd.default.samplerate = taajuus;
sd.default.channels = (1, 2);
sd.default.dtype = 'float32'; #pienempi aiheuttaa ylivuodon, käsittely liukulukuina

pArvo = lambda f, pitOs, pitNim: 1 - stat.f.cdf(f, pitOs, pitNim);
aani = [np.array([0]*nPist)]*2

class Odottaja(object):
    def __init__(self, var=1, alkukuuntelu=1):
        self.var = var;
        self.alkukuuntelu = alkukuuntelu;
        self.aInd = 0;
        self.pit = int(taajuus*kohinaa);
        if not self.alkukuuntelu:
            return;
        print("odota", end="");
        sys.stdout.flush();
        data = sd.rec(self.pit, blocking=1)[:,0];
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
            parv = pArvo(var/self.var, nPist, kohinaPit);
            if(parv < 1e-12):
                return;
            self.aInd = bInd;
            bInd = (bInd+1) % 2;
    def __exit__(self, exc_type, exc_value, traceback):
        pass;


od = Odottaja();

def yksiAani():
    with od:
        print("räjähti");
        aInd = od.aInd;
        bInd = (aInd+1) % 2;
        laskuri = 1;
        nauhoite = aani[od.aInd];
        while 0:
            sd.wait();
            aani[bInd] = sd.rec(nPist)[:,0];
            var = np.var(aani[aInd]);
            if(pArvo(var/od.var, nPist, nPist) > 1e-12):
                sd.stop();
                break;
            np.append(nauhoite, aani[aInd]);
            laskuri += 1;
            aInd = bInd;
            bInd = (bInd+1) % 2;
    return nauhoite, laskuri;

nauhoite, laskuri = yksiAani();
plt.plot(np.arange(0,len(nauhoite))/taajuus, nauhoite);
plt.title("laskuri = %i" %laskuri);
plt.show();
