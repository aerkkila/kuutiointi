#!/usr/bin/python
from matplotlib.pyplot import *
import matplotlib.ticker as ticker
import numpy as np
import struct, locale, sys, os

os.close(int(sys.argv[2]))
fd = int(sys.argv[1])
pit, = struct.unpack("i", os.read(fd,4))
param = np.ndarray(3, np.float64, buffer=os.read(fd, 8*3))
ajat = np.ndarray(pit, np.float32, buffer=os.read(fd, 4*pit))
indeksit = np.arange(0, pit, 1)
infmaski = np.isinf(ajat)
aikamaski = ~infmaski
x = indeksit[aikamaski]
x_dnf = indeksit[infmaski]
ajat = ajat[aikamaski]

os.close(fd)
#sr = pd.Series(ajat,index=x)

#ts = mk.original_test(sr)
#pns = stat.linregress(x, ajat)
pns_x = np.array([x[0], x[-1]])
pns_y = pns_x*param[1] + param[0]
plot(x, ajat, 'o', color='b')
plot(pns_x, pns_y, label="pns")
#plot(sr.index, sr.index*ts.slope+ts.intercept, label="ts")
xlim(-1,pit)
ylim(top=np.max(ajat))

#dnfien piirtäminen
ala,ula = ylim()
dnfaika = np.zeros(x_dnf.shape) + ula
plot(x_dnf, dnfaika, 'o', color='r')

legend()
locale.setlocale(locale.LC_ALL, os.getenv('LANG'))
#title(locale.format_string("theil-senn: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f\n"
#                               "pns: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f",
#                               (ts.slope*1000, ts.intercept, ts.p,
#                                pns.slope*1000, pns.intercept, pns.pvalue)))
title(locale.format_string("pns: %.3f $\\frac{s}{1000}$, %.3f, p = %.5f",
                           (param[1]*1000, param[0], param[2])))
paikallistaja = ticker.ScalarFormatter(useLocale=True)
gca().yaxis.set_major_formatter(paikallistaja)
tight_layout()
show()
