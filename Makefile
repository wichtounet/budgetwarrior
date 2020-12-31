default: release_debug

.PHONY: default release debug all clean

include make-utils/flags.mk
include make-utils/cpp-utils.mk

# Use C++17
$(eval $(call use_cpp17))

CXX_FLAGS += -pthread

ifneq (,$(findstring clang,$(CXX)))
	CXX_FLAGS += -stdlib=libc++
endif

LD_FLAGS += -luuid -lssl -lcrypto -ldl

CXX_FLAGS += -Icpp-httplib

# Add test includes
CXX_FLAGS += -Idoctest -Itest/include -Iloguru -Ifmt/include

$(eval $(call auto_folder_compile,src))
$(eval $(call auto_folder_compile,src/pages))
$(eval $(call auto_folder_compile,src/api))
$(eval $(call auto_add_executable,budget))

# Create the test executable
$(eval $(call folder_compile,test/src))
TEST_CPP_FILES=$(wildcard test/src/*.cpp) $(filter-out src/budget.cpp src/server.cpp src/api/%.cpp src/pages/%.cpp, $(AUTO_CXX_SRC_FILES))
$(eval $(call add_executable,budget_test,$(TEST_CPP_FILES)))
$(eval $(call add_executable_set,budget_test,budget_test))

release_debug: release_debug_budget
release: release_budget
debug: debug_budget

debug_test: debug_budget_test
release_debug_test: release_debug_budget_test
release_test: release_budget_test

run_debug_test: debug_budget_test
	./debug/bin/budget_test

run_release_debug_test: release_debug_budget_test
	./release_debug/bin/budget_test

run_release_test: release_budget_test
	./release/bin/budget_test

all: release release_debug debug

sonar: release
	cppcheck --xml-version=2 --enable=all --std=c++11 src include 2> cppcheck_report.xml
	/opt/sonar-runner/bin/sonar-runner

prefix ?= /usr/local
bindir = $(prefix)/bin
mandir = $(prefix)/share/man

install_extra:
	@ echo "Installation of budgetwarrior extra"
	@ echo "============================="
	@ echo ""
	install budget.man $(mandir)/man3/budget.3
	install completions/bash $(prefix)/etc/bash_completion.d/budget
	install completions/zsh $(prefix)/share/zsh/site-functions/_budget

install: release_debug
	@ echo "Installation of budgetwarrior"
	@ echo "============================="
	@ echo ""
	install release_debug/bin/budget $(bindir)/budget
	install tools/yfinance_quote.py $(bindir)/yfinance_quote.py

install_light: install

clean: base_clean

include make-utils/cpp-utils-finalize.mk
