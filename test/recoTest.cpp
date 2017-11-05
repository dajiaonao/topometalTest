#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <TError.h>
#include <TVector3.h>
#include "rootlibX.h"

using namespace std;

struct Hit{
  int pID;
  float t; /// time
  float A; /// amplitude
  float width;
  map<int, int> decay; /// <frame, count>
};

struct ROI{
  int frame;
  vector< Hit* > hits; /// a list of hists
};

struct Track{
  TVector3 p3;
  map< float, ROI > Rois; /// a list of ROIs <time, ROI>
};

class trackFinder{
 public:
  trackFinder():meanPed(nullptr),rmsPed(nullptr){};

  void test(){
    cout << "in trackFinder" << endl;
    Info("trackFinder", "testing %s", "trackFinder");
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

    //// get the data vector
    const int NPIX = pd1.nPix()*pd1.adcCha();
    cout << NPIX << endl;
    size_t nData = pd1.dataSize()/sizeof(short);

    vector< float > data(nData,0);
    short* ps = (short*)pd1.p;
    for(int i=0; i<nData; i++) {
      data[i] = ps[i] - meanPed[i%NPIX];
//       cout << "id=" << i%NPIX << " frame=" << i/NPIX <<  " data=" << data[i] << " ps=" << ps[i] << " mean=" << meanPed[i%NPIX] << endl;
     }

    /// check frame 169
//     pd1.getFrame(169, 0);
//     for(int i=0; i<72*72; i++){
//       pd1.frameDat[i] -= meanPed[8*i];
//       cout << "ttt==> i=" << i << " (x=" << i%72 << ",y=" << i/72 << ") c=" << pd1.frameDat[i] << " data=" << data[169*NPIX+i*8] << endl; 
//      }


    /// start operating on data vector
    /// -- loop over pixels to find the hits
    const size_t NFRAME = pd1.nFrame();
    vector< ROI* > ROIs(NFRAME, nullptr);
    for(int pid=0; pid<NPIX; pid++){
//       if(pid%8!=0) continue; /// FIXME: here we only process adc==0. Need to update

      /// the data indices are: iFrame*NPIX+pid
      for(int iframe=0; iframe<NFRAME; iframe++){
        short c = data[iframe*NPIX+pid];

        if(c<NCut || c/rmsPed[pid]<ZnCut) continue; /// at least 2 counts and 4 sigma
        if(iframe>0 && data[(iframe-1)*NPIX+pid]+NCut>c) continue; /// previous point should be background only
        if(iframe<NFRAME-1 && data[(iframe+1)*NPIX+pid]/rmsPed[pid]<ZnCut2) continue; /// the next one should also be significantly high

//         cout << "pid=" << pid << "(adc=" << pid%8 << ", x=" << (pid%8)%72 << ", y=" << (pid%8)/72 <<  ") frame=" << iframe << " count=" << c << " and Zn=" << c/rmsPed[pid] << endl;
        ROI* roi = ROIs[iframe];
        if(!roi){
          roi = new ROI();
          ROIs[iframe] = roi;
          roi->frame = iframe;
         }
//         cout << "adding ROI" << endl;

        Hit* h1 = new Hit();
        roi->hits.push_back(h1);

        //// now let's get the info of hits
        for(int jframe=iframe+1; jframe<NFRAME; jframe++){
          short cj = data[jframe*NPIX+pid];
          h1->decay[jframe] = cj; 
          if(cj<0.5*c) break;
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

      cout << i << " frame=" << roi->frame << " size=" << roi->hits.size() << endl;;
     }

    //// merge ROI


    return;


    /// loop the frame
//     for(int i=0;i<pd1.nFrame();i++){
    for(int i=0;i<1;i++){
      if(i%20==0) cout<<i<<endl;
      pd1.getFrame(i);
//       pede.subPede(pd1.frameDat);

      for(int j=0; j<pd1.nFrameDat; j++){
        int adc = j/(72*72);
        int pID = j%(72*72);
        int count = pd1.frameDat[j]; 

        Info("process", "frame %d, adc=%d pX=%d pY=%d, count=%d", i, adc, pID/72, pID%72, count);
       }
     }


    /// find the seed: the outstanding pixels that is not on the tail of a signal
    /// find the ROI by combining nearby pixels in a pixel
    /// Combine the ROI to form a track
    //
    return;
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
