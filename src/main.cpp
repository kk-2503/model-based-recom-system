#include <iostream>
#include <cstring>

#include "Sist_Recom.hpp"

using namespace std;

bool processInputParametres(int numberofparametres) {
  if (numberofparametres != 2)
    return false;
  else
    return true;
}

int main(int argc, char** argv) {
  if (processInputParametres(argc)) {
    Sist_Recom sr(argv[1]);
  }
  else {
    cout << "Modo de empleo: ./bin/modelo documentos.txt " << endl;
  }
}