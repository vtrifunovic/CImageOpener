CC=gcc
CFLAGS= -Werror
LINK= -lGL -lglfw -lm -lOpenCL -lGLEW

BP=binaryproc/
CV=conversions/
EF=effects/
MK=masks/
GPU=opencl_support/
RE=render/
REI=render/render_internal/
TL=tools/

BUILD_DIR=build/

output: mk_build $(BUILD_DIR)gb.o $(BUILD_DIR)bp.o $(BUILD_DIR)cv.o $(BUILD_DIR)ef.o \
	$(BUILD_DIR)mk.o $(BUILD_DIR)gpu.o $(BUILD_DIR)re.o $(BUILD_DIR)rei.o $(BUILD_DIR)tl.o \
	$(BUILD_DIR)flt.o $(BUILD_DIR)main.o clean

mk_build:
	@if [ ! -d $(BUILD_DIR) ]; then\
		mkdir build;\
	fi

$(BUILD_DIR)main.o:
	$(CC) -c main.c $(CFLAGS) $(LINK) -o $@
	$(CC) $(BUILD_DIR)main.o $(BUILD_DIR)gb.o $(BUILD_DIR)bp.o $(BUILD_DIR)cv.o $(BUILD_DIR)ef.o $(BUILD_DIR)mk.o $(BUILD_DIR)gpu.o $(BUILD_DIR)re.o $(BUILD_DIR)rei.o $(BUILD_DIR)tl.o $(BUILD_DIR)flt.o $(CFLAGS) $(LINK)

$(BUILD_DIR)gb.o:
	$(CC) -c global.c $(CFLAGS) -o $@

$(BUILD_DIR)bp.o:
	$(CC) -c $(BP)binaryproc.c $(CFLAGS) -o $@

$(BUILD_DIR)cv.o:
	$(CC) -c $(CV)conversions.c $(CFLAGS) -o $@

$(BUILD_DIR)ef.o:
	$(CC) -c $(EF)effects.c $(CFLAGS) -o $@

$(BUILD_DIR)mk.o:
	$(CC) -c $(MK)masks.c $(CFLAGS) -o $@

$(BUILD_DIR)gpu.o:
	$(CC) -c $(GPU)gpu_setup.c $(CFLAGS) -o $@

$(BUILD_DIR)re.o:
	$(CC) -c $(RE)render.c $(CFLAGS) -o $@

$(BUILD_DIR)rei.o:
	$(CC) -c $(REI)render_internal.c $(CFLAGS) -o $@

$(BUILD_DIR)tl.o: 
	$(CC) -c $(TL)basic_tools.c $(CFLAGS) -o $@

$(BUILD_DIR)flt.o:
	$(CC) -c $(TL)filters.c $(CFLAGS) -o $@

clean:
	rm $(BUILD_DIR)*.o