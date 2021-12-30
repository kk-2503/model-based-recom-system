#include "../include/Sist_Recom.hpp"

Sist_Recom::Sist_Recom(string file) {
  ifstream input_file;
  input_file.open(file);
  string v, word;
  map<string, int> doc_word;
  map<int, string> doc_index;
  if (input_file.is_open()) {
    while(getline(input_file, v)) {
      v.append("\n");
      word.clear();
      int index = 0;
      for (string::size_type i = 0; i < v.size(); i++) {
        if (v[i] == ' ' || v[i] == '\n') {
          if (doc_word.find(word) == doc_word.end()) {
            doc_word[word] = 1;
            doc_index[index] = word;
          } else {
            doc_word[word] += 1;
          }
          word.clear();
          index++;
        } else {
          if (v[i] != '.' && v[i] != ',') {
            word += v[i];
          }
        }
      }
      words_freq.push_back(doc_word);
      words_index.push_back(doc_index);
      doc_word.clear();
      doc_index.clear();

    }
    calculateTF();
    calculateIDF();
    calculateTFIDF();
    calculateSimCos();
    print();
    input_file.close();
  }
  else {
    cout << "Error en la apertura del fichero" << endl;
    exit(1);
  }
}

Sist_Recom::~Sist_Recom() {

}

void Sist_Recom::calculateTF(void) {
  map<string, double> doc_tf;
  for (vector<map<string, int>>::size_type i = 0; i < words_freq.size(); i++) {
    for (map<string, int>::const_iterator j = words_freq[i].begin(); j != words_freq[i].end(); j++) {
      doc_tf[j->first] = 1 + log10(j->second);
    }
    words_tf.push_back(doc_tf);
    doc_tf.clear();
  }
}

void Sist_Recom::calculateIDF(void) {
  vector<string> words;
  for (vector<map<string, int>>::size_type i = 0; i < words_freq.size(); i++) {
    for (map<string, int>::const_iterator j = words_freq[i].begin(); j != words_freq[i].end(); j++) {
      if(find(words.begin(), words.end(), j->first) == words.end())
        words.push_back(j->first);
    }
  }

  map<string, int> words_df;
  for (vector<string>::size_type i = 0; i < words.size(); i++) {
    int df = 0;
    for (vector<map<string, int>>::size_type j = 0; j < words_freq.size(); j++) {
      if (words_freq[j].find(words[i]) != words_freq[j].end()) {
        df++;
      }
    }
    words_df[words[i]] = df;
  }

  for (map<string, int>::const_iterator j = words_df.begin(); j != words_df.end(); j++) {
    words_idf[j->first] = log10(words_freq.size() / j->second);
  }
}

void Sist_Recom::calculateTFIDF(void) {
  for (vector<map<string, double>>::size_type i = 0; i < words_tf.size(); i++) {
    map<string, double> doc_tfidf;
    for (map<string, double>::const_iterator j = words_tf[i].begin(); j != words_tf[i].end(); j++) {
      doc_tfidf[j->first] = j->second * words_idf[j->first];
    }
    words_tfidf.push_back(doc_tfidf);
  }
}

void Sist_Recom::calculateSimCos(void) {
  vector<double> v_length(words_tf.size(), 0);
  for (vector<map<string, double>>::size_type i = 0; i < words_tf.size(); i++) {
    for (map<string, double>::const_iterator j = words_tf[i].begin(); j != words_tf[i].end(); j++) {
      v_length[i] += pow(j->second, 2);
    }
    v_length[i] = sqrt(v_length[i]);
  }

  vector<map<string, double>> v_normal(words_tf.size());
  for (vector<map<string, double>>::size_type i = 0; i < words_tf.size(); i++) {
    map<string, double> doc_normal;
    for (map<string, double>::const_iterator j = words_tf[i].begin(); j != words_tf[i].end(); j++) {
      doc_normal[j->first] = j->second / v_length[i];
    }
    v_normal[i] = doc_normal;
  }

  for (vector<map<string, double>>::size_type i = 0; i < v_normal.size(); i++) {
    for (vector<map<string, double>>::size_type j = 0; (j < v_normal.size()) && (j != i); j++) {
      pair<int, int> docs = make_pair(i, j);
      sim[docs] = 0;
      for (map<string, double>::const_iterator k = v_normal[i].begin(); k != v_normal[i].end(); k++) {
        if (v_normal[j].find(k->first) != v_normal[j].end()) {
          sim[docs] += k->second * v_normal[j][k->first];
        }
      }
    }
  }
}

void Sist_Recom::print(void) {
  for (vector<map<int, string>>::size_type i = 0; i < words_index.size(); i++) {
    cout << "DOCUMENTO " << i+1 << ": " << endl;
    cout << setw(5) << "Index " << setw(20) << "Termino" << setw(15) << "TF" << setw(15) << "IDF" << setw(15) << "TF-IDF" << endl;
    for (map<int, string>::const_iterator j = words_index[i].begin(); j != words_index[i].end(); j++) {
      cout << setw(5) << j->first << setw(20) << j->second << setw(15) << words_tf[i][j->second] << setw(15) << words_idf[j->second] << setw(15) << words_tfidf[i][j->second] << endl;
    }
    cout << endl;
  }

  cout << endl;
  cout << "SIMILITUD COSENO ENTRE CADA PAR DE DOCUMENTOS: " << endl;
  pair<int, int> maximum;
  double max_sim = 0;
  for (map<pair<int, int>, double>::const_iterator i = sim.begin(); i != sim.end(); i++) {
    if (max(i->second, max_sim) == i->second) {
      maximum = i->first;
      max_sim = i->second;
    }
    cout << "Documento " << i->first.first + 1 << " y " << i->first.second + 1 << ": " << i->second << endl;
  }

  cout << "Los dos documentos mas similares son el documento " << maximum.first + 1 << " y el documento " << maximum.second + 1 << " con una simitud de " << max_sim << endl;
}