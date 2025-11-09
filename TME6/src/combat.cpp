#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "util/rsleep.h"

using namespace std;

struct processus {
    pid_t pid;
    int pv;
};

volatile sig_atomic_t PV = 3;

struct sigaction sa;
//pere vador fils luke en global:
pid_t vador_pid;
pid_t luke_pid;
    
//attaque envoyé a l'adversaire
static pid_t adversaire_actuel;

void handler_attaque(int sig) {
    printf("Processus %d reçoit une attaque!\n", getpid());
    PV = PV - 1;
    //kill(getpid(), SIGUSR1); //renvoie une attaque à l'adversaire
}

void init_handler(){
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = &handler_attaque;
    sigaction(SIGUSR1, &sa, NULL);
    
}

void attaque (pid_t adversaire){
    //configure le handler pour recevoir les attaques
    //sigemptyset(&sa.sa_mask);
    //adversaire_actuel = adversaire;
    //sa.sa_handler = &handler_attaque;
    //sigaction(SIGUSR1, &sa, NULL);
    //init_handler();
    
    //envoie une attaque à l'adversaire
    printf("Processus %d attaque %d\n", getpid(), adversaire);
    kill(adversaire, SIGUSR1);
}

// La phase de défense consiste à désarmer le signal SIGUSR1 en positionnant son action à SIG_IGN ;
// Ensuite le processus s'endort pour une durée aléatoire
void defense(){
    signal(SIGUSR1, SIG_IGN);
    pr::randsleep();
    // Réactive le handler après la défense
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = &handler_attaque;
    sigaction(SIGUSR1, &sa, NULL);    
}

void combat(pid_t adversaire){
    while (PV > 0 ){
        if (PV <= 0) {
            if (getpid() == vador_pid){
                cout << "luke a gagné" << endl;
            } else {
                cout << "vador a gagné" << endl;
            }
            exit(0);
        }
        
        // Vérifier si l'ADVERSAIRE est mort
        if (kill(adversaire, 0) == -1) {
            if (getpid() == vador_pid){
                cout << "vador a gagné" << endl;
            } else {
                cout << "luke a gagné" << endl;
            }
            exit(0);
        }
    
        attaque(adversaire);
        defense();
        //attaque et defense 
        if (getpid() == vador_pid){
            cout << "score : " << PV << " vador" << endl;
        }else{
            cout << "score : " << PV << " luke" << endl;
        }
        if (getpid() == vador_pid) {
            int status;
            pid_t result = waitpid(luke_pid, &status, WNOHANG);
            if (result > 0) {  // Le fils est terminé
                cout << "vador a gagné" << endl;
                exit(0);
            }
        }

    }
    if (getpid() == vador_pid){
        cout << "luke a gagné" << endl;
        waitpid(luke_pid, nullptr, WNOHANG);
    }else if (luke_pid == 0){
        cout << "vador a gagné" << endl;
    }
    exit(0);
}

int main() {
    cout << "Placeholder for combat" << endl;
    
    vador_pid = getpid();
    luke_pid = fork();

    if (luke_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if( luke_pid == 0){
        init_handler();
        cout << "child process (Luke) " << getpid() << endl;
        //sleep(1);
        pr::randsleep();
        combat(vador_pid);
    }
    if( getpid() == vador_pid){
        init_handler();
        cout << "parent process (Vador) " << getpid() << endl;
        pr::randsleep();
        combat(luke_pid);
        waitpid(luke_pid, nullptr, 0);//la fin du fils
    }
    return 0;
}