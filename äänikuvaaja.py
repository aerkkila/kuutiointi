#!/usr/bin/python
import os, sys, struct
import numpy as np
from matplotlib.pyplot import *

luku = os.fdopen(int(sys.argv[1]), 'rb')
kirj = os.fdopen(int(sys.argv[4]), 'wb')
os.close(int(sys.argv[2]))
os.close(int(sys.argv[3]))
apu = luku.read(8)
raitoja, raidan_pit = struct.unpack("ii", apu);
data = np.empty((raitoja, raidan_pit), np.float32)
for r in range(raitoja):
    for i in range(raidan_pit):
        data[r,i], = struct.unpack("f", luku.read(4))
plot(data[0])
tight_layout()
show()
