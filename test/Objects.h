#ifndef Objects_H
#define Objects_H

#include <TObject.h>
#include <vector>
#include <iostream>

using namespace std;

class Hit: public TObject{
 public:
  int pID;
  float t; /// time
  float A; /// amplitude
  float width;
  vector< int > decay; /// <frame, count>

  ClassDef(Hit, 1)
};

class ROI: public TObject{
 public:
  ROI(){};
  ROI(ROI const& r):TObject(r){this->frame = r.frame;}
  ~ROI(){};

  int frame;
  vector< Hit* > hits; //! /// a list of hists

 private:
  vector< int > m_hitsI;

  ClassDef(ROI, 1)
};

#endif
