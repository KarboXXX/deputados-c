#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct memory { 
    unsigned char stat;
    char * msg[512];
};

int shmid;
key_t key;
struct memory* shmptr;

enum depu_status {
    seen = 0, bnew = 1, lnew = 2
};

void handle_interrupt(int sig) {
    printf("\rExiting with code %d...\n", sig);
    fflush(stdin);
    shmdt(shmptr);
    exit(sig);
}

/// @brief Destroys shared memory pool stored in `shmptr` using `shmctl`
void destroy_mem_pool() {
    printf("destroying shared memory pool...\n");
    shmdt(shmptr);

    // destroy the shared memory
    shmctl(shmid, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char ** argv) {
    char* choice;
    size_t shmem_size = sizeof(struct memory);
    printf("size of shared memory for allocation: %ld\n", shmem_size);
    printf("voce quer ser o bolsonaro(b), o lula(l) ou o server(s)? ou pretende excluir a shared memory pool (d)? ");
    scanf("%s", &choice);

    // ftok to generate unique key
    key = ftok("deputados", 65);
 
    // shmget returns an identifier in shmid
    shmid = shmget(key, shmem_size, 0666 | IPC_CREAT);
    if (shmid < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }
 
    // shmat to attach to shared memory
    shmptr = shmat(shmid, (void*)0, 0); /* attach */
    if ((int) shmptr == -1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }

    signal(SIGINT, handle_interrupt); // handle Ctrl+C

    size_t message_buffer_size = 512;
    char * message_input[512];
    if (message_input == NULL) {
        printf("*** insufficient memory space for message buffer. ***\n");
        exit(1);
    }

    if (strcmp(&choice, "b") == 0) {

        while (true) {
            printf("enviar mensagem como bolsonaro: ");
            fgets(message_input, message_buffer_size, stdin);
            // scanf("%s", &message_input);

            strcpy(shmptr->msg, message_input);

            shmptr->stat = bnew;
        }
        return 0;
    }

    if (strcmp(&choice, "l") == 0) {
        while (true) {
            printf("enviar mensagem como lula: ");
            fgets(message_input, message_buffer_size, stdin);
            // scanf("%s", &message_input);

            strcpy(shmptr->msg, message_input);
            shmptr->stat = lnew;
        }

        return 0;
    }

    if (strcmp(&choice, "s") == 0) {
        bool bolso_sent = false;
        bool lula_sent = false;

        shmptr->stat = 0;

        printf("ultima mensagem salva: %s\n", shmptr->msg);

        while (true) {
            bolso_sent = (shmptr->stat == bnew);
            lula_sent = (shmptr->stat == lnew);

            // printf("... %s | status: %d\n", &shmptr->bolso_msg, shmptr->stat);
            // fflush(stdout);
            
            if (bolso_sent)
                printf("bolsonaro: %s", shmptr->msg);
            
            if (lula_sent)
                printf("lula: %s", shmptr->msg);

            shmptr->stat = seen;
            usleep(10000);
        }

        return 0;
    }
    if (strcmp(&choice, "d") == 0) {
        destroy_mem_pool();
        return 0;
    }
    
    printf("\r*** no valid choice was chosen. ***\n");
    shmdt(shmptr);
    return 1;
}