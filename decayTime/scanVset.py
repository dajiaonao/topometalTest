#!/usr/bin/env python
import os, sys, re
from ROOT import *
from rootUtil import useAtlasStyle, waitRootCmd, savehistory, mkupHistSimple, get_default_fig_dir
funlist=[]

sDir = get_default_fig_dir()
sTag = 'test_'
sDirectly = False
if gROOT.IsBatch(): sDirectly = True

gROOT.LoadMacro('checkPixels.C+')
from ROOT import PixelChecker


def doScan():
#     dir1 = '/home/dzhang/work/topmetal2M/Samples/oct20_scan/Vreset1th/'
    dir1 = '/home/dzhang/work/topmetal2M/Samples/oct20_scan/Vreset2th/'
    tag = 'widthS2_'

    if len(sys.argv)<3: 
        print "Usage:", sys.argv[0], "filename Vreset"
        return
    fname = sys.argv[1]
    V = float(sys.argv[2])

    print "processing", fname, 'for', V
    ch1 = TChain('tree1')
    ch1.Add(dir1+fname)

    pc1 = PixelChecker();
    pc1.foutName = tag+fname
    print "output root file to", pc1.foutName
    pc1.setupOutTree()

    pc1.inTree = ch1
    pc1.Vreset = V
    pc1.checkAll()

def sequential():
    dir1 = '/home/dzhang/work/topmetal2M/Samples/oct20_scan/Vreset1th/'
    flist = [('out125.root', 0.82)
            ,('out126.root', 0.825)
            ,('out127.root', 0.83)
            ,('out128.root', 0.835)
            ,('out129.root', 0.84)
            ,('out130.root', 0.845)
            ,('out131.root', 0.85)
            ,('out132.root', 0.855)
            ,('out133.root', 0.86)
            ,('out134.root', 0.865)
            ,('out135.root', 0.87)
            ,('out136.root', 0.815)
            ,('out137.root', 0.81)
            ,('out138.root', 0.805)
            ,('out139.root', 0.80)
            ,('out140.root', 0.795)
            ,('out141.root', 0.79)
            ,('out142.root', 0.785)
            ,('out143.root', 0.78)
            ,('out144.root', 0.775)
            ,('out145.root', 0.77)
            ,('out146.root', 0.765)
            ,('out147.root', 0.76)
            ,('out148.root', 0.755)
            ,('out149.root', 0.75)
            ,('out150.root', 0.745)
            ,('out151.root', 0.74)
            ,('out152.root', 0.735)
            ,('out153.root', 0.73)
            ]
  
    for (fname, V) in flist:
        print "processing", fname, 'for', V
        ch1 = TChain('tree1')
        ch1.Add(dir1+fname)

        pc1 = PixelChecker();
        pc1.foutName = 'width_'+fname
        print "output root file to", pc1.foutName
        pc1.setupOutTree()

        pc1.inTree = ch1
        pc1.Vreset = V
        pc1.checkAll()

        del pc1
        del ch1

def test():
    '''Memeory problem....'''
    dir1 = '/home/dzhang/work/topmetal2M/Samples/oct20_scan/Vreset1th/'
    flist = [('out125.root', 0.82)
            ,('out126.root', 0.825)
            ,('out127.root', 0.83)
            ,('out128.root', 0.835)
            ,('out129.root', 0.84)
            ,('out130.root', 0.845)
            ,('out131.root', 0.85)
            ,('out132.root', 0.855)
            ,('out133.root', 0.86)
            ,('out134.root', 0.865)
            ,('out135.root', 0.87)
            ,('out136.root', 0.815)
            ,('out137.root', 0.81)
            ,('out138.root', 0.805)
            ,('out139.root', 0.80)
            ,('out140.root', 0.795)
            ,('out141.root', 0.79)
            ,('out142.root', 0.785)
            ,('out143.root', 0.78)
            ,('out144.root', 0.775)
            ,('out145.root', 0.77)
            ,('out146.root', 0.765)
            ,('out147.root', 0.76)
            ,('out148.root', 0.755)
            ,('out149.root', 0.75)
            ,('out150.root', 0.745)
            ,('out151.root', 0.74)
            ,('out152.root', 0.735)
            ,('out153.root', 0.73)
            ]

  
    k = 0
    while k<len(flist):
        pc1 = PixelChecker();
        pc1.foutName = 'decayWidth_f{0:d}.root'.format(k)
        print "output root file to", pc1.foutName
        pc1.setupOutTree()

        for (fname, V) in flist[k:min(k+5,len(flist))]:
            print "processing", fname, 'for', V
            ch1 = TChain('tree1')
            ch1.Add(dir1+fname)
            pc1.inTree = ch1
            pc1.Vreset = V
            pc1.checkAll()
        k += 5
        del pc1

#     waitRootCmd()
funlist.append(test)

if __name__ == '__main__':
    savehistory('.')
    useAtlasStyle()
    doScan()
#     for fun in funlist: print fun()
#     sequential()
