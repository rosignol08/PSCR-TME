#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>

volatile sig_atomic_t pid_fils_actuel = 0;

const char msg_signal[] = "Signal SIGINT reçu, envoi au processus fils\n";

// Custom mystrdup : allocates with new[], copies string (avoid strdup and free)
char* mystrdup(const char* src);

void sigint_handler(int sig){
    if (pid_fils_actuel > 0) {
        (void)write(STDOUT_FILENO, msg_signal, sizeof(msg_signal) - 1);
        kill(pid_fils_actuel, SIGINT);
    }
}

int main() {
    std::string line;

    signal(SIGINT, sigint_handler);


    while (true) {
        std::cout << "mini-shell> " << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nExiting on EOF (Ctrl-D)." << std::endl;
            break;
        }

        if (line.empty()) continue;

        // Simple parsing: split by spaces using istringstream
        std::istringstream iss(line);
        std::vector<std::string> args;
        std::string token;
        while (iss >> token) {
            args.push_back(token);
        }
        if (args.empty()) continue;

        // Prepare C-style argv: allocate and mystrdup
        char** argv = new char*[args.size()];
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = mystrdup(args[i].c_str());
        }
        //ajout de NULL a la fin de argv
        argv[args.size()] = NULL;
        if(strcmp(argv[0], "exit") == 0){
            //nettoyage argv
            for (size_t i = 0; i < args.size(); ++i) {
                delete[] argv[i];
            }
            delete[] argv;
            break;
        }

        pid_t pid_fils = fork();
        if (pid_fils < 0) {
            perror("fork failed");
            //nettoyage argv
            for (size_t i = 0; i < args.size(); ++i) {
                delete[] argv[i];
            }
            delete[] argv;
            continue;
        }
        else if(pid_fils == 0){
            signal(SIGINT, SIG_DFL);
            execvp(argv[0], argv);
            perror("execvp failed");
            //nettoyage argv
            for (size_t i = 0; i < args.size(); ++i) {
                delete[] argv[i];
            }
            delete[] argv;
            exit(EXIT_FAILURE);
        }
        else{
            //on est dans le pere
            pid_fils_actuel = pid_fils;
            int status;
            if (waitpid(pid_fils, &status, 0) == -1) {
                perror("waitpid failed");
            }
            else {
                pid_fils_actuel = 0;
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    std::cout << "Process " << pid_fils << " exit avec le status " << exit_status << std::endl;
                } else if (WIFSIGNALED(status)) {
                    int term_sig = WTERMSIG(status);
                    std::cout << "Process " << pid_fils << " fini par le signal " << term_sig << std::endl;
                }
            }
        }
        // Your code for fork/exec/wait/signals goes here...

        // cleanup argv allocations
        for (size_t i = 0; i < args.size(); ++i) {
            delete[] argv[i];
        }
        delete[] argv;
    }
    return 0;
}



char* mystrdup(const char* src) {
    if (src == nullptr) return nullptr;
    size_t len = strlen(src) + 1;  // +1 for null terminator
    char* dest = new char[len];
    memcpy(dest, src, len);  // Or strcpy if preferred
    return dest;
}
