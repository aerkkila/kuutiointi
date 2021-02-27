#!/usr/bin/python3

import sounddevice as sd
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sig

taajuus = 16000;
fps = 25; #ikkunoita sekunnissa
f0 = 0; #matalin sallittu taajuus
f1 = 8000; #korkein sallittu taajuus
nfa = 1;
varT = 4; #moneltako sekunnilta varianssi näkyy
varPit = fps*varT;
osa = 5; #aallon näkyvyyden takia näytetään vain osa luetusta äänestä

ikkuna = sig.windows.blackmanharris(taajuus//fps);

skaala = 2/len(ikkuna);

taaj = np.arange(1, taajuus//fps//2+1, nfa)*fps; #taajuudet
aika = np.arange(0, taajuus//fps);

var = np.array([np.nan]*(varPit));
varaika = np.arange(0,varPit)/fps;

ind0 = 0;
for i in range(len(taaj)):
    if(taaj[i] < f0):
        continue
    ind0 = i;
    break;

ind1 = len(taaj);
for i in range(ind0, len(taaj)):
    if(taaj[i] < f1):
        continue
    ind1 = i;
    break;

taaj = taaj[ind0:ind1]

#––––––––––––––––––––––––––––––––––––––––––––––––––––#

def spektri(x):
    X = np.fft.fft(sig.detrend(x)*ikkuna);

    if nfa == 1:
        S11 = (np.abs(X[ind0:ind1])**2)*skaala;
        return S11;

    ala=ind0; ula=ala+nfa;
    pit = ind1-ind0; #nfa on jo huomioitu
    S11 = np.zeros(pit);
    for i in range(pit):
        S11[i] = np.sum(np.abs(X[ala:ula])**2)*skaala;
        ala += nfa; ula += nfa;

    return S11;

#––––––––––––––––––––––––––––––––––––––––––––––––––––#

sd.default.samplerate = taajuus;
sd.default.channels = (1, 2);
sd.default.dtype = 'float32';


plt.ion();
fig = plt.figure(figsize=[12,10]);

ax1 = fig.add_subplot(311);
viiva1, = ax1.plot(taaj, taaj*0+1);
ax1.set_ylim(0, 70)
plt.title("Spektri");

ax2 = fig.add_subplot(312);
viiva2, = ax2.plot(aika[:len(aika)//osa]*0+1);
ax2.set_ylim(-128, 128);
plt.title("ääniaalto");

ax3 = fig.add_subplot(313);
viiva3, = ax3.plot(varaika, var);
ax3.set_xlim(0, varT);

kuva = 0;
while 1:
    try:
        data = sd.rec(taajuus//fps, blocking=1)[:,0];
        S11 = spektri(data);
        viiva1.set_data(taaj, S11);
        ax1.set_ylim(0, np.max(S11));

        viiva2.set_data(aika[:len(data)//osa], data[:len(data)//osa]);
        maks = np.max(data);
        ax2.set_ylim(-maks, maks);

        var[kuva] = np.var(data);
        viiva3.set_data(varaika, var);
        maks = np.max(var[~np.isnan(var)]);
        ax3.set_ylim(0, maks);
        fig.canvas.draw();
        fig.canvas.flush_events();

        kuva += 1;
        kuva %= varPit;
    except KeyboardInterrupt:
        break;

#––––––––––––––––––––––––––––––––––––––––––––––––––––#
