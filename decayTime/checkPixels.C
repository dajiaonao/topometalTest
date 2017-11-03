#include <iostream>
#include <vector>
#include <algorithm>
#include <TChain.h>
#include <TFile.h>

using namespace std;

const int FRAMEINTERVAL=72*72;

class PixelChecker{
 public:
  PixelChecker():frameInterval(FRAMEINTERVAL),nFrame(808),foutName("fout2.root"){};
  ~PixelChecker(){saveOutTree();}

  int frameInterval;
  int nFrame;
  float Vreset;
  int nBkg;
  int maxN;
  UInt_t chanID;
  int decayStart;
  int decayWidth;
  double* pID;
  double* frames;
  double* counts;
  TFile* fout;
  TTree* tree2;
  TChain* inTree;
  string foutName;

  vector <int> workFrames;

  int checkPixel(int chan, int adc=0){

    for(int i=0; i<nFrame; i++){
      workFrames[i] = counts[chan+i*frameInterval];
      if(int(pID[chan+i*frameInterval])!=chan) {
        cout << "Wrong pixel ID!!!" << " seen " << int(pID[chan+i*frameInterval]) << " and expect " << chan << " in frame " << i << endl;
       }
     }

    /// loop
    // The idea: find the maxima first;
    //  Then find the background using the points before;
    //  Check if there are similar maxima -- and locate the second largest, to avoid using the last one -- and check the background and consistent
    //  Then check the tail.


    /// find the maxima in 50-400
    vector< int >::iterator max1 = max_element(workFrames.begin()+50, workFrames.begin()+400);
    cout << *max1 << " at " << max1-workFrames.begin() << endl;
    maxN = *max1;

    /// get the starting point and background
    //// check the previous points. We would expect to see less then
    int istart = max1-workFrames.begin();
//     while(istart>1 && workFrames[istart]-workFrames[istart-1]<5*(workFrames[istart-1]-workFrames[istart-2])){
    while(istart>1 && abs(workFrames[istart]-workFrames[istart-1])<50){ /// if difference is less than 50
      istart--;
     }
    if(istart != max1-workFrames.begin()){
      cout << "starting point updated: " << max1-workFrames.begin() << "(" << *max1 << ") -> " << istart << "(" << workFrames[istart] << ")" << endl;
     }
    decayStart = istart;

    //// get the background
    int nbkg = workFrames[istart-1];
    cout << "nbkg=" << nbkg << endl;
    nBkg = nbkg;

    /// check 10 points for background statblity
    int nbig = 0;
    for(int i=2; i<10; i++){
//       cout << i << ": " << istart-i << " -> " << workFrames[istart-i] << endl;
      if(workFrames[istart-i]-nbkg>10) nbig++;
    }
    if(nbig>5) cout << "problematic background" << endl;
    /// FIXME: need to check if the background is well calculated

    //// get the width. Stop if the difference is less than 1% of the total diff
    for(vector< int >::iterator it=max1+1; it!=workFrames.end(); ++it){
      if((*max1 - *it) < 0.2*(*max1-nbkg)) continue;
      cout << "debug:" << (*max1 - *it) << " " << (*(it-1) - *it) << endl;
      if((*(it-1) - *it)>2) continue;
//       if((*max1 - *it) < 100*(*(it-1) - *it)) continue;
      cout << *it << " at " << it-workFrames.begin() << endl;
      decayWidth = it-workFrames.begin()-istart;
      break;
     }

    //// save this: the location used to measure, and the width, and the pID

    return 0;
   }

  void setupOutTree(){
    fout = new TFile(foutName.c_str(),"recreate");
    tree2 = new TTree("tree2","decay time");
    tree2->Branch("pID", &chanID, "pID/i");
    tree2->Branch("decayStart", &decayStart, "decayStart/I");
    tree2->Branch("decayWidth", &decayWidth, "decayWidth/I");
    tree2->Branch("maxN", &maxN, "maxN/I");
    tree2->Branch("nBkg", &nBkg, "nBkg/I");
    tree2->Branch("Vreset", &Vreset, "Vreset/f");
   }

  void saveOutTree(){
    tree2->Write();
    fout->Close();

    delete fout;
   }

  int checkAll(){
    /// get the array
    inTree->SetEstimate(frameInterval*nFrame+10);
    long nEvt = inTree->Draw("pID:frame:count","adc==0","goff");
    pID = inTree->GetV1();
    frames = inTree->GetV2();
    counts = inTree->GetV3();

    nFrame = nEvt/(72*72);

    workFrames.resize(nFrame);

    for(int i=0; i<72*72; i++){
      cout << "------- " << i << " ---------" << endl;
      checkPixel(i);
      cout << i << " " << decayStart << " " << decayWidth << endl;
      chanID = i;

      tree2->Fill();
     }

    return 0;
   }
};

int checkPixels(){
  PixelChecker pc1;
  pc1.setupOutTree();

  TChain ch1("tree1");
  ch1.Add("/home/dzhang/work/topmetal2M/Samples/oct20_scan/Vreset1th/out153.root");
  pc1.inTree = &ch1;
  pc1.checkAll();

  return 0;
//   return pc1.checkPixel(3);
}

int main(){

  return checkPixels();
}
