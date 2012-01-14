all:
	$(CXX) tblf.cpp -o tblf -O3 -Wall -pedantic

test: all
	find test/ -name \*.csv -exec ./tblf {} \;
