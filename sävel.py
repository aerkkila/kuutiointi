#!/usr/bin/python3

import sounddevice as sd
import numpy as np
import time

taajuus = 16000; #oltava vähintään 2*f1
Dt = 0.06; #vähintään 1/f0: 0,025, jos f0 = 40 Hz
#           samaten erottelukyky = 1/Dt = 25 Hz, jos 0,04 s

Ns = int(taajuus*Dt) #32000; 0,03 -> 960
sd.default.samplerate = taajuus;
sd.default.channels = (1,1);

#Yksittäisiäi virhesäveliä voi tulla ympäristön metelistä.
#Siksi vaaditaan asian toistamista kahdesti
#Sen jälkeen pitää tulla välissä tyhjää, jotta taas otetaan vastaan
def kuuntele_savelia():
    toisto = 0;
    while 1:
        data = sd.rec(Ns, blocking=1)[:,0];
        aika0 = time.time();
        #Spektri:
        #tarvitaan vain varianssi ja spektrin maksimi,
        #joten laatikkoikkuna kelpaa ja aiheuttaa vähiten laskentakuormaa
        #trendin poistaminen ei liene tarpeellista
        X = np.fft.fft(data)[0:Ns//4]
        if not (Ns % 2):
            X[-1] /= 2;

        Sxx = np.abs(X)**2; #skaalausta ei tarvita, koska käsitellään vain suhteita

        #Kohinan arvo haetaan yhdellä iteroinnilla
        #käytetään 1000 Hz:n pätkiä niin, että 1. f0 - 1000+f0; 2. 500+f0 - 1500+f0 jne
        n500ssa = int(500*Dt);
        alaraja = 0;
        ularaja = 2*n500ssa;
        pit = ularaja-alaraja;
        savel = 0;
        savelpiikki = 0;
        savelsuhde = 0;
        while ularaja < len(Sxx):

            ka0 = np.mean(Sxx[alaraja:ularaja]);
            ka1 = 0;
            raja = ka0*10;
            ohi = 0;
            maks = 0;
            maksind = 0;

            for i in range(alaraja, alaraja+pit):
                if Sxx[i] > raja:
                    ohi += 1;
                    if(Sxx[i] > maks):
                        maks = Sxx[i];
                        maksind = i;
                else:
                    ka1 += Sxx[i];

            alaraja += n500ssa;
            ularaja += n500ssa;

            ka1 = ka1/(ularaja-alaraja-ohi);
            arvo = maks-ka1;
            suhde = arvo/ka1
            if(suhde > 25):
                taaj = (maksind+1)/Dt;
                if(arvo > savelpiikki):
                    savel = taaj;
                    savelpiikki = arvo;
                    savelsuhde = suhde;
        if(70 < savel and savel < 3500):
            toisto += 1;
            if(toisto == 2):
                print(savel, savelsuhde);
            if(toisto == 1 and suhde > 250):
                toisto = 2;
                print(savel, savelsuhde);
        else:
            toisto = 0;

kuuntele_savelia();
