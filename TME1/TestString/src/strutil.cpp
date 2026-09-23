// strutil.cpp
#include "strutil.h"

namespace pr {

size_t length(const char* s) {
    size_t compteur = 0;
    int i;
    for (i = 0; s[i]; i++){
        compteur ++;
    }
    return compteur;
}

char* newcopy(const char* s) {
    char* copie = new char[length(s)+1];
    for (char* cp= copie ; ; ++cp, ++s){
        *cp = *s;
        if(! *cp){
            break;
        }
    }
    return copie;
}

int compare(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) { //verif que les deux chaines ont le meme caractere ici
            return a[i] - b[i];
        }
        i++;
    }
    return a[i] - b[i];
}

char* newcat(const char* a, const char* b) {
    size_t longueur_a = length(a);
    size_t longueur_b = length(b);
    char* copie = new char[longueur_a + longueur_b + 1];
    size_t i;
    for (i = 0; i < longueur_a; ++i) {
        copie[i] = a[i];
    }
    for (size_t j = 0; j < longueur_b; ++j) {
        copie[longueur_a + j] = b[j];
    }
    copie[longueur_a + longueur_b] = '\0';
    return copie;

}
};
