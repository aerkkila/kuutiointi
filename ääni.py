#!/usr/bin/python3

import sounddevice as sd
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sig

taajuus = 44100;
fps = 25; #enintään siirtoja sekunnissa (ikkunan pituus)
f0 = 0; #matalin sallittu taajuus
f1 = 6000; #korkein sallittu taajuus
nfa = 1;

ikkuna = sig.windows.blackmanharris(taajuus//fps);

skaala = 2/(nfa*np.linalg.norm(ikkuna)**2);

xdata = np.arange(1, taajuus//fps//2+1)*fps; #taajuudet
ind0 = 0;
for i in range(len(xdata)):
    if(xdata[i] < f0):
        continue
    ind0 = i;
    break;

ind1 = len(xdata);
for i in range(ind0, len(xdata)):
    if(xdata[i] < f1):
        continue
    ind1 = i;
    break;

xdata = xdata[ind0:ind1]

#––––––––––––––––––––––––––––––––––––––––––––––––––––#

def spektri(x):
    X = np.fft.fft(sig.detrend(x)*ikkuna);

    #mikä on skaalaus?

    if nfa == 1:
        S11 = (np.abs(X[ind0:ind1])**2)*skaala;
        return S11;
    

#––––––––––––––––––––––––––––––––––––––––––––––––––––#

sd.default.samplerate = taajuus;
sd.default.channels = (1, 2);

plt.ion();
fig = plt.figure();
ax = fig.add_subplot(111);
viiva, = ax.plot(xdata, xdata*0);
ax.set_ylim(0,1);

while 1:
    try:
        data = sd.rec(taajuus//fps);
        sd.wait();
        ydata = spektri(data[:,0]);
        viiva.set_data(xdata, ydata);
        fig.canvas.draw();
        fig.canvas.flush_events();
    except KeyboardInterrupt:
        break;
