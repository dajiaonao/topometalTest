#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TROOT.h>
#include <TMath.h>
#include <TParameter.h>
#include <TGraph.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TSystem.h>
#include "rootlibX.h"
#include <string>
// #include <io.h>

using std::string;

struct SIG{
  UInt_t adc;
  UInt_t pID;
  UInt_t frame;
  Float_t count;
};
const std::string SIG_s = "adc/i:pID/i:frame/i:count/F";

int doConvert(char* fn){
  cout << "Converting " << fn << endl;

   /// setup data
  placData_1 pd1;
  pd1.read(fn);
  pd1.print();

  TString outname(fn);
  outname.ReplaceAll(".pd1",".root");

  SIG sig;
  TFile* f1 = new TFile(outname,"recreate");
  TTree* t1 = new TTree("tree1","topmetal2M data");
  t1->Branch("sig", &sig, SIG_s.c_str());

  /// loop over data to fill TTree 
  for(int i=0;i<pd1.nFrame();i++){
    if(i%20==0) cout<<i<<endl;
    pd1.getFrame(i);

    sig.frame = i;
    for(int j=0; j<pd1.nFrameDat; j++){
      sig.adc = j/(72*72);
      sig.pID = j%(72*72);
      sig.count = pd1.frameDat[j]; 

      t1->Fill();
     }
   }
  t1->Write();
  f1->Close();

  cout<<"end"<<endl;

};

int main(int argc, char **argv){
  
//   int pedeSet = atol(argv[1]);
//   int dataSet = atol(argv[2]);
  for(int i=1; i<argc; i++) doConvert(argv[i]);

  return 0;
}
