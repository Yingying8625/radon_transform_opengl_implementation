CXXFLAGS = -lmali
CXX = aarch64-linux-gnu-g++
INC=-I$(CURDIR)/Include/ -I$(CURDIR)/Include/libdrm -I$(CURDIR)/Include/winsys_dummy -I$(CURDIR)/Include/khronos -L$(CURDIR)/Libs/mali_ddk/
objects = mali_demo.cpp
exe = mali_demo
#.PHONY: $(exe)

$(exe):$(objects)
	$(CXX) $(INC)  $(CXXFLAGS)  -o $@ $^

clean:
	$(RM) $(exe)
	$(RM) *.o
