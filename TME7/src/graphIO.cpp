// main.cpp
#include "BinIO.h"
#include "EdgeListIO.h"

/*
Si le noeud est déjà dans la map, retournez son offset.
- Sinon, noter la position actuelle de fin de fichier; ce sera l’offset du nouveau noeud. Ajoutez
l’offset à la map tout de suite, avant toute récursion.
- Créer un espace suffisant pour le noeud en fin de fichier. Plusieurs stratégies possibles : lseek
+ une écriture, ftruncate, etc.
- Écrivez le header placeholder.
- Pour chaque enfant : récursion pour obtenir l’offset enfant, puis l’écrire au bon endroit
(lseek+write ou pwrite).
- Retournez l’offset.
*/

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " [c|d] input output" << std::endl;
    return 1;
  }

  std::string mode = argv[1];
  if (mode == "c") {
    Graph graph = parseEdgeList(argv[2]);
    writeBin(graph, argv[3]);
  } else if (mode == "d") {
    Graph graph = parseBin(argv[2]);
    writeEdgeList(graph, argv[3]);
  } else {
    std::cerr << "Invalid mode: " << mode << std::endl;
    return 1;
  }

  return 0;
}