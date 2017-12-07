#ifndef TEST1_H
#define TEST1_H
#include <TObject.h>

class simpleTest:public TObject{
 public:
  simpleTest(){};
  void test();

  ClassDef(simpleTest,1)  // The Monte Carlo Event
};

#endif //TEST1_H
