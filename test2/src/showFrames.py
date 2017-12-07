#!/usr/bin/env python
import sys
from ROOT import *
from rootUtil import waitRootCmdX
gSystem.Load('libTest.so')

def test():
    tf1 = trackFinder()
    tf1.CF_inBkg = "/data/Samples/xRayPol/topmetal1202/pede27.txt";
    tf1.CF_inData = "/data/Samples/xRayPol/20171107/out135.pd1";

    narg = len(sys.argv)
    fstart = int(sys.argv[1]) if narg>1 else 0
    fN = int(sys.argv[2]) if narg>2 else 1
    mode = int(sys.argv[3]) if narg>3 else 1 # mode: 0->raw, 1->bkg subtracted, 2->signficance, 3->only ROI

    pd1 = tf1.drawFrames(fstart, fN, mode)
    waitRootCmdX()
    

if __name__ == '__main__':
    test()
