#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PERMS 0644

struct my_msgbuf {
    long mtype;
    int mnumber;
};

int main(void) {
    struct my_msgbuf buf;
    int msqid;
    int toend;
    key_t key;

    if ((key = ftok("msgq.txt", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, PERMS)) == -1) {
        perror("msgget");
        exit(1);
    }
    printf("Message queue: Ready to receive numbers.\n");

    for (;;) {
        if (msgrcv(msqid, &buf, sizeof(buf.mnumber), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        if (buf.mnumber == 0) {
            printf("Received the end signal.\n");
            break;
        }

        printf("Received number: %d\n", buf.mnumber);
    }

    printf("message queue: done receiving messages.\n");
    system("rm msgq.txt");
    return 0;
}
