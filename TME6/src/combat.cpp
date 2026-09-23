#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include "util/rsleep.h"
#include <unistd.h>

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
    //printf("Processus %d reçoit une attaque!\n", getpid());
    PV = PV - 1;
    //kill(getpid(), SIGUSR1); //renvoie une attaque à l'adversaire
}

void handler_parade(int sig) {
    const char msg[] = "coup pare\n";
    write(STDOUT_FILENO, msg, sizeof(msg)-1);
}

void init_handler(){
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = &handler_attaque;
    sigaction(SIGUSR1, &sa, NULL);
    
}

void attaque (pid_t adversaire){
    //config du handler pour recevoir les attaques
    struct sigaction sa_atk;
    sigemptyset(&sa_atk.sa_mask);
    sa_atk.sa_handler = handler_attaque;
    sa_atk.sa_flags = 0;
    sigaction(SIGUSR1, &sa_atk, NULL);

    //attaque
    if (kill(adversaire, SIGUSR1) == -1) {
       //erreur ou victoire
    }
    
    //envoie une attaque à l'adversaire
    //printf("Processus %d attaque %d\n", getpid(), adversaire);
    //kill(adversaire, SIGUSR1);
    pr::randsleep();
}

// La phase de défense consiste à désarmer le signal SIGUSR1 en positionnant son action à SIG_IGN ;
// Ensuite le processus s'endort pour une durée aléatoire
void defense(){
    struct sigaction sa_def;
    sa_def.sa_handler = SIG_IGN;
    sigemptyset(&sa_def.sa_mask);
    sa_def.sa_flags = 0;
    sigaction(SIGUSR1, &sa_def, NULL);
    
    pr::randsleep();
}

void defenseLuke() {
    // 1. Positionner le handler "coup paré" [cite: 61]
    struct sigaction sa;
    sa.sa_handler = handler_parade;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    // 2. Masquer le signal SIGUSR1 [cite: 62]
    // Cela empêche le signal d'interrompre le sommeil immédiatement
    sigset_t mask_bloque, mask_orig;
    sigemptyset(&mask_bloque);
    sigaddset(&mask_bloque, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask_bloque, &mask_orig);

    // 3. Dormir (pendant ce temps, si un signal arrive, il reste "en attente") [cite: 63]
    pr::randsleep();

    // 4. Suspendre pour tester l'attaque [cite: 64]
    // sigsuspend remplace temporairement le masque par un masque vide (ici mask_orig qui ne contient pas USR1)
    // Si une attaque a eu lieu pendant le sleep, le handler s'exécute IMMÉDIATEMENT et sigsuspend rend la main.
    // Sinon, on attend l'attaque.
    sigset_t mask_vide;
    sigemptyset(&mask_vide); 
    sigsuspend(&mask_vide); 

    // 5. Restaurer le masque d'origine [cite: 65]
    sigprocmask(SIG_SETMASK, &mask_orig, NULL);
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
    
        if (getpid() == vador_pid) {
            defense();//vador : défense classique
        } else {
            defenseLuke();//luke défense question 4
        }

        attaque(adversaire);
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