#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <TError.h>
#include <TPad.h>
#include <TVector3.h>
#include <TObject.h>
#include <TPad.h>
#include <TStyle.h>
#include "rootlibX.h"
#include "trackFinder.h"

using namespace std;
ClassImp(trackFinder)

void trackFinder::test(){
  std::cout << "in trackFinder" << std::endl;
  Info("trackFinder", "testing %s", "trackFinder");
 }

float trackFinder::distance(Hit& h1, Hit& h2){
  int adc1 = h1.pID % 8;
  int adc2 = h2.pID % 8;
  int dx = (h1.pID/8)%72-(h2.pID/8)%72;
  int dy = (h1.pID/8)/72-(h2.pID/8)/72;

  if (adc1!=adc2) return 999;

  return sqrt(dx*dx+dy*dy);
}

void trackFinder::setupBkg(placData_1* pd1){
  std::ifstream inPD(CF_inBkg.c_str());
  if(!inPD.is_open()) {
    cout << "No pedestal table found!" << endl;
    return ;
   }

  const int NADC = pd1->adcCha();
  const int NChan = NADC*pd1->nPix();
  if(!meanPed) meanPed = new float[NChan];
  if(!rmsPed) rmsPed = new float[NChan];

  int iChipT = 0, iPixelT = 0;
  float pedestalT = 0., noiseT = 0.;
  while(!inPD.eof()) {
    inPD >> iChipT >> iPixelT >> pedestalT >> noiseT;
    meanPed[iChipT+iPixelT*NADC] = pedestalT;
    rmsPed[iChipT+iPixelT*NADC] = noiseT;
   }		
 }

void trackFinder::process(){
  cout << "Processing file: " << CF_inData << endl;
  /// setup data
  char* fn = &CF_inData[0];
  placData_1 pd1;
  pd1.read(fn);
  pd1.print();

  setupBkg(&pd1);

  int NCut = 2;
  int ZnCut = 4;
  int ZnCut2 = 2;
  int ZnCut3 = 1;

  //// get the data vector
  const int NPIX = pd1.nPix()*pd1.adcCha();
  cout << NPIX << endl;
  size_t nData = pd1.dataSize()/sizeof(short);

  vector< float > data(nData,0);
  short* ps = (short*)pd1.p;
  for(int i=0; i<nData; i++) {
    data[i] = ps[i] - meanPed[i%NPIX];
   }

  /// start operating on data vector
  /// -- loop over pixels to find the hits
  const size_t NFRAME = pd1.nFrame();
  vector< ROI* > ROIs(NFRAME, nullptr);
  for(int pid=0; pid<NPIX; pid++){
    /// the data indices are: iFrame*NPIX+pid
    for(int iframe=0; iframe<NFRAME; iframe++){
      float c = data[iframe*NPIX+pid];

      if(c<NCut || c/rmsPed[pid]<ZnCut) continue; /// at least 2 counts and 4 sigma
      if(iframe>0 && data[(iframe-1)*NPIX+pid]+NCut>c) continue; /// previous point should be background only
      if(iframe<NFRAME-1 && data[(iframe+1)*NPIX+pid]/rmsPed[pid]<ZnCut2) continue; /// the next one should also be significantly high
      if(iframe<NFRAME-2 && data[(iframe+2)*NPIX+pid]/rmsPed[pid]<ZnCut3) continue; /// the next one should also be significantly high

      ROI* roi = ROIs[iframe];
      if(!roi){
        roi = new ROI();
        ROIs[iframe] = roi;
        roi->frame = iframe;
       }

      if(!roi->hits) roi->hits = new vector< Hit >();
      roi->hits->emplace_back();
      auto& h1 = roi->hits->back();

      //// now let's get the info of hits
      for(int jframe=iframe; jframe<NFRAME; jframe++){
        float cj = data[jframe*NPIX+pid];
        h1.decay.push_back(cj); 
        if(cj<0.1*c) break;
       }

      h1.pID = pid;
      /// to be improved
      h1.A = c;
      h1.t = iframe;
      h1.width = h1.decay.size();
     }
   }


  //// tests
//   for(int i=0; i<ROIs.size(); i++){
//     ROI* roi = ROIs[i];
//     if(!roi) continue;
// 
//     cout << i << " frame=" << roi->frame << " size=" << roi->hits->size() << endl;
//    }

  vector< ROI >* tm_ROIs = new vector< ROI >();
  vector< Hit >* tm_Hits(nullptr);
  bool saveRoot = false;
  TFile* f1(nullptr);
  TTree* tree1(nullptr);

  //// save them
  if(saveRoot){
    f1 = new TFile("foutA1.root","recreate");
    tree1 = new TTree("physics", "physics data");
    tree1->Branch("RoIs", "std::vector< ROI >", &tm_ROIs);
    tree1->Branch("Hits", "std::vector< Hit >", &tm_Hits);
   }

  /// dump numbers from histograms
  ofstream fout(CF_saveTag+"_out.txt");
  fout << "##-- mode " << CF_mode << " data 72x72";

  //// let's do things frame by frame now
  for(int i=0; i<ROIs.size(); i++){
    ROI* roi = ROIs[i];
    if(!roi) continue;

    /// merge with next frame if applicable
    if(i<ROIs.size()-1 && ROIs[i+1]){
      ROI* roiN = ROIs[i+1];

      int nClose = 0;
      for(auto& jh: *(roi->hits)){
        for(auto& kh: *(roiN->hits)){
          if(jh.pID<0 || kh.pID<0) continue;
          if(distance(jh, kh)<1.5) nClose++;
         }
       }

      ///merge the two if more than 5 hits are close -- further will need to ask them to be in same column?
      if(nClose>5){
        moveHits(roi->hits, roiN->hits);
        roi->status = 1;

        Info("merging frames", "frame %d with nClose=%d", roi->frame, nClose);

        delete roiN;
        ROIs[i+1] = nullptr;
       }
     }

    /// skip frames that is not interesting
    if(roi->hits->size()<10) continue;
    drawFrames(roi->frame, 1, CF_mode);

    tm_Hits = roi->hits;

    tm_ROIs->clear();
    splitROI(roi, tm_ROIs);

    int ir(0);
    for(auto t: *tm_ROIs){
      checkEdgeROI(&t);
      cout << t.status << " " << t.m_hitsI.size() << endl;
      if(t.m_hitsI.size()<10) continue;

      fout << "\n\n# frame " << t.frame << "-" << ir << " status " << t.status << " nHits " << t.m_hitsI.size() << " ADC " << t.ADC();
      vector<float> t_data(72*72,0);
      for(auto i: t.m_hitsI){
        t_data[t.hits->at(i).pID/8] = t.hits->at(i).A;
       }
      for(int i=0; i<72; i++){
        for(int j=0; j<72; j++){
          if(j==0) fout << endl;
          else fout<<" ";
          fout << t_data[i*72+j];
         }
       }
      
//       drawROI(&t);
     }
   }
  fout.close();

  return;
}

void trackFinder::moveHits(vector< Hit >* des, vector< Hit >* source){
  /// move all hits in source to des
  des->reserve(des->size() + source->size());
  des->insert(des->end(), source->begin(), source->end());
  source = nullptr;

  return;
}

void trackFinder::moveHits(ROI* des, int is, ROI* source){
  des->m_hitsI.push_back(is);
  source->m_hitsI[is] = -1;

  int nHits = source->m_hitsI.size();
  for(int i=0; i<nHits; i++){
    if(source->m_hitsI[i]<0) continue;
    if(distance((*(source->hits))[i], (*(source->hits))[is])>2.5) continue;

    moveHits(des, i, source);
   }

  return;
}

void trackFinder::splitROI(ROI* big_roi, vector< ROI >* out){
  /// split the big_roi to small ones are ave in _out
  vector< Hit >& hits_o = *(big_roi->hits);

  /// incase it's not initilizated
  if(big_roi->m_hitsI.size()==0) big_roi->m_hitsI.assign(big_roi->hits->size(),0);

  while(true){
    /// first find the seed
    int seed(-1);
    float maxC(-1);
    for(int j=0; j<hits_o.size(); j++){if(big_roi->m_hitsI[j]>=0 && hits_o[j].A > maxC){seed=j; maxC=hits_o[j].A;}}
    if(seed<0) break;

    out->emplace_back();
    auto& roi_t = out->back();

    roi_t.frame = big_roi->frame;
    roi_t.status = big_roi->status;
    roi_t.hits = big_roi->hits;

    /// collect the nearby hits
    moveHits(&roi_t, seed, big_roi);
   }

  return;
}

void trackFinder::drawROI(const ROI* r) const
{
  TH2F* h2 = new TH2F("h2","h2",72,0,72,72,0,72);
  int ch=0;
  init_keyboard();

  for(auto i: r->m_hitsI){
    auto& h = (*(r->hits))[i];

    int adc1 = h.pID % 8;
    int t = h.pID/8;
    h2->Fill(t%72, t/72, h.A);
    cout << t%72 << " " << t/72 << " " << h.A << endl;
   }

  h2->SetFillColor(4);
  h2->Draw("box");
  gStyle->SetOptStat(0);

  m_lt->DrawLatexNDC(0.2, 0.94, TString::Format("frame %d, status %d", r->frame, r->status));

  gPad->Update();

  ch=readch();

  return;
}

void trackFinder::checkEdgeROI(ROI* r) const
{
  int nEdge = 0;
  for(auto i: r->m_hitsI){
    auto& h = (*(r->hits))[i];

    int adc1 = h.pID%8;
    int t = h.pID/8;
    int x=t%72;
    int y=t/72;
    if(x<2 || x>70 || y<2 || y>70) nEdge++;
   }
  if(nEdge>2) r->status = 2;

  cout << nEdge << endl;

  return;
}


void trackFinder::drawFrames(int fstart, int fN, int mode){
  //// setup pede
  pd1Pede pede;
  pede.setup(CF_inBkg);

  char* fn = &CF_inData[0];
  placData_1 pd1;
  pd1.read(fn);
  pd1.print();

  int nCol = round(sqrt(fN));
  int nRow = fN/nCol;
  if (nCol*nRow < fN) nRow++;

  /// setup plots
  plot pt;
  if(!m_tpad){
    m_tpad = new TPad();
    pt.setupCanvas(nRow,nCol,1200,600); //设置canvas上图片的行数，列数
   }
  pt.setup2D(nRow,nCol,72,72,"hist2d"); //Tcanvas中的图的行数，列数，阵列的行数，列数，图的名字

  int ch=0;
  if(mode>=10){
    init_keyboard();
   }
  int adc(0);

  for(int ic=0; ic<fN; ic++){
    pd1.getFrame(fstart+ic, adc);
    if(mode%10>0) pede.subPede(pd1.frameDat, adc);  //subtract pede 不加第二个参数，会操作所有通道的数据
    if(mode%10>1) pede.getSignificance(pd1.frameDat, adc); 
    pt.loadData2Hist2D(pd1.frameDat,ic);

    char str[20];
    sprintf(str, "frame %d", fstart+ic);
    pt.h2[ic]->SetTitle(str);
   }
  pt.draw2D();
  pt.c->SaveAs(TString::Format("%s_%d.eps",CF_saveTag.c_str(), fstart));
  pt.c->SaveAs(TString::Format("%s_%d.png",CF_saveTag.c_str(), fstart));

  if(mode>=10) ch=readch();

  return;
}

