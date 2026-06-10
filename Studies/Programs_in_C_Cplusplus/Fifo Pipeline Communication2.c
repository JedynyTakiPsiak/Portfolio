#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define FIFO_USER1_TO_USER2 "fifo_user1_to_user2"
#define FIFO_USER2_TO_USER1 "fifo_user2_to_user1"
#define BUFFER_SIZE 1024

void setup_fifo() {
    if (mkfifo(FIFO_USER1_TO_USER2, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo FIFO_USER1_TO_USER2");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(FIFO_USER2_TO_USER1, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo FIFO_USER2_TO_USER1");
        exit(EXIT_FAILURE);
    }
}

void* receive_messages(void* arg) {
    int fd_from_user1 = *(int*)arg;
    char readbuf[BUFFER_SIZE];

    while (1) {
        int read_bytes = read(fd_from_user1, readbuf, sizeof(readbuf) - 1);
        if (read_bytes > 0) {
            readbuf[read_bytes] = '\0';
            printf("\rUser1: %s\nUser2: ", readbuf);
            fflush(stdout);
        }
    }
    return NULL;
}

int main() {
    setup_fifo();

    char writebuf[BUFFER_SIZE];
    int fd_to_user1, fd_from_user1;

    fd_to_user1 = open(FIFO_USER2_TO_USER1, O_RDWR);
    if (fd_to_user1 == -1) {
        perror("open FIFO_USER2_TO_USER1");
        exit(EXIT_FAILURE);
    }
    
    fd_from_user1 = open(FIFO_USER1_TO_USER2, O_RDWR);
    if (fd_from_user1 == -1) {
        perror("open FIFO_USER1_TO_USER2");
        exit(EXIT_FAILURE);
    }

    printf("Admin: Welcome User2 in Two-Way communication via FIFO-pipe. Type 'end' to close chat.\n");

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, &fd_from_user1);

    while (1) {
        printf("User2: ");
        fflush(stdout);
        if (fgets(writebuf, sizeof(writebuf), stdin) == NULL) {
            break;
        }
        writebuf[strcspn(writebuf, "\n")] = '\0';

        if (strcmp(writebuf, "end") == 0) {
            write(fd_to_user1, "Disconnected o7", 15);
            printf("Communication ended.\n");
            break;
        }

        write(fd_to_user1, writebuf, strlen(writebuf) + 1);
    }

    pthread_cancel(recv_thread);
    close(fd_to_user1);
    close(fd_from_user1);
	remove(FIFO_USER1_TO_USER2);
	remove(FIFO_USER2_TO_USER1);
    return 0;
}
