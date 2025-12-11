#include "pipe.h"

#include <fcntl.h>
#include <unistd.h>
#include <limits.h> // For PIPE_BUF
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <signal.h>
#include <algorithm>

namespace pr {

// Place this in shared memory !    
struct PipeShm {
    char buffer[PIPE_BUF];
    size_t head; // write position
    size_t tail; // read position
    size_t count; // number of bytes in the pipe
    size_t nbReaders; // number of readers
    size_t nbWriters; // number of writers
    sem_t  semMut; //semaphore semMut
    sem_t  canRead; //semaphore canRead
    sem_t  canWrite; //semaphore canWrite
};

// This is per-process handle, not in shared memory
struct Pipe {
    PipeShm *shm; // pointer to shared memory
    int oflags; // O_RDONLY or O_WRONLY
};

int pipe_create(const char *name) {
    // Construct shared memory name
    char shm_name[256];
    // add a '/' at the beginning for shm_open
    snprintf(shm_name, sizeof(shm_name), "/%s", name);
    // Try to create shared memory with O_CREAT|O_EXCL
    int fd = shm_open(shm_name, O_CREAT|O_EXCL, 0666);
    if(fd == -1){
        perror("cassé");
        exit(1);
    }
    size_t taille_pipe_shm = sizeof(PipeShm);
    //shm_unlink(name);
    // Set size of shared memory
    ftruncate(fd  , taille_pipe_shm);
    
    // Map the shared memory
    void* addresse = mmap(NULL,taille_pipe_shm, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    // Initialize the PipeShm structure
    // memset(shm, 0, sizeof(PipeShm));
    PipeShm *shm = (PipeShm*) addresse;
    // Including semaphores
    
    if(sem_init(&shm->semMut,1,1 ) == -1){
        perror("cassé");
        exit(1);
    }
    if(sem_init(&shm->canRead,1,0 ) == -1){
        perror("cassé");
        exit(1);
    }
    if(sem_init(&shm->canWrite,1,taille_pipe_shm ) == -1){
        perror("cassé");
        exit(1);
    }

    // Unmap and close (setup persists in shared memory)
    //unlink(shm_name);
    munmap(addresse, taille_pipe_shm);
    close(fd);
    
    
    return 0;
}

Pipe * pipe_open(const char *name, int oflags) {
    // Construct shared memory name
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/%s", name);
    
    // Open shared memory (without O_CREAT)
    int fd = shm_open(name, O_RDWR, 0666);
    // Map the shared memory
    PipeShm* shm = (PipeShm*) mmap(nullptr,sizeof(PipeShm),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    // Can close fd after mmap
    close(fd);
    // Increment nbReaders or nbWriters
    if(oflags == O_RDONLY){
        shm->nbReaders++;
    }
    if(oflags == O_WRONLY){
        shm->nbWriters++;
    }

    // Create and return Pipe handle
    Pipe *handle = new Pipe();
    handle->shm = shm;
    handle->oflags = oflags;
    return handle;
}

ssize_t pipe_read(Pipe *handle, void *buf, size_t count) {
    if (handle == nullptr || handle->oflags != O_RDONLY) {
        errno = EBADF;
        return -1;
    } else if (count > PIPE_BUF) {
        errno = EINVAL;
        return -1;
    }
    size_t* zero = 0;
    // wait until some data available or no writers
    while (true) {
        sem_wait(&handle->shm->canRead); // attendre un signal
        sem_wait(&handle->shm->semMut); // entrer en section critique
        if (&handle->shm->nbWriters == 0 || &handle->shm->count > zero)
        break; // condition satisfaite, sortir de la boucle
        else
        sem_post(&handle->shm->semMut); // condition non satisfaite, relâcher le mutex et attendre
    }
    // ici : condition OK est vrai, et on est en section critique (mutex acquis)
    //... section critique ...
    
    //sem_post(&handle->shm->canRead); // signaler si besoin signal EOF

    // Check if pipe is empty and no writers : EOF
    if(handle->shm->count == 0 && handle->shm->nbWriters == 0){
        //eof
        //signaler le lecteur suivant
        sem_post(&handle->shm->canRead);
        sem_post(&handle->shm->semMut); // relâcher le mutex
        return 0;
    }
    // Read min(count, shm->count) bytes
    PipeShm *shm = handle->shm;
    size_t to_read = std::min(count, shm->count);
    char *output = (char *)buf;
    
    // Handle circular buffer: may need to copy in two parts
    size_t first_chunk = std::min(to_read, PIPE_BUF - shm->tail);
    memcpy(output, &shm->buffer[shm->tail], first_chunk);
    
    if (first_chunk < to_read) {
        // Wrap around to beginning of buffer
        memcpy(output + first_chunk, &shm->buffer[0], to_read - first_chunk);
    }
    
    shm->tail = (shm->tail + to_read) % PIPE_BUF;
    shm->count -= to_read;
    
    // warn other readers/writers if needed
    if (handle->shm->count > 0){
            sem_post(&handle->shm->canRead);
        }
    sem_post(&handle->shm->semMut);
    return to_read;
}

ssize_t pipe_write(Pipe *handle, const void *buf, size_t count) {
    if (handle == nullptr || handle->oflags != O_WRONLY) {
        errno = EBADF;
        return -1;
    } else if (count > PIPE_BUF) {
        errno = EINVAL;
        return -1;
    }
    
    PipeShm *shm = handle->shm;
    
    // wait until *enough* space available or no readers
    while (true) {
        sem_wait(&handle->shm->canRead); // attendre un signal
        sem_wait(&handle->shm->semMut); // entrer en section critique
        if ( PIPE_BUF - shm->count >= count && shm->nbReaders > 0)
        break; // condition satisfaite, sortir de la boucle
        else
        sem_post(&handle->shm->semMut); // condition non satisfaite, relâcher le mutex et attendre
    }
    // Check if no readers => SIGPIPE
    if(shm->nbReaders == 0){
        raise(SIGPIPE);
        return -1;
    }
    // Write count bytes
    const char *input = (const char *)buf;
    
    // Handle circular buffer: may need to copy in two parts
    size_t first_chunk = std::min(count, PIPE_BUF - shm->head);
    memcpy(&shm->buffer[shm->head], input, first_chunk);
    
    if (first_chunk < count) {
        // Wrap around to beginning of buffer
        memcpy(&shm->buffer[0], input + first_chunk, count - first_chunk);
    }
    
    shm->head = (shm->head + count) % PIPE_BUF;
    shm->count += count;
    
    // warn other readers/writers if needed
    sem_post(&handle->shm->canRead); // signaler aux lecteurs qu'il y a des données à lire
    sem_post(&handle->shm->semMut); // relachement du mutex
    return count;
}

int pipe_close(Pipe *handle) {
    if (handle == nullptr) {
        errno = EBADF;
        return -1;
    }
    
    PipeShm *shm = handle->shm;
    /*
    
    Il doit faire la réciproque de pipe_open.Il faut décrémenter le nombre de
    lecteurs ou d’écrivains selon le mode d’ouverture. Si on est le dernier lecteur ou écrivain, assurez vous de notifier les
    process potentiellement bloqués (e.g. read doit maintenant rendre 0 au lieu de bloquer, write doit lever SIGPIPE).
    Enfin on libère les ressources allouées dans pipe_open (avec des munmap/close/delete, mais pas de unlink)

    */
    // Decrement reader or writer count
    // Warn other process as needed (e.g. if last reader/writer)
    int oflags = handle->oflags;
    if(oflags == O_RDONLY){
        shm->nbReaders--;
    }
    if(oflags == O_WRONLY){
        shm->nbWriters--;
    }
    
    // Unmap memory
    munmap(shm, sizeof(PipeShm));
    // Free handle
    delete handle;
    
    return 0;
}

int pipe_unlink(const char *name) {
    // Construct shared memory name
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/%s", name);
    
    // Unlink shared memory (this also destroys the embedded semaphores)
    /*
    Il doit détruire le segment de mémoire partagée. On ne se souciera pas
    de synchronisations. Par contre, sous Linux, il est recommandé de sem_destroy les sémaphores; il faut donc
    shm_open/mmap le segment une dernière fois
    */
    PipeShm* shm = (PipeShm*) shm_open(shm_name,O_RDWR, 0666);
    //for (int i = 0; i<shm->)
    sem_destroy(&shm->semMut);
    sem_destroy(&shm->canRead);
    sem_destroy(&shm->canWrite);
    return 0;
}

} // namespace pr