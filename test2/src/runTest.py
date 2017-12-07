#!/usr/bin/env python
import sys
from ROOT import *
gSystem.Load('libTest.so')

def test():
    print "testing"
    tf1 = trackFinder()
    tf1.CF_inBkg = "/data/Samples/xRayPol/topmetal1202/pede27.txt";
    tf1.CF_inData = "/data/Samples/xRayPol/20171107/out135.pd1";

    tf1.test();
    tf1.process();

def checkData():
    if len(sys.argv)<2:
        print "need to give file name"
        return
    filelist = sys.argv[1]

    tf1 = trackFinder()
    tf1.CF_inBkg = "/data/Samples/xRayPol/topmetal1202/pede27.txt";

    with open(filelist,'r') as f1:
        for line in f1.readlines():
            tf1.CF_inData = line.rstrip()
            tf1.process()

if __name__ == '__main__':
    if len(sys.argv)>1:
        checkData()
    else:
        test()
#     test()
