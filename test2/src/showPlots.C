#include "Objects.h"
#include <TChain.h>
#include <iostream>

using namespace std;


int showPlots()
{
  gSystem->Load("recoTest.so");
  cout << "testing" << endl;

  TChain ch("physics");
  ch.Add("foutA1.root");

  cout << "ch size=" << ch.GetEntries() << endl;

  vector< ROI >* x = new vector< ROI >();
  ch.SetBranchAddress("RoIs", &x);

  for(size_t t=0; t<ch.GetEntries(); t++){
    ch.GetEntry(t);
    cout << x->size() << endl;
    for(size_t i=0; i<x->size(); i++){
      cout << i << " frame:" << x->at(i).frame << endl;
     }
   }

//   ch.Show(0);

  return 0;
}
