
ROOTLIBS := $(shell root-config --glibs)
ROOTFLAG := $(shell root-config --cflags)
CXXFLAG := -I/data/repos/xRayPol/TM1_8 $(ROOTFLAG)

test: run_recoTest.exe
	./run_recoTest.exe	

run_pd1Draw2DX0.exe: pd1Draw2DX0.cpp
run_pd1ToRoot.exe: pd1ToRoot.cpp
run_get_width.exe: get_width.cpp

run_%.exe: %.cpp
	g++ -o $@ $^ $(CXXFLAG) $(ROOTLIBS)

clean:
	rm run_*.exe
