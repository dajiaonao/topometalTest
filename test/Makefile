ROOTLIBS := $(shell root-config --glibs)
ROOTFLAG := $(shell root-config --cflags)
CXXFLAG := -I/home/dzhang/work/topmetal2M/TM1_8 $(ROOTFLAG)

test2: tf_test.exe

test1: run_recoTest.exe
	./run_recoTest.exe	

run_pd1Draw2DX0.exe: pd1Draw2DX0.cpp
run_pd1ToRoot.exe: pd1ToRoot.cpp
run_get_width.exe: get_width.cpp

run_%.exe: %.cpp
	g++ -o $@ $^ $(CXXFLAG) $(ROOTLIBS)



testC: testC.cpp Objects.h ObjectsDict.o
	g++ -o $@ $^ $(CXXFLAG) $(ROOTLIBS)

tf_test.exe: trackFinder.o Objects.h ObjectsDict.o
	g++ -o $@ $^ $(CXXFLAG) $(ROOTLIBS)

%.o: %.cpp
	gcc -c $^ -o $@ $(CXXFLAG)

ObjectsDict.cpp: Objects.h LinkDef.h
	rootcint -f $@ -c -p $^


clean:
	-rm run_*.exe
	-rm *.o
