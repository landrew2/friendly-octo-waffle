# simple makefile to build all .c or .cpp files and run to create a png

# list of all .o files that could be generated from .cpp files
# uses all source files in this directory
# typically, you would list these out by hand to say exactly what to use
OBJS = $(patsubst %.cpp, build/%.o, $(wildcard *.cpp))

# set to -O for optimized, -g for debug; also can use 'make OPT=-g'
OPT = -O

# maximum warnings, good idea to catch easy bugs
CXXFLAGS += -Wall
CFLAGS += -Wall

# how to build trace executable from OBJS files
# link with c++ compiler to allow c++ code
# $@ is the current target (trace)
# set OPT variable to -g or -O for debugging or optimization
# set LDFLAGS to any library directorys (-L...) to search
# set LDLIBS to any libraries to use (-l...)
# should be OK to leave all of those blank
build/trace: $(OBJS)
	mkdir -p build
	$(CXX) $(OPT) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

# .o from .cpp, also generating dependency file
build/%.o: %.cpp
	mkdir -p build
	$(CXX) $(OPT) -c -o $@ $< $(CXXFLAGS)
	$(CXX) -MM -o build/$*.d $<

# automatic dependency tracking using generated *.d files
-include $(patsubst %.o, %.d, $(OBJS))

clean:
	rm -rf build
