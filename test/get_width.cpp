#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TString.h>
#include <iostream>
#include <fstream>

using namespace std;

class SignalFinder{
 public:
  SignalFinder(){set_up();}
  ~SignalFinder(){treeOut->Write();fout->Close();}

  void set_up(){
    fout = new TFile("fout1.root","recreate");
    treeOut = new TTree("tree2", "decay time width");
    treeOut->Branch("adc", &adc, "adc/i");
    treeOut->Branch("pID", &pID, "pID/i");
    treeOut->Branch("frameStart", &frameStart, "frameStart/I");
    treeOut->Branch("frameWidth", &frameWidth, "frameWidth/I");

    cout << "created!!!" << endl;
    return;
   }

  void getNoiseInfo(){
    const int nPixelsOnChip = 72*72;
    const int NCHAN = nPixelsOnChip*8;
    pedeV = new vector< float >(NCHAN,0);
    sigmaV = new vector< float >(NCHAN,0);

    string a("../../data/pedeData/pede8.txt");
    /// read info from file
    std::ifstream inPD(a.c_str());
    if(!inPD.is_open()) {
      cout << "No pedestal table found!" << endl;
      return ;
     }
    
    int iChipT = 0, iPixelT = 0, iCounter=0;
    float pedestalT = 0., noiseT = 0.;
    while(!inPD.eof() && iCounter<NCHAN) {
      inPD >> iChipT >> iPixelT >> pedestalT >> noiseT;
      (*pedeV)[iChipT*nPixelsOnChip+iPixelT] = pedestalT;
      (*sigmaV)[iChipT*nPixelsOnChip+iPixelT] = noiseT;
      iCounter++;
     }		
    cout<<iCounter<<" items is read"<<endl;
   }

  int scan_all(){
    adc = 0;
    TEntryList* el1 = treeIn->GetEntryList();
    TString elist_t = TString::Format("elist_adc%d", adc);
    treeIn->Draw(">>"+elist_t,TString::Format("adc==%d", adc),"entrylist");
    TEntryList* el2 = (TEntryList*)gDirectory->Get(elist_t);
    treeIn->SetEntryList(el2);

    /// loop
    for(int i=0; i<72*72; i++){
      pID = i;
      find_signals();
     }

    treeIn->SetEntryList(el1); 

    return 0;
   }


  int find_signals(){
    long n = treeIn->Draw("frame:count",TString::Format("adc==%d&&pID==%d", adc, pID),"goff");
    double* vframe = treeIn->GetV1();
    double* vcount = treeIn->GetV2();

    bool inSignal(false);
    float pede = (*pedeV)[adc*72*72+pID];
    float sigma = (*sigmaV)[adc*72*72+pID];
    float rMax = -1;

    int nS = 0;
    for(int i=0;i<n;i++){
//       cout << vframe[i] << " " << vcount[i] << endl;
      float r = vcount[i] - pede;
      float Zn = r/sigma;
//       cout << "r=" << r << " Zn=" << Zn << endl;

      if(!inSignal){
        if(Zn<4) continue; /// at least 4 sigma

        if(r>rMax) rMax = r;
        else if(r<0.3*rMax) continue; /// at least 0.3 of the expected magnitude, to ignore the second small peak

        inSignal = true;
        frameStart = vframe[i];
       }else{
         if(r>rMax) rMax = r;

         if(Zn>1.) continue;
         inSignal = false;
         frameWidth = vframe[i]-frameStart;

         treeOut->Fill();
         nS++;
         cout << "signal:" << frameStart << " " << frameWidth << endl;

         /// reset
         frameStart = -1;
         frameWidth = -1;
       }
     }
    cout << "pID=" << pID << ": " << nS << " found. Pede=" << pede << "+/-" << sigma << endl;

    return 0;
  }

  TFile* fout;
  TTree* treeOut;
  TChain* treeIn;

  UInt_t adc;
  UInt_t pID;
  Int_t  frameStart;
  Int_t  frameWidth;

  vector< float >* pedeV;
  vector< float >* sigmaV;
};

int get_width(){
  TChain ch1("tree1");
  ch1.Add("test1.root");
  ch1.Show(0);
  ch1.SetEstimate(ch1.GetEntries());


  SignalFinder sf1;
  sf1.treeIn = &ch1;
  sf1.getNoiseInfo();
//   sf1.adc = 0;
//   sf1.pID = 100;
//   sf1.find_signals();
  sf1.scan_all();

  return 0;
}

int main(){
  return get_width();
}
