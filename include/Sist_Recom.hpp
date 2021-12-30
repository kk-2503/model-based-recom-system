#pragma once

#include <iostream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

class Sist_Recom {
private:
  vector<map<int, string>> words_index;
  vector<map<string, int>> words_freq;
  vector<map<string, double>> words_tf;
  map<string, double> words_idf;
  vector<map<string, double>> words_tfidf;
  map<pair<int, int>, double> sim;
public:
  Sist_Recom(string file);
  ~Sist_Recom();
  
  void calculateTF(void);
  void calculateIDF(void);
  void calculateTFIDF(void);
  void calculateSimCos(void);

  void print(void);
};