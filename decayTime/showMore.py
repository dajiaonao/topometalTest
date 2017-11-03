#!/usr/bin/env python
import os, sys, re
from ROOT import *
from rootUtil import useAtlasStyle, waitRootCmdX, savehistory, mkupHistSimple, get_default_fig_dir
funlist=[]

sDir = get_default_fig_dir()
sTag = 'test_'
sDirectly = False
if gROOT.IsBatch(): sDirectly = True


def show2D():
    ls1 = [0.74, 0.76, 0.78, 0.79, 0.80, 0.82, 0.84, 0.87]

    ch1 = TChain('tree2')
    ch1.Add('widthS2_out*.root')
    tag = 'Chip 2, '

    lt = TLatex()

    c1= TCanvas()
    c1.Divide(4,2)
   
    for i,v in enumerate(ls1):
        c1.cd(i+1)
        gPad.SetRightMargin(0.16)
        ch1.Draw("decayWidth:pID/72:pID%72>>h"+str(i)+"(72,0,72,72,0,72)","abs(Vreset-{0:.3f})<0.001".format(v),"profcolz")
        lt.DrawLatexNDC(0.2,0.96,tag+"V_{reset}="+"{0:.3f}".format(v))
        hi = gPad.GetPrimitive('h'+str(i))
        hi.SetMaximum(160)
        gPad.SetLogz()
    waitRootCmdX()


def addCurve(ch1, vreset, h, lc):
    ch1.Draw("decayWidth>>"+h.GetName(),"abs(Vreset-{0:.3f})<0.001".format(vreset))
    h.SetLineColor(lc)
    return h


def efficiency():
    ch1 = TChain('tree2')
    ch1.Add('widthS1_out*.root')

    g1 = TGraph()

    norm = 1./5184
    ### 0.73 to 0.87: 0.14/0.05+1 = 29
    for i in range(29):
        v = 0.73 + 0.005*i
        eff = norm*ch1.GetEntries("decayWidth>10&&decayWidth<150&&abs(Vreset-{0:.3f})<0.001".format(v))
        g1.SetPoint(i, v, eff)
    g1.Draw()
    g1.GetYaxis().SetTitle("Efficinecy")
    g1.GetXaxis().SetTitle("V_{reset}")

    lt = TLatex()
    lt.DrawLatexNDC(0.4,0.5,"10<N_{frame}<150")


    waitRootCmdX()


def getEfficiency(files):
    ch1 = TChain('tree2')
    ch1.Add(files)

    g1 = TGraph()

    norm = 1./5184
    ### 0.73 to 0.87: 0.14/0.05+1 = 29
    for i in range(29):
        v = 0.73 + 0.005*i
        eff = norm*ch1.GetEntries("decayWidth>10&&decayWidth<150&&abs(Vreset-{0:.3f})<0.001".format(v))
        g1.SetPoint(i, v, eff)
    return g1


def efficiency2():
    g1 = getEfficiency("widthS1_out*.root")
    g2 = getEfficiency("widthS2_out*.root")

    g1.Draw()
    g1.GetYaxis().SetTitle("Efficinecy")
    g1.GetXaxis().SetTitle("V_{reset}")
    g2.SetMarkerColor(2)
    g2.SetMarkerStyle(24)
    g2.SetLineColor(2)
    g2.SetLineStyle(2)
    g2.Draw("PLsame")

    lt = TLatex()
    lt.DrawLatexNDC(0.4,0.5,"10<N_{frame}<150")

    lg = TLegend(0.7,0.8,0.9,0.9)
    lg.SetFillStyle(0)
    lg.AddEntry(g1,"Chip 1", 'lp')
    lg.AddEntry(g2,"Chip 2", 'lp')
    lg.Draw()


    waitRootCmdX()



def test():
    dir0 = os.getenv('SAMPLEDIR_LAMB')
    print dir0
    ch1 = TChain('tree2')
#     ch1.Add('fout.root')
    ch1.Add('widthS2_out*.root')
    ch1.Show(0)

    v1 = 0.82
    v2 = 0.79
    v3 = 0.80
#     v4 = 0.795
    v4 = 0.78
    vr1 = 'V_{reset} = '+'{0:.3f}'.format(v1)
    vr2 = 'V_{reset} = '+'{0:.3f}'.format(v2)
    vr3 = 'V_{reset} = '+'{0:.3f}'.format(v3)
    vr4 = 'V_{reset} = '+'{0:.3f}'.format(v4)


    h1 = TH1F("h1","h1;N_{frame};# of Pixel",40,0,160)
    h2 = addCurve(ch1, v2, h1.Clone('h2'), 2)
    h3 = addCurve(ch1, v3, h1.Clone('h3'), 4)
    h4 = addCurve(ch1, v4, h1.Clone('h4'), 3)


    ch1.Draw("decayWidth>>h1","abs(Vreset-{0:.3f})<0.001".format(v1))
    h2.Draw("same")
    h3.Draw("same")
    h4.Draw('same')

    h1.GetYaxis().SetRangeUser(0.9, 1.15*max(h1.GetBinContent(h1.GetMaximumBin()),h2.GetBinContent(h2.GetMaximumBin()),h3.GetBinContent(h3.GetMaximumBin())))
    
    h1.GetYaxis().SetRangeUser(0.9, 1.15*max([h.GetBinContent(h.GetMaximumBin()) for h in [h1,h2,h3,h4]]))

    lg = TLegend(0.6,0.8,0.86,0.9)
    lg.SetFillStyle(0)
    lg.AddEntry(h1,vr1,'l')
    lg.AddEntry(h2,vr2,'l')
    lg.AddEntry(h3,vr3,'l')
    lg.AddEntry(h4,vr4,'l')
    lg.Draw()

    waitRootCmdX()
funlist.append(test)

if __name__ == '__main__':
    savehistory('.')
    useAtlasStyle()
    show2D()
#     for fun in funlist: print fun()
#     efficiency2()
