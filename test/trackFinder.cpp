#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <TError.h>
#include <TVector3.h>
#include "rootlibX.h"
#include "Objects.h"

using namespace std;

class trackFinder{
 public:
  trackFinder():meanPed(nullptr),rmsPed(nullptr){};

  void test(){
    cout << "in trackFinder" << endl;
    Info("trackFinder", "testing %s", "trackFinder");
   }

  float distance(Hit* h1, Hit* h2){
    if(!h1 || !h2){
      Error("distance","h1 or h2 is null....");
      return 999;
     }
    int adc1 = h1->pID % 8;
    int adc2 = h2->pID % 8;
    int dx = (h1->pID/8)%72-(h2->pID/8)%72;
    int dy = (h1->pID/8)/72-(h2->pID/8)/72;

    if (adc1!=adc2) return 999;

    return sqrt(dx*dx+dy*dy);
   }

  void setupBkg(placData_1& pd1){
    std::ifstream inPD(CF_inBkg.c_str());
    if(!inPD.is_open()) {
      cout << "No pedestal table found!" << endl;
      return ;
     }
  
    const int NADC = pd1.adcCha();
    const int NChan = NADC*pd1.nPix();
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

  void findHits(){
    /// A hit should: previous frame much less hits; much more hits in this frame
   }

  void process(){
    CF_inData = "/home/dzhang/work/topmetal2M/Samples/Nov03_Guangxi/out386.pd1";
    CF_inBkg = "/home/dzhang/work/topmetal2M/Samples/Nov03_Guangxi/pede22.txt";
    /// setup data
    char* fn = &CF_inData[0];
    placData_1 pd1;
    pd1.read(fn);
    pd1.print();

    setupBkg(pd1);

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
        short c = data[iframe*NPIX+pid];

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

        Hit* h1 = new Hit();
        roi->hits.push_back(h1);

        //// now let's get the info of hits
        for(int jframe=iframe; jframe<NFRAME; jframe++){
          short cj = data[jframe*NPIX+pid];
          h1->decay.push_back(cj); 
          if(cj<0.1*c) break;
         }

        h1->pID = pid;
        /// to be improved
        h1->A = c;
        h1->t = iframe;
        h1->width = h1->decay.size();
       }
     }


    //// tests
    for(int i=0; i<ROIs.size(); i++){
      ROI* roi = ROIs[i];
      if(!roi) continue;

      cout << i << " frame=" << roi->frame << " size=" << roi->hits.size() << endl;
      for(int i=0; i<roi->hits.size(); i++) cout << "  ==> " << i << " A" << roi->hits[i]->A << " " << roi->hits[i]->pID << " width=" << roi->hits[i]->width << endl;
     }

    //// merge & split ROI in each frame
//     vector< vector< ROI* >* > roiV(NFRAME, nullptr); /// each frame has a vector

    /// for each frame, merge the pixels to form a real ROI
//     for(int i=0; i<ROIs.size(); i++){
//       ROI* roi = ROIs[i];
//       if(!roi) continue; /// skip frames that does not have any interesting pixels
// 
//       roiV[i] = splitROI(roi);
// 
//       Info("process", "check splitted ROIs in frame %d", i);
//       for(int j=0; j<roiV[i]->size(); j++){
//         Info("process", "check splitted ROI %d", j);
//         Info("process", "%d hits found", roiV[i]->at(j)->hits.size());
//        }
//      }

    //// save them
    TFile* f1 = new TFile("foutA1.root","recreate");
    TTree* tree1 = new TTree("physics", "physics data");

    vector< ROI >* tm_ROI = new vector< ROI >();
    tree1->Branch("RoIs", "std::vector< ROI >", &tm_ROI);

    for(int i=0; i<ROIs.size(); i++){
      ROI* roi = ROIs[i];
      if(!roi) continue; /// skip frames that does not have any interesting pixels

      tm_ROI->clear();
      splitROI(roi, tm_ROI);
      cout << i << " - write out vector size: " << tm_ROI->size() << endl;

      tree1->Fill();
     }
    f1->Write();
   

    return;
  }

  void moveHits(vector< Hit* >& des, int is, vector< Hit* >& source){
    /// move the neibours of i in source to des
    Hit* temp = source[is];
    if(!temp){
      Error("moveHits","seed is None?");
     }
    des.push_back(temp);
    source[is] = nullptr;

    for(int i=0; i<source.size(); i++){
      if(!source[i]) continue;
      if(distance(source[i], temp)>1.1) continue;
      Info("moveHits","moving to next: %d", i);
      moveHits(des, i, source);
     }

    return;
   }

  void splitROI(ROI* big_roi, vector< ROI >* out){
    /// split the big_roi to small ones are ave in _out
    vector< Hit* >& hits_o = big_roi->hits;

    while(true){
      /// first find the seed
      int seed(-1);
      float maxC(-1);
      for(int j=0; j<hits_o.size(); j++){if(hits_o[j] && hits_o[j]->A > maxC){seed=j; maxC=hits_o[j]->A;}}
      if(seed<0) break;

      cout << "seed " << seed << " A=" << hits_o[seed]->A << endl;
      ROI roi_t;
      roi_t.frame = big_roi->frame;

      /// collect the nearby hits
      moveHits(roi_t.hits, seed, hits_o);
      out->push_back(roi_t);
     }
    cout << "here: out size = " << out->size() << endl;

    return;
   }


  vector< ROI* >* splitROI( ROI* big_roi){
    /// if it's not empty
    vector< ROI* >* newV = new vector< ROI* >();
    vector< Hit* >& hits_o = big_roi->hits;

    while(true){
      /// first find the seed
      int seed(-1);
      float maxC(-1);
      for(int j=0; j<hits_o.size(); j++){if(hits_o[j] && hits_o[j]->A > maxC){seed=j; maxC=hits_o[j]->A;}}
      if(seed<0) break;

      cout << "seed " << seed << " A=" << hits_o[seed]->A << endl;
      ROI* roi_t = new ROI();
      roi_t->frame = big_roi->frame;

      /// collect the nearby hits
      moveHits(roi_t->hits, seed, hits_o);
      newV->push_back(roi_t);
     }

    return newV;
   }

  string CF_inData;
  string CF_inBkg;
  float* meanPed;
  float* rmsPed;
};

int recoTest(){
  /// create an instance
  trackFinder tf1;
  /// configuration

  /// run the test
  tf1.test();
  tf1.process();

  return 0;
}


int main(){
  cout << "testing" << endl;
  recoTest();
  return 0;
}
