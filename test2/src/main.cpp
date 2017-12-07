#include <iostream>
#include "simpleTest.h"
#include "trackFinder.h"

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
  simpleTest st1;
  st1.test();

  std::cout << "testing" << std::endl;
  recoTest();
  return 0;
}
