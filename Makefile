SDSL_DIR=../sdsl-lite
include $(SDSL_DIR)/Make.helper

# Multithreading with OpenMP.
PARALLEL_FLAGS=-fopenmp -pthread
LIBS=-L$(LIB_DIR) -lsdsl -ldivsufsort -ldivsufsort64

ifeq ($(shell uname -s),Darwin)
    # Our compiler might be clang that lacks -fopenmp support.
    # Sniff that
    ifeq ($(strip $(shell $(MY_CXX) -fopenmp /dev/null -o/dev/null 2>&1 | grep fopenmp | wc -l)), 1)
		# The compiler complained about fopenmp instead of its nonsense input file.
        # We need to use the hard way of getting OpenMP not bundled with the compiler.
        
        # The compiler only needs to do the preprocessing
        PARALLEL_FLAGS = -Xpreprocessor -fopenmp -pthread

        ifeq ($(shell if [ -d /opt/local/lib/libomp ];then echo 1;else echo 0;fi), 1)
            # Use /opt/local/lib/libomp if present, because Macports installs libomp there.
            # Brew is supposed to put it somewhere the compiler can find it by default.
            LIBS += -L/opt/local/lib/libomp
            # And we need to find the includes. Homebrew puts them in the normal place
            # but Macports hides them in "libomp"
            PARALLEL_FLAGS += -I/opt/local/include/libomp
        endif

        # We also need to link it
        LIBS += -lomp
		
    endif
endif

OTHER_FLAGS=$(PARALLEL_FLAGS)

CXX_FLAGS=$(MY_CXX_FLAGS) $(OTHER_FLAGS) $(MY_CXX_OPT_FLAGS) -Iinclude -I$(INC_DIR)
LIBOBJS=algorithms.o bwtmerge.o cached_gbwt.o dynamic_gbwt.o files.o gbwt.o internal.o metadata.o support.o utils.o variants.o
SOURCES=$(wildcard *.cpp)
HEADERS=$(wildcard include/gbwt/*.h)
OBJS=$(SOURCES:.cpp=.o)

LIBRARY=libgbwt.a
PROGRAMS=build_gbwt merge_gbwt benchmark metadata_tool remove_seq
OBSOLETE=prepare_text prepare_text.o metadata

all:$(LIBRARY) $(PROGRAMS)

%.o:%.cpp $(HEADERS)
	$(MY_CXX) $(CPPFLAGS) $(CXXFLAGS) $(CXX_FLAGS) -c $<

$(LIBRARY):$(LIBOBJS)
	ar rcs $@ $(LIBOBJS)

build_gbwt:build_gbwt.o $(LIBRARY)
	$(MY_CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXX_FLAGS) -o $@ $< $(LIBRARY) $(LIBS)

merge_gbwt:merge_gbwt.o $(LIBRARY)
	$(MY_CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXX_FLAGS) -o $@ $< $(LIBRARY) $(LIBS)

benchmark:benchmark.o $(LIBRARY)
	$(MY_CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXX_FLAGS) -o $@ $< $(LIBRARY) $(LIBS)

metadata_tool:metadata_tool.o $(LIBRARY)
	$(MY_CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXX_FLAGS) -o $@ $< $(LIBRARY) $(LIBS)

remove_seq:remove_seq.o $(LIBRARY)
	$(MY_CXX) $(LDFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(CXX_FLAGS) -o $@ $< $(LIBRARY) $(LIBS)

test:$(LIBRARY)
	cd tests && $(MAKE) test

clean:
	rm -f $(PROGRAMS) $(OBJS) $(LIBRARY) $(OBSOLETE)
