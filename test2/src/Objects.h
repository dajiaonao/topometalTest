#ifndef Objects_H
#define Objects_H

#include <TObject.h>
#include <vector>
#include <iostream>

using namespace std;

class Hit: public TObject{
 public:
  Hit(){};
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
  ROI(ROI const& r):TObject(r){
    this->frame = r.frame;
    this->status = r.status;
    this->m_hitsI.assign(r.m_hitsI.begin(), r.m_hitsI.end());
    this->hits = r.hits;
   }
  ~ROI(){};

  int frame;
  int status = 0;
  vector< Hit >* hits=nullptr; //! /// a list of hists

  vector< int > m_hitsI;
 private:

  ClassDef(ROI, 1)
};

#endif
