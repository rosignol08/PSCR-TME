#include <iostream>
using namespace std;
int main() {
    int * tab[10];
    for (u_short i=9; i >= 0 ; i--) {
        if (tab[i] - tab[i-1] != 1) {
        cout << "probleme !";
        }
    }

    std::cout << "Hello World!" << std::endl;
    return 0;
}
