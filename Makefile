CXXFLAGS = -lmali
CXX = aarch64-linux-gnu-g++
INC=-I$(CURDIR)/Include/ -I/home/freli01/ddk-internal/clean_float/product/thirdparty/usr/include/libdrm -I/home/freli01/ddk-internal/clean_float/product/include/khronos/arm/winsys_dummy -I/home/freli01/ddk-internal/clean_float/product/include/khronos/original/ -L/home/freli01/ddk-internal/clean_float/product/_G710_Juno_r32/install/lib -I/home/freli01/ddk-internal/clean_float/product/_G710_Juno_r32/install/lib -I/home/freli01/ddk-internal/clean_float/product/include
objects = mali_demo.cpp
exe = mali_demo
#.PHONY: $(exe)

$(exe):$(objects)
	$(CXX) $(INC)  $(CXXFLAGS)  -o $@ $^

clean:
	$(RM) $(exe)
	$(RM) *.o
