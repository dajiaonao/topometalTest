ROOTLIBS := $(shell root-config --glibs)
ROOTFLAG := $(shell root-config --cflags)
CXXFLAGS := -I/data/repos/xRayPol/TM1_8 $(ROOTFLAG)

CXX:=gcc -g
LD:=g++

OBJ=simpleTest.o trackFinder.o 
HEADS=simpleTest.h trackFinder.h Objects.h

lib: libTest.so

main: main.exe
	echo "Done"

%.exe: %.o $(OBJ) testDict.o
	$(LD) $(ROOTLIBS) $^ -o $@

libTest.so: $(OBJ) testDict.o
	$(LD) $(ROOTLIBS) $^ -shared -o $@

%.o: %.cpp Objects.h
	$(CXX) -fPIC $(CXXFLAGS) -c $< -o $@

%Dict.cpp: $(HEADS) Linkdef.h
	rootcint -f $@ -c $(CXXFLAGS) -p $^

clean:
	-rm *.o
cleanall: clean
	-rm *.exe
	-rm *.so
	-rm testDict*

