# TAREA: Sistemas de Recomendación. Modelos Basados en el Contenido

El objetivo de esta práctica es implementar un sistema de recomendación siguiendo el modelo basados en el contenido.

## Descripción de la Solución Propuesta

El lenguaje de programación usado en esta práctica es C++. A continuación se va a explicar el código fuente desarrollado:

### Fichero src/main.cpp

```cpp
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
```

Lo primero que se hace es asegurar que el número de parámetros sea el adecuado. En este caso, sólo tendremos uno (el nombre del fichero con extensión .txt que contiene los documentos a analizar), pero como se puede observar en el código se ha especificado que hay dos parámetros, ya que en c++ el nombre del ejecutable también se considera un argumento. Si no se ha recibido el número correcto de argumentos, el programa lanza un mensaje explicativo de cómo usar el programa.

### Fichero include/Sist_Recom.hpp

Para el desarrollo de la solución se ha optado por usar una clase, **Sist_Recom**:

```cpp
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
```

Esta clase contiene un atributo para cada valor que se debe calcular. `vector<map<int, string>> words_index` contiene los indices de las palabras, donde cada posición del vector indica un documento almacenado como un **map** cuya *key* es el índice y *value* es la palabra a la que corresponde dicho índice. `vector<map<string, int>> words_freq` almacena la frequencia de cada palabra en un documento y su estructura es similar a la de *words_index*. De la misma forma `vector<map<string, double>> words_tf` contiene el valor TF de cada palabra, `map<string, double> words_idf` almacena el valor IDF y `vector<map<string, double>> words_tfidf` contiene los valores TF-IDF de cada palabra. En cuanto a `map<pair<int, int>, double> sim`, almacena los valores de la similitud coseno de cada par de documentos. Para ello, se ha optado por usar la estructura **pair** junto con **map**, que almacena los documentos a los que corresponde el valor de la similitud (*pair.first* y *pair.second* contienen el índice de los documentos, y el *value* del map es el valor de la similitud). Luego tenemos un constructor que se encarga de leer el contenido del fichero .txt y calcular los índices y las frecuencias de las palabras, aparte de invocar los métodos que calculan el resto de los valores y el que imprime los resultados por la pantalla.

### Fichero src/Sist_Recom.cpp

Contiene el código de cada método cuyas cabeceras se habían definido en *include/Sist_Recom.hpp*.

```cpp
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
```

El método `calculateTF()` calcula el valor TF de cada palabra usando la fórmula `TF[palabra] = 1 + log10(freq_palabra)`

```cpp
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
```

Para calcular el valor IDF de cada palabra lo primero que se debe hacer es calcular el DF. Para ello, lo primero que hacemos es crear un vector que sólo contenga las palabras únicas en todos los documentos, y luego a partir de ese vector calculamos cúantos documentos tienen una palabra X (DF). Finalmente, obtenemos el valor IDF de X a través de la fórmula `IDF(X) = log10(N / DF(X))`, donde N es el número de documentos del corpus.

```cpp
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
```

Para calcular la similitud coseno de los documentos, primero se debe calcular el vector normalizado, y para ello necesitamos el tamaño del vector normalizado, que se calcula con la raíz cuadrada de la suma de los valores TF de cada palabra al cuadrado `tamaño[documento] = sqrt(palabra1 ^ 2 + palabra2 ^ 2 + ... + palabraN ^ 2)` donde palabra*i* i=1...N son las palabras del documento en cuestión. Luego para calcular el vector normalizado usamos `valor_normal[palabra] = TF[palabra] / tamaño[documento]`. Y finalmente obtenemos la similitud entre el documento X e Y con `sim(X, Y) = valor_normal[X][palabra1] x valor_normal[Y][palabra1] + valor_normal[X][palabra2] x valor_normal[Y][palabra2] + ...`.

### Compilación y Ejecución del Programa

Para compilar el programa se puede usar el fichero Makefile ejecutando el comando `make` en la línea de comandos. Y para ejecutarlo se debe usar el comando `./bin/modelo test.txt`, donde en lugar de test.txt se puede usar cualquier fichero con extensión .txt que contenga los documentos a analizar separados en diferentes líneas. Un ejemplo de uso sería el siguiente:

```bash
usuario@ubuntu:~/gco/sist_recom$ make
make: Nothing to be done for 'all'.
usuario@ubuntu:~/gco/sist_recom$ ./bin/modelo test.txt
DOCUMENTO 1: 
Index              Termino             TF            IDF         TF-IDF
    0              Aromas              1              1              1
    1             include              1              1              1
    2            tropical              1              1              1
    3               fruit              1        0.69897        0.69897
    4               broom              1              1              1
    5           brimstone              1              1              1
    6                 and        1.30103              0              0
    7               dried        1.30103        0.69897       0.909381
    8                herb              1        0.69897        0.69897
    9                 The              1       0.477121       0.477121
   10              palate              1       0.477121       0.477121
   11               isn't              1              1              1
   12              overly              1              1              1
   13          expressive              1              1              1
   14            offering              1              1              1
   15           unripened              1              1              1
   16               apple              1        0.69897        0.69897
   17              citrus              1              1              1
   20                sage              1              1              1
   21           alongside              1              1              1
   22               brisk              1        0.69897        0.69897
   23             acidity              1              0              0

DOCUMENTO 2: 
Index              Termino             TF            IDF         TF-IDF
    0                This              1       0.477121       0.477121
    1                  is        1.30103       0.477121       0.620749
    2                ripe              1              1              1
    3                 and        1.30103              0              0
    4              fruity              1        0.69897        0.69897
    5                   a              1              0              0
    6                wine              1        0.30103        0.30103
    7                that              1        0.69897        0.69897
    9              smooth              1              1              1
   10               while              1       0.477121       0.477121
   11               still              1              1              1
   12          structured              1              1              1
   13                Firm              1              1              1
   14             tannins              1        0.69897        0.69897
   15                 are              1              1              1
   16              filled              1              1              1
   17                 out              1              1              1
   18                with        1.30103              0              0
   19               juicy              1              1              1
   20                 red              1        0.69897        0.69897
   21               berry              1        0.69897        0.69897
   22              fruits              1        0.69897        0.69897
   24           freshened              1              1              1
   26             acidity              1              0              0
   27                It's              1        0.30103        0.30103
   28                                  1              1              1
   29             already              1              1              1
   30           drinkable              1              1              1
   31            although              1              1              1
   32                  it              1        0.69897        0.69897
   33                will              1              1              1
   34           certainly              1              1              1
   35                  be              1              1              1
   36              better              1              1              1
   37                from              1        0.69897        0.69897
   38                2016              1              1              1

DOCUMENTO 3: 
Index              Termino             TF            IDF         TF-IDF
    0                Tart              1              1              1
    1                 and        1.30103              0              0
    2              snappy              1              1              1
    3                 the        1.30103        0.30103       0.391649
    4             flavors        1.30103       0.477121       0.620749
    5                  of              1              0              0
    6                lime              1              1              1
    7               flesh              1              1              1
    9                rind              1        0.69897        0.69897
   10            dominate              1              1              1
   11                Some              1              1              1
   12               green              1        0.69897        0.69897
   13           pineapple              1              1              1
   14               pokes              1              1              1
   15             through              1              1              1
   16                with              1              0              0
   17               crisp              1        0.69897        0.69897
   18             acidity              1              0              0
   19        underscoring              1              1              1
   22                 The              1       0.477121       0.477121
   23                wine              1        0.30103        0.30103
   24                 was              1              1              1
   25                 all              1              1              1
   26     stainless-steel              1              1              1
   27           fermented              1              1              1

DOCUMENTO 4: 
Index              Termino             TF            IDF         TF-IDF
    0           Pineapple              1              1              1
    1                rind              1        0.69897        0.69897
    2               lemon              1              1              1
    3                pith              1              1              1
    4                 and        1.30103              0              0
    5              orange              1              1              1
    6             blossom              1              1              1
    7               start              1              1              1
    8                 off              1        0.69897        0.69897
    9                 the              1        0.30103        0.30103
   10              aromas              1       0.477121       0.477121
   11                 The              1       0.477121       0.477121
   12              palate              1       0.477121       0.477121
   13                  is              1       0.477121       0.477121
   14                   a        1.30103              0              0
   15                 bit              1              1              1
   16                more              1              1              1
   17             opulent              1              1              1
   18                with              1              0              0
   19               notes              1        0.69897        0.69897
   20                  of              1              0              0
   21      honey-drizzled              1              1              1
   22               guava              1              1              1
   24               mango              1              1              1
   25              giving              1              1              1
   26                 way              1              1              1
   27                  to              1       0.477121       0.477121
   29            slightly              1              1              1
   30          astringent              1              1              1
   31             semidry              1              1              1
   32              finish              1        0.69897        0.69897

DOCUMENTO 5: 
Index              Termino             TF            IDF         TF-IDF
    0                Much              1              1              1
    1                like              1              1              1
    2                 the              1        0.30103        0.30103
    3             regular              1              1              1
    4            bottling              1              1              1
    5                from              1        0.69897        0.69897
    6                2012              1              1              1
    7                this              1       0.477121       0.477121
    8               comes              1              1              1
    9              across              1              1              1
   10                  as        1.30103              1        1.30103
   11              rather              1              1              1
   12               rough              1              1              1
   13                 and              1              0              0
   14              tannic              1              1              1
   15                with              1              0              0
   16              rustic              1              1              1
   17              earthy              1              1              1
   18              herbal              1        0.69897        0.69897
   19     characteristics              1              1              1
   20         Nonetheless              1              1              1
   21                  if              1              1              1
   22                 you              1              1              1
   23               think              1              1              1
   24                  of              1              0              0
   25                  it              1        0.69897        0.69897
   27                   a        1.47712              0              0
   28          pleasantly              1              1              1
   29             unfussy              1              1              1
   30             country              1              1              1
   31                wine              1        0.30103        0.30103
   32                it's              1        0.69897        0.69897
   34                good              1              1              1
   35           companion              1              1              1
   36                  to              1       0.477121       0.477121
   38              hearty              1              1              1
   39              winter              1              1              1
   40                stew              1              1              1

DOCUMENTO 6: 
Index              Termino             TF            IDF         TF-IDF
    0          Blackberry              1              1              1
    1                 and        1.30103              0              0
    2           raspberry              1              1              1
    3              aromas              1       0.477121       0.477121
    4                show              1              1              1
    5                   a              1              0              0
    6             typical              1              1              1
    7            Navarran              1              1              1
    8               whiff              1              1              1
    9                  of              1              0              0
   10               green              1        0.69897        0.69897
   11               herbs              1              1              1
   13                  in              1       0.477121       0.477121
   14                this        1.30103       0.477121       0.620749
   15                case              1              1              1
   16         horseradish              1              1              1
   17                  In              1              1              1
   18                 the        1.30103        0.30103       0.391649
   19               mouth              1              1              1
   21                  is        1.30103       0.477121       0.620749
   22              fairly              1              1              1
   23                full              1              1              1
   24              bodied              1              1              1
   25                with              1              0              0
   26            tomatoey              1              1              1
   27             acidity              1              0              0
   28               Spicy              1              1              1
   29              herbal              1        0.69897        0.69897
   30             flavors              1       0.477121       0.477121
   31          complement              1              1              1
   32                dark              1              1              1
   33                plum              1              1              1
   34               fruit              1        0.69897        0.69897
   35               while              1       0.477121       0.477121
   37              finish              1        0.69897        0.69897
   39               fresh              1        0.30103        0.30103
   40                 but              1              1              1
   41              grabby              1              1              1

DOCUMENTO 7: 
Index              Termino             TF            IDF         TF-IDF
    0              Here's              1              1              1
    1                   a              1              0              0
    2              bright              1              1              1
    3            informal              1              1              1
    4                 red              1        0.69897        0.69897
    5                that        1.30103        0.69897       0.909381
    6               opens              1              1              1
    7                with        1.30103              0              0
    8              aromas              1       0.477121       0.477121
    9                  of              1              0              0
   10             candied              1              1              1
   11               berry              1        0.69897        0.69897
   12               white              1              1              1
   13              pepper              1              1              1
   14                 and        1.30103              0              0
   15              savory              1              1              1
   16                herb              1        0.69897        0.69897
   18               carry              1              1              1
   19                over              1              1              1
   20                  to              1       0.477121       0.477121
   21                 the              1        0.30103        0.30103
   22              palate              1       0.477121       0.477121
   23                It's              1        0.30103        0.30103
   24            balanced              1        0.69897        0.69897
   26               fresh              1        0.30103        0.30103
   27             acidity              1              0              0
   29                soft              1              1              1
   30             tannins              1        0.69897        0.69897

DOCUMENTO 8: 
Index              Termino             TF            IDF         TF-IDF
    0                This              1       0.477121       0.477121
    1                 dry              1        0.69897        0.69897
    2                 and        1.30103              0              0
    3          restrained              1              1              1
    4                wine              1        0.30103        0.30103
    5              offers              1              1              1
    6               spice              1        0.69897        0.69897
    7                  in              1       0.477121       0.477121
    8           profusion              1              1              1
    9            Balanced              1              1              1
   10                with              1              0              0
   11             acidity              1              0              0
   13                   a              1              0              0
   14                firm              1              1              1
   15             texture              1        0.69897        0.69897
   16                it's              1        0.69897        0.69897
   17                very              1              1              1
   18                much              1              1              1
   19                 for              1              1              1
   20                food              1              1              1

DOCUMENTO 9: 
Index              Termino             TF            IDF         TF-IDF
    0              Savory              1              1              1
    1               dried              1        0.69897        0.69897
    2               thyme              1              1              1
    3               notes              1        0.69897        0.69897
    4              accent              1              1              1
    5             sunnier              1              1              1
    6             flavors              1       0.477121       0.477121
    7                  of              1              0              0
    8           preserved              1              1              1
    9               peach              1              1              1
   10                  in              1       0.477121       0.477121
   11                this              1       0.477121       0.477121
   12               brisk              1        0.69897        0.69897
   13             off-dry              1              1              1
   14                wine              1        0.30103        0.30103
   15                It's              1        0.30103        0.30103
   16              fruity              1        0.69897        0.69897
   17                 and              1              0              0
   18               fresh              1        0.30103        0.30103
   19                with              1              0              0
   20                  an              1              1              1
   21             elegant              1              1              1
   22           sprightly              1              1              1
   23           footprint              1              1              1

DOCUMENTO 10: 
Index              Termino             TF            IDF         TF-IDF
    0                This              1       0.477121       0.477121
    1                 has              1              1              1
    2               great              1              1              1
    3               depth              1              1              1
    4                  of        1.30103              0              0
    5              flavor              1              1              1
    6                with        1.30103              0              0
    7                 its              1              1              1
    8               fresh              1        0.30103        0.30103
    9               apple              1        0.69897        0.69897
   10                 and        1.47712              0              0
   11                pear              1              1              1
   12              fruits              1        0.69897        0.69897
   14               touch              1              1              1
   16               spice              1        0.69897        0.69897
   17                It's              1        0.30103        0.30103
   18                 off              1        0.69897        0.69897
   19                 dry              1        0.69897        0.69897
   20               while              1       0.477121       0.477121
   21            balanced              1        0.69897        0.69897
   23             acidity              1              0              0
   25                   a              1              0              0
   26               crisp              1        0.69897        0.69897
   27             texture              1        0.69897        0.69897
   28               Drink              1              1              1
   29                 now              1              1              1


SIMILITUD COSENO ENTRE CADA PAR DE DOCUMENTOS: 
Documento 2 y 1: 0.0902351
Documento 3 y 1: 0.146744
Documento 3 y 2: 0.155517
Documento 4 y 1: 0.134182
Documento 4 y 2: 0.159349
Documento 4 y 3: 0.23617
Documento 5 y 1: 0.0426057
Documento 5 y 2: 0.181676
Documento 5 y 3: 0.170487
Documento 5 y 4: 0.200994
Documento 6 y 1: 0.11959
Documento 6 y 2: 0.195079
Documento 6 y 3: 0.261431
Documento 6 y 4: 0.264077
Documento 6 y 5: 0.207842
Documento 7 y 1: 0.176939
Documento 7 y 2: 0.315769
Documento 7 y 3: 0.220569
Documento 7 y 4: 0.297809
Documento 7 y 5: 0.204414
Documento 7 y 6: 0.265423
Documento 8 y 1: 0.122406
Documento 8 y 2: 0.249151
Documento 8 y 3: 0.198246
Documento 8 y 4: 0.154274
Documento 8 y 5: 0.201156
Documento 8 y 6: 0.19599
Documento 8 y 7: 0.200166
Documento 9 y 1: 0.152045
Documento 9 y 2: 0.185313
Documento 9 y 3: 0.219752
Documento 9 y 4: 0.154274
Documento 9 y 5: 0.161628
Documento 9 y 6: 0.243025
Documento 9 y 7: 0.208505
Documento 9 y 8: 0.193001
Documento 10 y 1: 0.151732
Documento 10 y 2: 0.29151
Documento 10 y 3: 0.234564
Documento 10 y 4: 0.224381
Documento 10 y 5: 0.164628
Documento 10 y 6: 0.249762
Documento 10 y 7: 0.338264
Documento 10 y 8: 0.379333
Documento 10 y 9: 0.23217
Los dos documentos mas similares son el documento 10 y el documento 8 con una simitud de 0.379333
```
