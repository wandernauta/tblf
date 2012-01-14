all:
	$(CXX) tblf.cpp -o tblf -O3 -Wall -pedantic

test: all
	find test/ -name \*.csv -exec ./tblf {} \;

README:
	MANWIDTH=80 man -l ./tblf.1 | col | head -n-2 | tail -n+3 > README

.PHONY: README
