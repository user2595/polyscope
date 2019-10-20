# Samples from a matplotlib colormap and prints a constant string to stdout, which can then be embedded in a C++ program

import math
import numpy as np
import matplotlib.cm

import colorsys

def plain_cmap(s):
    cm = matplotlib.cm.get_cmap('RdPu')
    return cm(pow(s, 0.8))

divisions = 15
offset = 0.75
def cmap(s):
    ds = 1/divisions
    if (abs(s % ds) < ds / 2.):
        return plain_cmap(0.05 + 0.95 * offset * s)
    else:
        return plain_cmap(offset * s + (1-offset))



nValues = 500;

# get a matplotlib colormap
# cmapName = 'Spectral'
# cmap = matplotlib.cm.get_cmap(cmapName)

# get a cmocean colormap
# import cmocean
# cmapName = 'phase'
# cmap = cmocean.cm.phase

cmapName = 'heat'


print("const ValueColorMap CM_" + cmapName.upper() + " = {")
print("    \"" + cmapName + "\",")

dataStr = "{"
for i in range(nValues):

    floatInd = float(i) / (nValues-1)
    color = cmap(floatInd)

    dataStr += "{" + str(color[0]) + "," + str(color[1]) + "," + str(color[2]) + "},"

dataStr += "}"
print(dataStr)

print("};")


# Generate a constant colormap
# constCmapName = 'const_red'
# constCmapColor = (196/255., 133/255., 133/255.)

# print("static const Colormap CM_" + constCmapName.upper() + " = {")
# print("    \"" + constCmapName + "\",")

# dataStr = "{"
# for i in range(nValues):
    # dataStr += "{" + str(constCmapColor[0]) + "," + str(constCmapColor[1]) + "," + str(constCmapColor[2]) + "},"

# dataStr += "}"
# print(dataStr)

# print("};")
