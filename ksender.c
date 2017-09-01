#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc, char** argv) {
    msg t;
    SendInitFrame sendInit;
    KermitFrame frame, recv_frame;
    int i, fd;
    char buffer[MAX_L];
    int size;

    init(HOST, PORT);

    //Pachet S (Send-Init)
    sendInit.fields[MAXL] = MAX_L;
    sendInit.fields[TIME] = TIME_MS / 1000;
    sendInit.fields[NPAD] = 0x00;
    sendInit.fields[PADC] = 0x00;
    sendInit.fields[EOL] = 0x0D;
    frame.soh = 0x01;
    frame.len = sizeof(sendInit) + 5;
    frame.seq = 0;
    frame.type = 'S';
    memcpy(frame.data, &sendInit, sizeof(sendInit));
    frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
    frame.mark = sendInit.fields[EOL];

    memcpy(t.payload, &frame, sizeof(frame));
    t.len = sizeof(frame);
    send_message(&t);

    msg *y = receive_message_timeout(TIME_MS);
    if (y == NULL) {
        perror("receive error");
    } else {
        memset(&recv_frame, 0, sizeof(struct kermit));
        memcpy(&recv_frame, y->payload, y->len);//retin mesajul primit intr-un frame
        printf("[%s] Got reply with payload: %c\n", argv[0], recv_frame.type);
        while (recv_frame.type != 'Y') {
            frame.seq = (recv_frame.seq + 1) % 64;// in caz de ACK incrementez secventa primita
            frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
            send_message(&t);
            y = receive_message_timeout(TIME_MS);
            if (y == NULL) {
                perror("receive error");
            } else {
                memset(&recv_frame, 0, sizeof(struct kermit));
                memcpy(&recv_frame, y->payload, y->len);//in caz NACK retrimit ultimul pachet
            }
            printf("[%s] Got reply with payload: %c\n", argv[0], recv_frame.type);
        }
    }
    for (i = 1; i < argc; i++) {
        //Pachet F (File Header)
        frame.seq = (recv_frame.seq + 1) % 64;
        frame.type = 'F';
        sprintf((char *) frame.data, "%s", argv[i]);//copiez numele fisierului in campul data
        frame.len = strlen((char *) frame.data) + 5;
        frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
        memcpy(t.payload, &frame, sizeof(frame));
        t.len = sizeof(frame);
        send_message(&t);
        y = receive_message_timeout(TIME_MS);
        if (y == NULL) {
            perror("receive error");
        }
        memset(&recv_frame, 0, sizeof(struct kermit));
        memcpy(&recv_frame, y->payload, y->len);
        while (recv_frame.type != 'Y') {
            frame.seq = (recv_frame.seq + 1) % 64;
            frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
            memcpy(t.payload, &frame, sizeof(frame));
            t.len = sizeof(frame);
            send_message(&t);
            y = receive_message_timeout(TIME_MS);
            if (y == NULL) {
                perror("receive error");
            } else {
                memset(&recv_frame, 0, sizeof(struct kermit));
                memcpy(&recv_frame, y->payload, y->len);
            }
            printf("[%s] Got reply with payload: %c - %s\n", argv[0], recv_frame.type, recv_frame.data);
        }

        // Pachet D (Date)
        fd = open(argv[i], O_RDONLY);
        while ((size = read(fd, buffer, MAX_L)) > 0) {
            frame.seq = (recv_frame.seq + 1) % 64;
            frame.type = 'D';
            frame.len = size + 5;
            memcpy(frame.data, buffer, size);
            frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
            memcpy(t.payload, &frame, sizeof(frame));
            t.len = sizeof(frame);
            send_message(&t);

            y = receive_message_timeout(TIME_MS);
            if (y == NULL) {
                perror("receive error");
            }
            memset(&recv_frame, 0, sizeof(struct kermit));
            memcpy(&recv_frame, y->payload, y->len);
            while (recv_frame.type != 'Y') {
                frame.seq = (recv_frame.seq + 1) % 64;
                frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
                memcpy(t.payload, &frame, sizeof(frame));
                t.len = sizeof(frame);
                send_message(&t);
                y = receive_message_timeout(TIME_MS);
                if (y == NULL) {
                    perror("receive error");
                } else {
                    memset(&recv_frame, 0, sizeof(struct kermit));
                    memcpy(&recv_frame, y->payload, y->len);
                }   
                printf("[%s] Got reply with payload: %c\n", argv[0], recv_frame.type);
            }
        }

        // Pachet Z
        frame.seq = (recv_frame.seq + 1) % 64;
        frame.type = 'Z';
        frame.len = 5;
        frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
        memcpy(t.payload, &frame, sizeof(frame));
        t.len = sizeof(frame);
        send_message(&t);
        y = receive_message_timeout(TIME_MS);
        if (y == NULL) {
            perror("receive error");
        }
        memset(&recv_frame, 0, sizeof(struct kermit));
        memcpy(&recv_frame, y->payload, y->len);
        while (recv_frame.type != 'Y') {
            frame.seq = (recv_frame.seq + 1) % 64;
            frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
            memcpy(t.payload, &frame, sizeof(frame));
            t.len = sizeof(frame);
            send_message(&t);
            y = receive_message_timeout(TIME_MS);
            if (y == NULL) {
                perror("receive error");
            } else {
                memset(&recv_frame, 0, sizeof(struct kermit));
                memcpy(&recv_frame, y->payload, y->len);
            }
            printf("[%s] Got reply with payload: %c\n", argv[0], recv_frame.type);
        }
        close(fd);
    }

    // Pachet B
    frame.seq = (recv_frame.seq + 1) % 64;
    frame.type = 'B';
    frame.len = 5;
    frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
    memcpy(t.payload, &frame, sizeof(frame));
    t.len = sizeof(frame);
    send_message(&t);
    y = receive_message_timeout(TIME_MS);
    if (y == NULL) {
        perror("receive error");
    }
    memset(&recv_frame, 0, sizeof(struct kermit));
    memcpy(&recv_frame, y->payload, y->len);
    while (recv_frame.type != 'Y') {
        frame.seq = (recv_frame.seq + 1) % 64;
        frame.check = crc16_ccitt(&frame, sizeof(frame) - 3);
        memcpy(t.payload, &frame, sizeof(frame));
        t.len = sizeof(frame);
        send_message(&t);
        y = receive_message_timeout(TIME_MS);
        if (y == NULL) {
            perror("receive error");
        } else {
            memset(&recv_frame, 0, sizeof(struct kermit));
            memcpy(&recv_frame, y->payload, y->len);
        }
        printf("[%s] Got reply with payload: %c\n", argv[0], recv_frame.type);
    }

    return 0;
}
