#!/usr/bin/python
import matplotlib.pyplot as plt
import scipy.stats as stat
import numpy as np
import struct, locale
import pymannkendall as mk
import pandas as pd

f = open(".kuvaaja.bin", "rb")
ajat = []
ind = []
dnfind = []
i = 0
while 1:
    x = f.read(4)
    if len(x) < 4:
        break
    x = struct.unpack("f", x)[0]
    if(not np.isinf(x) and not np.isnan(x)):
        ajat.append(x)
        ind.append(i)
    else:
        dnfind.append(i)
    i+=1
f.close()

ajat = np.array(ajat)
dnfind = np.array(dnfind)
ind = np.array(ind)
sr = pd.Series(ajat, ind)

ts = mk.original_test(sr)
pns = stat.linregress(ind,ajat)

plt.plot(ind, ajat, 'o', color='b')
plt.plot(ind, ind*pns.slope+pns.intercept, label="pns")
plt.plot(ind, ind*ts.slope+ts.intercept, label="ts")
plt.xlim(right=np.max(ind))
plt.ylim(top=np.max(ajat))

#dnfien piirtäminen
ala,ula = plt.ylim()
dnfaika = np.zeros(np.shape(dnfind)) + ula
plt.plot(dnfind, dnfaika, 'o', color='r')

plt.legend()
locale.setlocale(locale.LC_ALL, 'fi_FI.utf8')
plt.title(locale.format_string("theil-senn: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f\n"
                                   "pns: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f",
                                   (ts.slope*1000, ts.intercept, ts.p,
                                        pns.slope*1000, pns.intercept, pns.pvalue)))
plt.show()
