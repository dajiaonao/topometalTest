#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <TError.h>
#include <TPad.h>
#include <TVector3.h>
#include <TObject.h>
#include "rootlibX.h"
#include "trackFinder.h"

using namespace std;
ClassImp(trackFinder)

void trackFinder::test(){
  std::cout << "in trackFinder" << std::endl;
  Info("trackFinder", "testing %s", "trackFinder");
 }

float trackFinder::distance(Hit* h1, Hit* h2){
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

// void trackFinder::process0(){
//   cout << "Processing file: " << CF_inData << endl;
//   /// setup data
//   char* fn = &CF_inData[0];
//   placData_1 pd1;
//   pd1.read(fn);
//   pd1.print();
// 
//   setupBkg(&pd1);
// 
//   int NCut = 2;
//   int ZnCut = 4;
//   int ZnCut2 = 2;
// 
//   //// get the data vector
//   const int NPIX = pd1.nPix()*pd1.adcCha();
//   cout << NPIX << endl;
//   size_t nData = pd1.dataSize()/sizeof(short);
// 
//   vector< float > data(nData,0);
//   short* ps = (short*)pd1.p;
//   for(int i=0; i<nData; i++) {
//     data[i] = ps[i] - meanPed[i%NPIX];
// //       cout << "id=" << i%NPIX << " frame=" << i/NPIX <<  " data=" << data[i] << " ps=" << ps[i] << " mean=" << meanPed[i%NPIX] << endl;
//    }
// 
//   /// check frame 169
// //     pd1.getFrame(169, 0);
// //     for(int i=0; i<72*72; i++){
// //       pd1.frameDat[i] -= meanPed[8*i];
// //       cout << "ttt==> i=" << i << " (x=" << i%72 << ",y=" << i/72 << ") c=" << pd1.frameDat[i] << " data=" << data[169*NPIX+i*8] << endl; 
// //      }
// 
// 
//   /// start operating on data vector
//   /// -- loop over pixels to find the hits
//   const size_t NFRAME = pd1.nFrame();
//   vector< ROI* > ROIs(NFRAME, nullptr);
//   for(int pid=0; pid<NPIX; pid++){
// //       if(pid%8!=0) continue; /// FIXME: here we only process adc==0. Need to update
// 
//     /// the data indices are: iFrame*NPIX+pid
//     for(int iframe=0; iframe<NFRAME; iframe++){
//       short c = data[iframe*NPIX+pid];
// 
//       if(c<NCut || c/rmsPed[pid]<ZnCut) continue; /// at least 2 counts and 4 sigma
//       if(iframe>0 && data[(iframe-1)*NPIX+pid]+NCut>c) continue; /// previous point should be background only
//       if(iframe<NFRAME-1 && data[(iframe+1)*NPIX+pid]/rmsPed[pid]<ZnCut2) continue; /// the next one should also be significantly high
// 
// //         cout << "pid=" << pid << "(adc=" << pid%8 << ", x=" << (pid%8)%72 << ", y=" << (pid%8)/72 <<  ") frame=" << iframe << " count=" << c << " and Zn=" << c/rmsPed[pid] << endl;
//       ROI* roi = ROIs[iframe];
//       if(!roi){
//         roi = new ROI();
//         ROIs[iframe] = roi;
//         roi->frame = iframe;
//        }
// //         cout << "adding ROI" << endl;
// 
//       Hit* h1 = new Hit();
//       roi->hits.push_back(h1);
// 
//       //// now let's get the info of hits
//       for(int jframe=iframe+1; jframe<NFRAME; jframe++){
//         short cj = data[jframe*NPIX+pid];
//         h1->decay[jframe] = cj; 
//         if(cj<0.5*c) break;
//        }
//       h1->pID = pid;
//       /// to be improved
//       h1->A = c;
//       h1->t = iframe;
//       h1->width = h1->decay.size();
//      }
//    }
// 
// 
//   //// tests
//   for(int i=0; i<ROIs.size(); i++){
//     ROI* roi = ROIs[i];
//     if(!roi) continue;
//     if(roi->hits.size() < 10) continue;
// 
//     cout << i << " frame=" << roi->frame << " size=" << roi->hits.size() << endl;
// 
//     /// showFrame
//     /// showROI
//     /// dump numbers
//    }
// 
//   //// merge ROI
// 
// 
//   return;
// 
// 
//   /// loop the frame
//   for(int i=0;i<1;i++){
//     if(i%20==0) cout<<i<<endl;
//     pd1.getFrame(i);
// 
//     for(int j=0; j<pd1.nFrameDat; j++){
//       int adc = j/(72*72);
//       int pID = j%(72*72);
//       int count = pd1.frameDat[j]; 
// 
//       Info("process", "frame %d, adc=%d pX=%d pY=%d, count=%d", i, adc, pID/72, pID%72, count);
//      }
//    }
// 
// 
//   /// find the seed: the outstanding pixels that is not on the tail of a signal
//   /// find the ROI by combining nearby pixels in a pixel
//   /// Combine the ROI to form a track
//   //
//   return;
// }

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
//     for(int i=0; i<roi->hits.size(); i++) cout << "  ==> " << i << " A" << roi->hits[i]->A << " " << roi->hits[i]->pID << " width=" << roi->hits[i]->width << endl;
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

  /// merge ROIs -- if the leading edge appear in two frames
  for(int i=0; i<ROIs.size()-1; i++){
    ROI* roi = ROIs[i];
    if(!roi) continue;

    ROI* roiN = ROIs[i+1];
    if(!roiN) continue;

    roi->status = 0;
    /// the closest distance is 1

    int nClose = 0;
    for(size_t j=0; j<roi->hits.size(); j++){
      for(size_t k=0; k<roiN->hits.size(); k++){
        if(roi->hits[j] && roiN->hits[k] && distance(roi->hits[j], roiN->hits[k])<1.5) nClose++;
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

  vector< ROI >* tm_ROI = new vector< ROI >();
  for(int i=0; i<ROIs.size(); i++){
    ROI* roi = ROIs[i];
    if(!roi) continue; /// skip frames that does not have any interesting pixels
    if(roi->hits.size()<10) continue;

    tm_ROI->clear();
    splitROI(roi, tm_ROI);

    drawFrames(roi->frame, 6, 10);
   }

  bool saveRoot = false;
  if(saveRoot){
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
//     cout << i << " - write out vector size: " << tm_ROI->size() << endl;

    tree1->Fill();
   }
  f1->Write();
  }

  return;
}

void trackFinder::moveHits(vector< Hit* >& des, vector< Hit* >& source){
  /// move all hits in source to des
  for(int i=0; i<source.size(); i++){
    if(!source[i]) continue;
    des.push_back(source[i]);
    source[i] = nullptr;
   }

  return;
}


void trackFinder::moveHits(vector< Hit* >& des, int is, vector< Hit* >& source){
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
//     Info("moveHits","moving to next: %d", i);
    moveHits(des, i, source);
   }

  return;
}

void trackFinder::splitROI(ROI* big_roi, vector< ROI >* out){
  /// split the big_roi to small ones are ave in _out
  vector< Hit* >& hits_o = big_roi->hits;

  while(true){
    /// first find the seed
    int seed(-1);
    float maxC(-1);
    for(int j=0; j<hits_o.size(); j++){if(hits_o[j] && hits_o[j]->A > maxC){seed=j; maxC=hits_o[j]->A;}}
    if(seed<0) break;

//     cout << "seed " << seed << " A=" << hits_o[seed]->A << endl;
    ROI roi_t;
    roi_t.frame = big_roi->frame;

    /// collect the nearby hits
    moveHits(roi_t.hits, seed, hits_o);
    out->push_back(roi_t);
   }
//   cout << "here: out size = " << out->size() << endl;

  return;
}


vector< ROI* >* trackFinder::splitROI( ROI* big_roi){
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
    pede.subPede(pd1.frameDat, adc);  //subtract pede 不加第二个参数，会操作所有通道的数据
    pede.getSignificance(pd1.frameDat, adc); 
    pt.loadData2Hist2D(pd1.frameDat,ic);

    char str[20];
    sprintf(str, "frame %d", fstart+ic);
    pt.h2[ic]->SetTitle(str);
   }
  pt.draw2D();
  pt.c->SaveAs("signal_example.eps");
  pt.c->SaveAs("signal_example.png");

  if(mode>=10) ch=readch();

  return;
}

