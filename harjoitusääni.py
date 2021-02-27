#!/usr/bin/python3

import scipy.stats as stat
import numpy as np
import sounddevice as sd
import time
import sys
import matplotlib.pyplot as plt

taajuus = 16000;
nuku0 = 0.2; #aika ennen kuin tehdään mitään
kohinaa = 1.5; #montako sekunta kuunnellaan taustakohinaa
tPak = 1; #yhden äänityspaketin kesto

nPist = int(taajuus*tPak); #äänityspaketin pituus: N
kohinaPit = int(taajuus*kohinaa); #kohinakuuntelun pituus: N

sd.default.samplerate = taajuus;
sd.default.channels = (1, 2);
sd.default.dtype = 'float32'; #pienempi aiheuttaa ylivuodon, käsittely liukulukuina

pArvo = lambda f, pitOs, pitNim: 1 - stat.f.cdf(f, pitOs, pitNim);

class Odottaja(object):
    def __init__(self, var=1, alkukuuntelu=1):
        self.var = var;
        self.alkukuuntelu = alkukuuntelu;
        self.aani = np.array([[0]*nPist, [0]*nPist]);
        self.onko0vai1 = 0;
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
        self.onko0vai1 = 0;
        self.aani[0] = sd.rec(nPist)[:,0];
        while 1:
            a = self.onko0vai1;
            b = (a + 1) % 2;
            sd.wait();
            self.aani[b] = sd.rec(nPist)[:,0];
            var = np.var(self.aani[a][0:100]);
            print(np.max(self.aani[a]));
            print("%.3f\t%.3f" %(var, self.var))
            parv = pArvo(var/self.var, nPist, kohinaPit);
            if(parv < 1e-12):
                sd.wait();
                return;
            self.onko0vai1 = b;
    def __exit__(self, exc_type, exc_value, traceback):
        pass;


od = Odottaja();

for i in range(3):
    with od:
        print("räjähti");
        aani = np.array([[0]*nPist, [0]*nPist]);
        aInd = 0;
        bInd = 1;
        aani[aInd] = sd.rec(nPist)[:,0];
        laskuri = 1;
        nauhoite = od.aani[od.onko0vai1];
        if(pArvo(od.aani[od.onko0vai1], nPist, nPist) != 0):
            sd.stop();
            break;
        np.append(nauhoite, od.aani[od.onko0vai1]);
        laskuri += 1;
        while 1:
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
        plt.plot(np.arange(0,len(nauhoite))/taajuus, nauhoite);
        plt.title("laskuri = %i" %laskuri);
        plt.show();
