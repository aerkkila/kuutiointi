#!/usr/bin/python
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import scipy.stats as stat
import numpy as np
import struct, locale
import pymannkendall as mk
import pandas as pd
import sys, os

os.close(int(sys.argv[2]))
fd = int(sys.argv[1])
pit, = struct.unpack("i", os.read(fd,4))
ajat = np.empty(pit,float)
ind = np.empty(pit,int)
dnfind = []

kohta=0
for i in range(pit):
    x, = struct.unpack("f", os.read(fd,4))
    if(not np.isinf(x)):
        ajat[kohta] = x
        ind[kohta] = i
        kohta += 1
    else:
        dnfind.append(i)

os.close(fd)
ajat = ajat[:kohta]
ind = ind[:kohta]
dnfind = np.array(dnfind)
sr = pd.Series(ajat,index=ind)

ts = mk.original_test(sr)
pns = stat.linregress(ind, ajat)
plt.plot(ind, ajat, 'o', color='b')
plt.plot(sr.index, sr.index*pns.slope+pns.intercept, label="pns")
plt.plot(sr.index, sr.index*ts.slope+ts.intercept, label="ts")
plt.xlim(-1,pit)
plt.ylim(top=np.nanmax(ajat))

#dnfien piirtäminen
ala,ula = plt.ylim()
dnfaika = np.zeros(np.shape(dnfind)) + ula
plt.plot(dnfind, dnfaika, 'o', color='r')

plt.legend()
locale.setlocale(locale.LC_ALL, os.getenv('LANG'))
plt.title(locale.format_string("theil-senn: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f\n"
                               "pns: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f",
                               (ts.slope*1000, ts.intercept, ts.p,
                                pns.slope*1000, pns.intercept, pns.pvalue)))
paikallistaja = ticker.ScalarFormatter(useLocale=True)
plt.gca().yaxis.set_major_formatter(paikallistaja)
plt.tight_layout()
plt.show()
