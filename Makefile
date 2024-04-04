.SUFFIXES: 

default: 

PRIVATE_FILES=Assignment.key.ipynb admin .git  Assignment-equinix.key.ipynb

STUDENT_EDITABLE_FILES=hello_world2.cpp

.PHONY: create-labs

COMPILER=$(CXX) 
MICROBENCH_OPTIMIZE= -DHAVE_LINUX_PERF_EVENT_H -I$(PWD) 
LIBS= -lm -pthread 
BUILD=./build/

$(BUILD)perfstats.o: perfstats.c perfstats.h
	mkdir -p $(BUILD) 
	cp  $< $(BUILD)$<
	$(COMPILER) $(MICROBENCH_OPTIMIZE) $(LIBS) -o $(BUILD)perfstats.o -c $(BUILD)perfstats.c

$(BUILD)microbench_cuda.o: microbench_cuda.cu
	mkdir -p $(BUILD) 
	cp  $< $(BUILD)$<
	/usr/local/cuda/bin/nvcc --disable-warnings $(MICROBENCH_OPTIMIZE) -L/usr/local/cuda/lib64 -DCUDA -lcuda -lcudart -arch=sm_75 -o $(BUILD)microbench_cuda.o -c $(BUILD)microbench_cuda.cu

#$(BUILD)join_count.o: Makefile config.make

include $(PWD)/config.make

join_count.exe: $(BUILD)perfstats.o $(BUILD)join_count.o $(BUILD)join_count_main.o $(BUILD)join_count_baseline.o 
	$(COMPILER) $(OPTIMIZE) $(MICROBENCH_OPTIMIZE) $(LIBS) $(BUILD)perfstats.o $(BUILD)join_count.o $(BUILD)join_count_main.o $(BUILD)join_count_baseline.o -o join_count.exe

$(BUILD)join_count.o: OPTIMIZE=$(QUERY_OPTIMIZE)
		
$(BUILD)%.o: %.cpp
	mkdir -p $(BUILD) 
	cp  $< $(BUILD)$<
	$(COMPILER) $(OPTIMIZE) $(MICROBENCH_OPTIMIZE) $(LIBS) -c -o $(BUILD)$*.o $(BUILD)$*.cpp


microbench.exe: $(BUILD)perfstats.o microbench.cpp
	$(COMPILER) $(MICROBENCH_OPTIMIZE) $(LIBS) microbench.cpp $(BUILD)perfstats.o  -o microbench.exe

microbench_cuda.exe: $(BUILD)perfstats.o $(BUILD)microbench_cuda.o microbench.cpp
	/usr/local/cuda/bin/nvcc --disable-warnings -I/usr/local/cuda/include -L/usr/local/cuda/lib64 -DCUDA -lcuda -lcudart $(MICROBENCH_OPTIMIZE) microbench.cpp $(BUILD)perfstats.o $(BUILD)microbench_cuda.o  -o microbench_cuda.exe

%.exe: $(BUILD)%.o
	$(COMPILER) $(MICROBENCH_OPTIMIZE) $(LIBS) $< -o $*.exe

.PHONY: autograde

autograde: hello_world2.exe
	./hello_world2.exe

bitcount.exe: $(BUILD)bitcount.o

clean: 
	rm -f *.exe $(BUILD)*
#test
#test2
