default: release

.PHONY: default release debug all clean

CPP_FILES=$(wildcard src/*.cpp)

DEBUG_D_FILES=$(CPP_FILES:%.cpp=debug/%.cpp.d)
RELEASE_D_FILES=$(CPP_FILES:%.cpp=release/%.cpp.d)

DEBUG_O_FILES=$(CPP_FILES:%.cpp=debug/%.cpp.o)
RELEASE_O_FILES=$(CPP_FILES:%.cpp=release/%.cpp.o)

NONEXEC_CPP_FILES := $(filter-out src/budget.cpp,$(CPP_FILES))

NON_EXEC_DEBUG_O_FILES=$(NONEXEC_CPP_FILES:%.cpp=debug/%.cpp.o)
NON_EXEC_RELEASE_O_FILES=$(NONEXEC_CPP_FILES:%.cpp=release/%.cpp.o)

CXX=clang++
LD=clang++

WARNING_FLAGS=-Wextra -Wall -Qunused-arguments -Wuninitialized -Wsometimes-uninitialized -Wno-long-long -Winit-self -Wdocumentation
CXX_FLAGS=-Iinclude -std=c++11 $(WARNING_FLAGS)
LD_FLAGS=$(CXX_FLAGS) -luuid -lboost_date_time

DEBUG_FLAGS=-g
RELEASE_FLAGS=-g -DNDEBUG -Ofast -march=native -fvectorize -fslp-vectorize-aggressive -fomit-frame-pointer

debug/src/%.cpp.o: src/%.cpp
	@ mkdir -p debug/src/
	$(CXX) $(CXX_FLAGS) $(DEBUG_FLAGS) -o $@ -c $<

release/src/%.cpp.o: src/%.cpp
	@ mkdir -p release/src/
	$(CXX) $(CXX_FLAGS) $(RELEASE_FLAGS) -o $@ -c $<

debug/bin/%: debug/src/%.cpp.o $(NON_EXEC_DEBUG_O_FILES)
	@ mkdir -p debug/bin/
	$(LD) $(LD_FLAGS) $(DEBUG_FLAGS) -o $@ $+

release/bin/%: release/src/%.cpp.o $(NON_EXEC_RELEASE_O_FILES)
	@ mkdir -p release/bin/
	$(LD) $(LD_FLAGS) $(RELEASE_FLAGS) -o $@ $+

debug/src/%.cpp.d: $(CPP_FILES)
	@ mkdir -p debug/src/
	@ $(CXX) $(CXX_FLAGS) $(DEBUG_FLAGS) -MM -MT debug/src/$*.cpp.o src/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

release/src/%.cpp.d: $(CPP_FILES)
	@ mkdir -p release/src/
	@ $(CXX) $(CXX_FLAGS) $(RELEASE_FLAGS) -MM -MT release/src/$*.cpp.o src/$*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

release: release/bin/budget
debug: debug/bin/budget

all: release debug

sonar: release
	cppcheck --xml-version=2 --enable=all --std=c++11 src include 2> cppcheck_report.xml
	/opt/sonar-runner/bin/sonar-runner

install: install-man
	echo "Installation of budgetwarrior"
	install budget.man /usr/share/man/man3/budget.3

clean:
	rm -rf release/
	rm -rf debug/

-include $(DEBUG_D_FILES)
-include $(RELEASE_D_FILES)