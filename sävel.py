#!/usr/bin/python3

import sounddevice as sd
import numpy as np
import sysv_ipc as ipc
import scipy.signal as sig

taajuus = 16000 #oltava vähintään 2*f1
ikk = 0.1 #ikkunan pituus sekunteina; mieluiten taajuus*ikk = 2^m
          #vähintään 1/f0: 0,025, jos f0 = 40 Hz
          #samaten erottelukyky = 1/ikk = 25 Hz, jos 0,04 s
Dt = 0.08 #aikaväli, jolla spektri päivitetään

Ns = int(taajuus*Dt) #16000; 0,06 -> 960
ikks = int(taajuus*ikk)
sd.default.samplerate = taajuus;
sd.default.channels = 1;

### muistiin liittyminen ###
try:
    avain = ipc.ftok("/tmp", 44)
    shm = ipc.SharedMemory(avain, 0, 0)
    shm.attach(0,0)
except Exception as e:
    print("Muistiin liittyminen pythonilla: %s" %e)

#Kohinan arvo haetaan yhdellä iteroinnilla
#käytetään 1000 Hz:n pätkiä niin, että 1. f0 - 1000+f0; 2. 500+f0 - 1500+f0 jne
n500ssa = int(500*Dt);
alaraja = 0;
ularaja = 2*n500ssa;
pit = ularaja-alaraja;

toisto = 0 #vaaditaan kahta toistoa virheitten välttämiseksi
ikkuna = sig.windows.blackmanharris(ikks)

def taajuus(data):
    global toisto
    X = np.fft.rfft((sig.detrend(data))*ikkuna)[0:Ns//2]
    if not (Ns % 2):
        X[-1] /= 2;

    Sxx = np.abs(X)**2; #skaalausta ei tarvita, koska käsitellään vain suhteita
    savel = 0;
    savelpiikki = 0;
    savelsuhde = 0;
    alaraja = 0;
    ularaja = 2*n500ssa;
    while ularaja < len(Sxx):

        ka0 = np.mean(Sxx[alaraja:ularaja]);
        ka1 = 0;
        raja = ka0*30;
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
        if(ka1 == 0.0):
            continue
        suhde = arvo/ka1
        if(suhde > 60):
            taaj = (maksind+1)/Dt;
            if(arvo > savelpiikki):
                savel = taaj;
                savelpiikki = arvo;
                savelsuhde = suhde;
    if(70 < savel and savel < 3500):
        toisto += 1;
        if(toisto > 1):
            shm.write(np.float32(savel));
        if(toisto == 1 and suhde > 250):
            shm.write(np.float32(savel));
    else:
        toisto = 0;

data = np.zeros(ikks)

def kutsupyynt(d, a, b, c):
    global data
    data = np.roll(data, -Ns)
    data[-Ns:] = d[:,0]
    taajuus(data)

with sd.InputStream(callback=kutsupyynt, blocksize=Ns):
    while True:
        sd.sleep(500)
