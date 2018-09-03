default: release_debug

.PHONY: default release debug all clean

include make-utils/flags.mk
include make-utils/cpp-utils.mk

CXX_FLAGS += -pthread

ifneq (,$(findstring clang,$(CXX)))
	CXX_FLAGS += -stdlib=libc++
endif

LD_FLAGS += -luuid -lssl -lcrypto

CXX_FLAGS += -Icpp-httplib

$(eval $(call auto_folder_compile,src))
$(eval $(call auto_add_executable,budget))

release_debug: release_debug_budget
release: release_budget
debug: debug_budget

all: release release_debug debug

sonar: release
	cppcheck --xml-version=2 --enable=all --std=c++11 src include 2> cppcheck_report.xml
	/opt/sonar-runner/bin/sonar-runner

prefix = /usr/local
bindir = $(prefix)/bin
mandir = $(prefix)/share/man

install: release_debug
	@ echo "Installation of budgetwarrior"
	@ echo "============================="
	@ echo ""
	install budget.man $(mandir)/man3/budget.3
	install release_debug/bin/budget $(bindir)/budget
	install completions/bash $(prefix)/etc/bash_completion.d/budget
	install completions/zsh $(prefix)/share/zsh/site-functions/_budget

clean: base_clean

include make-utils/cpp-utils-finalize.mk
