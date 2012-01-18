/*
 * tblf
 * A delimiter-seperated data file formatter in C++.
 *
 * Copyright (c) 2012 Wander Nauta, wandernauta.nl 
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm> // For min_element/max_element

#include <getopt.h>

typedef unsigned int uint;
typedef std::vector< std::vector <std::string> > table;
typedef std::vector< int > intvec;
typedef std::map< char, intvec > countmap;
typedef countmap::iterator cmit;

// Find out if a string represents a number.
bool isnum(std::string in) {
  std::istringstream ins(in);
  double test;
  ins >> test;

  if (!ins) return false;
  return (ins.rdbuf()->in_avail() == 0);
}

// Return the string version of an integer value.
std::string strval(int i) {
  std::ostringstream out;
  out << i;
  return out.str();
}

// A UTF-8 strlen function. Also works for ASCII.
uint strlen_utf8(std::string s) {
  uint i = 0, j = 0, sz = s.size();
  while (i < sz) {
    if ((s[i] & 0xc0) != 0x80) j++;
    i++;
  }
  return j;
}

// Try to find which seperator is the most probable.
char probable_sep(std::istream &in) {
  countmap counts;
  counts['\t'] = intvec();
  counts[','] = intvec();
  counts[';'] = intvec();
  counts[':'] = intvec();
  counts['|'] = intvec();

  std::string line;
  char c;

  while (in.good()) {
    getline(in, line);
    if (line == "") continue;

    // Add a new zero to every counts vector
    for (cmit it = counts.begin(); it != counts.end(); ++it) {
      it->second.push_back(0);
    }

    // Consume the line char by char
    std::istringstream lines(line);
    while (lines.good()) {
      lines.get(c);

      if (c == '\t' or c == ';' or c == ':' or c == '|' or c == ',') {
        int lastidx = counts[c].size() - 1;
        counts[c][lastidx] += 1;
      }
    } 
  } 

  // Check which count vector has identical non-zero values
  
  for (cmit it = counts.begin(); it != counts.end(); ++it) {
    intvec vec = it->second;

    if (vec.empty()) break; // No rows! Bail.

    int min = *std::min_element(vec.begin(), vec.end());
    int max = *std::max_element(vec.begin(), vec.end());

    if (min == max && min != 0) {
      in.clear();
      return it->first;
    }
  }

  // No plausible seperator found, return something that won't collide
  return '\0';
}

// Enlarge a vector of ints to size `count`, adding zero's if necessary
void ensure_sz(std::vector<uint> & vec, uint count) {
  if (vec.size() < count) {
    vec.resize(count, 0);
  }
}

// Return the length of the longest value for each column in a table.
std::vector<uint> col_widths(table &tblf) {
  std::vector<uint> w;

  for (uint row = 0; row < tblf.size(); row++) {
    for (uint col = 0; col < tblf[row].size(); col++) {
      ensure_sz(w, col+1);
      uint width = strlen_utf8(tblf[row][col]);
      if (w[col] < width) {
        w[col] = width;
      }
    }
  }

  return w;
}

// Format an istream table onto stdout.
int tblf(std::istream & f, char sep, bool want_zebra, bool want_right, bool want_lineno) {
  std::string line;
  table rows;

  if (sep == '\0') {
    sep = probable_sep(f);
    f.seekg(0, std::ios::beg);
  }

  int lineno = 1;

  while (f.good()) {
    getline(f, line);
    std::vector<std::string> row;
    std::string val;
    if (line == "") {
      continue; // Collapse empty lines
    } else {
      if (want_lineno) row.push_back(strval(lineno));

      std::istringstream lines(line);
      while (lines.good()) {
        getline(lines, val, sep);
        row.push_back(val);
      }
      rows.push_back(row);
      lineno++;
    }
  }

  std::vector<uint> widths = col_widths(rows);

  // If something went wrong during the parsing stage, bail.
  if (rows.empty()) {
    std::cerr << "tblf: Misformed or empty data file, quitting after reading " << lineno << " lines" << std::endl;
    return 1;
  }

  for (uint row = 0; row < rows.size(); row++) {
    if (want_zebra and (row % 2 == 1)) std::cout << "\x1b[1m";

    for (uint col = 0; col < rows[row].size(); col++) {
      std::string val = rows[row][col];
      int len = strlen_utf8(val);
      int width = widths[col];
      int pad = (width - len);
      
      if (want_right and isnum(val)) {
        std::cout << std::string(pad, ' ') << val;
      } else {
        std::cout << val << std::string(pad, ' ');
      }

      std::cout << "  ";
    }

    if (want_zebra and (row % 2 == 1)) std::cout << "\x1b[0m";
    std::cout << std::endl;
  }

  return 0;
}

// It's quite obvious what this thing does.
int main(int argc, char* argv[]) {
  char sep = '\0';
  bool want_zebra = false;  // Do I want zebra striping?
  bool want_right = true;   // Do I want right-alignment of numbers?
  bool want_lineno = false; // Do I want line numbers?
  int c;

  while ((c = getopt (argc, argv, "d:zlrn")) != -1) {
    switch (c) {
      case 'd':
        sep = optarg[0];
        break;
      case 'z':
        want_zebra = true; // Yes, I want zebra striping!
        break;
      case 'l':
        want_right = false; // Left-align numeric columns
        break;
      case 'r':
        want_right = true; // Right-align numeric columns
        break;
      case 'n':
        want_lineno = true; // Yes, I want line numbering!
        break;
    }
  }

  if (!argv[optind]) {
    std::cerr << "usage: tblf [-lrnz] [-d delimiter] FILE" << std::endl;
    std::cerr << "       tblf [-lrnz] [-d delimiter] -" << std::endl;
    return 1;
  }

  std::string file = std::string(argv[optind]);

  if (file == "-") {
    // Use stdin
    std::stringstream ss;
    ss << std::cin.rdbuf();
    return tblf(ss, sep, want_zebra, want_right, want_lineno);
  } else {
    // Open a file and use that
    std::ifstream f(file.c_str());
    return tblf(f, sep, want_zebra, want_right, want_lineno);
  }

  // This shouldn't happen
  return 1;
}

