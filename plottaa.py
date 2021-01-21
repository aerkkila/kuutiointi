import matplotlib.pyplot as plt
import scipy.stats as stat
import numpy as np
import struct
import pymannkendall as mk
import pandas as pd

f = open("plottaa.bin", "rb");
a = [];
while 1:
    x = f.read(4);
    if len(x) < 4:
        break;
    x = struct.unpack("f", x)[0];
    if(not np.isinf(x) and not np.isnan(x)):
        a.append(x);
f.close();

y = np.array(a);
x = np.array(range(len(a)));
sr = pd.Series(y, x);

ts = mk.original_test(sr);
pns = stat.linregress(x,y);

plt.plot(a, 'o');
plt.plot(x*pns.slope+pns.intercept);
plt.plot(x*ts.slope+ts.intercept);
plt.xlim(right=len(y));
#plt.tight_layout();
plt.title('theil-senn: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f\npns: %.2f $\\frac{s}{1000}$, %.2f, p = %.3f' %(ts.slope*1000, ts.intercept, ts.p, pns.slope*1000, pns.intercept, pns.pvalue));
plt.show();
