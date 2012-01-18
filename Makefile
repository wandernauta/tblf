all: tblf
tblf: tblf.cpp
	$(CXX) tblf.cpp -o tblf -O3 -Wall -pedantic

test: all
	find test/ -type f -print -exec ./tblf -n {} \;

README:
	MANWIDTH=80 man -l ./tblf.1 | col | head -n-2 | tail -n+3 > README

clean:
	rm tblf

.PHONY: README clean
