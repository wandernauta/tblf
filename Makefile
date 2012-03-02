# This Makefile builds tblf, the table formatter.

INSTALL_PATH ?= /usr/local

all: tblf
tblf: tblf.cpp
	$(CXX) tblf.cpp -o tblf -O3 -Wall -pedantic

check: all
	./tblf -nz test/1.csv
	./tblf -n test/2.tsv
	./tblf test/3.csv

README:
	MANWIDTH=80 man -l ./tblf.1 | col | head -n-2 | tail -n+3 > README

clean:
	rm tblf

install:
	install ./tblf $(INSTALL_PATH)/bin

uninstall:
	rm -i $(INSTALL_PATH)/bin/tblf

help:
	@echo "This Makefile supports the following targets:"
	@echo "- all       (builds tblf)"
	@echo "- clean     (removes the tblf binary)"
	@echo "- check     (does a few example runs"
	@echo "- install   (copies tblf to $(INSTALL_PATH)/bin)"
	@echo "- uninstall (removes tblf from $(INSTALL_PATH)/bin)"

.PHONY: README clean
