.PHONY: all clean realclean
.SECONDARY:

top:=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

all:: demo

all-src:=demo.cc model_config_io.cc

vpath %.cc $(top)

CXXSTD?=c++23
OPTFLAGS?=-O2 -fsanitize=address -march=native
CXXFLAGS+=$(OPTFLAGS) -MMD -MP -std=$(CXXSTD) -pedantic -Wall -Wextra -g -pthread
CPPFLAGS+=-I $(top)include

depends:=$(patsubst %.cc, %.d, $(all-src))
-include $(depends)

demo: demo.o model_config_io.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f demo.o model_config.o

realclean: clean
	rm -f $(depends)
