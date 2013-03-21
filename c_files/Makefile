CC = gcc
CXX = g++
NVCC = nvcc

CFLAGS = -Wall -Wno-format -Wno-sign-compare -Wno-unused-result -O3 -g
CPPFLAGS = -Wall -Wno-format -Wno-sign-compare -Wno-unused-result -O3 -g -Wno-c++0x-compat

LIBS = lapack ptf77blas ptcblas atlas pthread gfortran cublas gthread-2.0
LIBS := $(addprefix -l, $(LIBS))

LIB_DIRS = /usr/local/atlas/lib /usr/local/cuda/lib64
LIB_DIRS := $(addprefix -L, $(LIB_DIRS))

DEFINES = MULTITHREAD_CONTRAST NUM_THREADS=8 USE_SINGLE ENABLE_GPU
DEFINES := $(addprefix -D, $(DEFINES))

INCLUDE = include
INCLUDE := $(addprefix -I, $(INCLUDE))

PKG_CONFIG = `pkg-config --cflags --libs gtk+-2.0`

################################################################################
################################################################################
# Source files.

# C source files.
C_SRC = src/ica/fastica/contrast.c \
        src/ica/fastica/fastica.c \
        src/ica/jade/jade.c \
        src/ica/aux.c \
        src/xltek/xltek.c \
        src/xltek/erd/s9.c \
        src/xltek/erd/headbox_types.c \
        src/xltek/erd/channel_labels.c \
        src/xltek/erd/s8.c \
        src/xltek/erd/s6.c \
        src/xltek/erd/s5.c \
        src/xltek/erd/s7.c \
        src/xltek/erd/erd.c \
        src/xltek/guids.c \
        src/xltek/edf.c \
        src/main/test/wavelet.c \
        src/main/test/blink_detect.c \
        src/main/test/convolution.c \
        src/main/ede.c \
        src/test/wavelets.c \
        src/test/convolution.c \
        src/gui/chan_plot.c \
        src/gui/blink_control.c \
        src/gui/ede_model.c \
        src/gui/ede_window.c \
        src/gui/eeg_plot.c \
        src/wavelet/wavelets.c \
        src/wavelet/wavelet_filters.c \
        src/blink/detect.c \
        src/blink/aux.c \
        src/convolution.c \
        src/cmd_args/blink_detect.c \
        src/cmd_args/ica.c \
        src/cmd_args/runtime.c \
        src/cmd_args/blink_source.c \
        src/cmd_args/blink_remove.c \
        src/matrix.c

# C++ source files.
CPP_SRC = src/ica/ica.cpp \
          src/ica/ica_thread.cpp \
          src/main/runtime.cpp \
          src/main/test/ica.cpp \
          src/main/test/blink_remove.cpp \
          src/main/test/blink_source.cpp \
          src/blink/remove.cpp

# CUDA source files.
CU_SRC = src/ica/fastica/cuda/fastica.cu \
         src/ica/fastica/cuda/kernels.cu \
         src/ica/fastica/cuda/contrast.cu \
         src/ica/jade/cuda/jade.cu \
         src/ica/jade/cuda/kernels.cu

################################################################################
################################################################################
# Object files (source files with "src/" replaced by "objs/" and the file
# extension replaced with ".o".
C_OBJS = $(subst src/, objs/, $(C_SRC:.c=.o))
CPP_OBJS = $(subst src/, objs/, $(CPP_SRC:.cpp=.o))
CU_OBJS = $(subst src/, objs/, $(CU_SRC:.cu=.o))

################################################################################
################################################################################
# Object files for specific libraries.

# Object files for the ICA library.
ICA_OBJS = objs/ica/ica.o \
           objs/ica/aux.o \
           objs/ica/ica_thread.o \
           objs/ica/fastica/fastica.o \
           objs/ica/fastica/contrast.o \
           objs/ica/jade/jade.o \
           objs/ica/fastica/cuda/fastica.o \
           objs/ica/fastica/cuda/contrast.o \
           objs/ica/fastica/cuda/kernels.o \
           objs/ica/jade/cuda/jade.o \
           objs/ica/jade/cuda/kernels.o

# Object files for the Wavelet library.
WAVELET_OBJS = objs/wavelet/wavelets.o objs/wavelet/wavelet_filters.o

# Object files for the convolution library.
CONVOLUTION_OBJS = objs/convolution.o

# Object files for the matrix library.
MATRIX_OBJS = objs/matrix.o

# Object files for the xltek library.
XLTEK_OBJS = objs/xltek/xltek.o \
             objs/xltek/erd/s9.o \
             objs/xltek/erd/headbox_types.o \
             objs/xltek/erd/channel_labels.o \
             objs/xltek/erd/s8.o \
             objs/xltek/erd/s6.o \
             objs/xltek/erd/s5.o \
             objs/xltek/erd/s7.o \
             objs/xltek/erd/erd.o \
             objs/xltek/guids.o \
             objs/xltek/edf.o

# Object files for the blink library (excludes needed ICA library).
BLINK_OBJS = objs/blink/detect.o \
             objs/blink/remove.o \
             objs/blink/aux.o

# Object files for the EDE GUI.
EDE_GUI_OBJS = objs/gui/ede_model.o \
               objs/gui/chan_plot.o \
               objs/gui/eeg_plot.o \
               objs/gui/blink_control.o \
               objs/gui/ede_window.o

################################################################################
################################################################################
# Program specific object files.

# Convolution test program.
BINS := bin/conv_test $(BINS)
CONV_TEST_OBJS = objs/main/test/convolution.o \
                 objs/test/convolution.o \
                 $(CONVOLUTION_OBJS)

# Wavelet test program.
BINS := bin/wavelet_test $(BINS)
WAVELET_TEST_OBJS = objs/main/test/wavelet.o \
                    objs/test/wavelets.o \
                    $(WAVELET_OBJS) \
                    $(CONVOLUTION_OBJS)

# ICA test program.
BINS := bin/ica_test $(BINS)
ICA_TEST_OBJS = objs/main/test/ica.o \
                objs/cmd_args/ica.o \
                $(ICA_OBJS) \
                $(MATRIX_OBJS)

# Blink detection test program.
BINS := bin/bd_test $(BINS)
BD_TEST_OBJS = objs/main/test/blink_detect.o \
               objs/cmd_args/blink_detect.o \
               $(ICA_OBJS) \
               $(BLINK_OBJS)\
               $(XLTEK_OBJS) \
               $(MATRIX_OBJS) \
               $(WAVELET_OBJS) \
               $(CONVOLUTION_OBJS)

# Blink source detection test program.
BINS := bin/src_test $(BINS)
SRC_TEST_OBJS = objs/main/test/blink_source.o \
                objs/cmd_args/blink_source.o \
                $(ICA_OBJS) \
                $(BLINK_OBJS)\
                $(XLTEK_OBJS) \
                $(MATRIX_OBJS) \
                $(WAVELET_OBJS) \
                $(CONVOLUTION_OBJS)

# Blink removal test program.
BINS := bin/br_test $(BINS)
BR_TEST_OBJS = objs/main/test/blink_remove.o \
               objs/cmd_args/blink_remove.o \
               $(ICA_OBJS) \
               $(XLTEK_OBJS)\
               $(BLINK_OBJS) \
               $(MATRIX_OBJS) \
               $(WAVELET_OBJS) \
               $(CONVOLUTION_OBJS)

# Eyeblink Detector and Eliminator test program.
BINS := bin/ede $(BINS)
EDE_OBJS = objs/main/ede.o \
           $(ICA_OBJS) \
           $(XLTEK_OBJS)\
           $(BLINK_OBJS) \
           $(MATRIX_OBJS) \
           $(WAVELET_OBJS) \
           $(EDE_GUI_OBJS)  \
           $(CONVOLUTION_OBJS)

# Runtime evaluator.
BINS := bin/runtime $(BINS)
RUNTIME_OBJS = objs/main/runtime.o \
							 objs/cmd_args/runtime.o \
               $(ICA_OBJS) \
               $(MATRIX_OBJS)

################################################################################
################################################################################
# Compilation rules.
all : $(BINS)

bin/conv_test : $(CONV_TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(CONV_TEST_OBJS) -o bin/conv_test $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

bin/wavelet_test : $(WAVELET_TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(WAVELET_TEST_OBJS) -o bin/wavelet_test $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

bin/ica_test : $(ICA_TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(ICA_TEST_OBJS) -o bin/ica_test $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

bin/bd_test : $(BD_TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(BD_TEST_OBJS) -o bin/bd_test $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

bin/src_test : $(SRC_TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(SRC_TEST_OBJS) -o bin/src_test $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

bin/br_test : $(BR_TEST_OBJS)
	@mkdir -p bin
	$(CXX) $(BR_TEST_OBJS) -o bin/br_test $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

bin/ede : $(EDE_OBJS)
	@mkdir -p bin
	$(CXX) $(EDE_OBJS) -o bin/ede $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

bin/runtime : $(RUNTIME_OBJS)
	@mkdir -p bin
	$(CXX) $(RUNTIME_OBJS) -o bin/runtime $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

################################################################################
# Rule for compiling C files.
objs/%.o : src/%.c
	@mkdir -p $(@D)
	$(CC) $< -c -o $@ $(CFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

################################################################################
# Rule for compiling C++ files.
objs/%.o : src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $< -c -o $@ $(CPPFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

################################################################################
# Rule for compiling CUDA files.
objs/%.o : src/%.cu
	@mkdir -p $(@D)
	$(NVCC) --cuda $< -o $(@:.o=.cpp) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS)
	$(CXX) $(@:.o=.cpp) -c -o $@ $(CPPFLAGS) $(DEFINES) $(INCLUDE) $(LIB_DIRS) $(LIBS) $(PKG_CONFIG)

clean :
	rm -f bin/*
	rm -rf objs
