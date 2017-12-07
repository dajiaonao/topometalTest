#ifndef trackFinder_H
#define trackFinder_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <TError.h>
#include <TVector3.h>
#include <TObject.h>
#include "Objects.h"
// #include "rootlibX.h"

class placData_1;
class TPad;
// struct Hit{
//   int pID;
//   float t; /// time
//   float A; /// amplitude
//   float width;
//   std::map<int, int> decay; /// <frame, count>
// };
// 
// struct ROI{
//   int frame;
//   std::vector< Hit* > hits; /// a list of hists
// };
// 
// struct Track{
//   TVector3 p3;
//   std::map< float, ROI > Rois; /// a list of ROIs <time, ROI>
// };

class trackFinder:public TObject{
 public:
  trackFinder():meanPed(nullptr),rmsPed(nullptr){};

  void test();
  float distance(Hit* h1, Hit* h2);
  void setupBkg(placData_1* pd1);
  void findHits(){};
  void process0(){};
  void process();
  void moveHits(vector< Hit* >& des, vector< Hit* >& source);
  void moveHits(vector< Hit* >& des, int is, vector< Hit* >& source);
  void splitROI(ROI* big_roi, vector< ROI >* out);
  vector< ROI* >* splitROI( ROI* big_roi);
  void saveFrame(){};
  void drawFrames(int fstart, int fN, int mode=1);

  std::string CF_inBkg = "/data/Samples/xRayPol/topmetal1202/pede27.txt";
  std::string CF_inData = "/data/Samples/xRayPol/20171107/out136.pd1";

//   std::string CF_inData;
//   std::string CF_inBkg;
  float* meanPed;
  float* rmsPed;
  TPad* m_tpad=nullptr;

  ClassDef(trackFinder, 1)
};

#endif //trackFinder
