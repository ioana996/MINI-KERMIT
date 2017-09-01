#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char** argv) {
    msg r, t;
    SendInitFrame sendInit;
    KermitFrame frame, send_frame;
    int fd;
    char buffer[100];
    int ok = 1;
    int flag = 0;

    init(HOST, PORT);

    while (ok) {
        if (recv_message(&r) < 0) {
            perror("Receive message");
            return -1;
        }
        memset(&frame, 0, sizeof(frame));
        memcpy(&frame, r.payload, r.len);
        unsigned short crc = crc16_ccitt(&frame, sizeof(frame) - 3);
        if (frame.type == 'S' && flag == 0) {
            send_frame.soh = frame.soh;
            send_frame.seq = (frame.seq + 1) % 64;
            send_frame.mark = frame.mark;
            if (frame.check != crc) {
                send_frame.type = 'N';
                flag = 1;
            } else {
                send_frame.type = 'Y';
                memcpy(send_frame.data, &sendInit, sizeof(sendInit));
            }
            send_frame.check = crc16_ccitt(&send_frame, sizeof(send_frame) - 3);
            send_frame.len = sizeof(send_frame) - 2;
            memcpy(t.payload, &send_frame, sizeof(send_frame));
            t.len = sizeof(send_frame);
            send_message(&t);
        } else if (frame.type == 'F') {
            send_frame.soh = frame.soh;
            send_frame.seq = (frame.seq + 1) % 64;
            send_frame.mark = frame.mark;
            if (frame.check != crc) {
                send_frame.type = 'N';
            } else {
                send_frame.type = 'Y';
                memcpy(send_frame.data, frame.data, sizeof(frame.data));
                sprintf(buffer, "recv_%s", frame.data);
                fd = open(buffer, O_RDWR|O_CREAT|O_TRUNC, 0600);
            }
            send_frame.check = crc16_ccitt(&send_frame, sizeof(send_frame) - 3);
            send_frame.len = sizeof(send_frame) - 2;
            memcpy(t.payload, &send_frame, sizeof(send_frame));
            t.len = sizeof(send_frame);
            send_message(&t);
        } else if (frame.type == 'D') {
            send_frame.soh = frame.soh;
            send_frame.seq = (frame.seq + 1) % 64;
            send_frame.mark = frame.mark;
            if (frame.check != crc) {
                send_frame.type = 'N';
            } else {
                send_frame.type = 'Y';
                write(fd, frame.data, frame.len - 5);
            }
            send_frame.check = crc16_ccitt(&send_frame, sizeof(send_frame) - 3);
            send_frame.len = sizeof(send_frame) - 2;
            memcpy(t.payload, &send_frame, sizeof(send_frame));
            t.len = sizeof(send_frame);
            send_message(&t);
        } else if (frame.type == 'Z') {
            if (fd != -1) {
                close(fd);
                fd = -1;
            }
            send_frame.soh = frame.soh;
            send_frame.seq = (frame.seq + 1) % 64;
            send_frame.mark = frame.mark;
            if (frame.check != crc) {
                send_frame.type = 'N';
            } else {
                send_frame.type = 'Y';
            }
            send_frame.check = crc16_ccitt(&send_frame, sizeof(send_frame) - 3);
            send_frame.len = sizeof(send_frame) - 2;
            memcpy(t.payload, &send_frame, sizeof(send_frame));
            t.len = sizeof(send_frame);
            send_message(&t);
        } else if (frame.type == 'B') {
            send_frame.soh = frame.soh;
            send_frame.seq = (frame.seq + 1) % 64;
            send_frame.mark = frame.mark;
            if (frame.check != crc) {
                send_frame.type = 'N';
            } else {
                send_frame.type = 'Y';
                ok = 0;
            }
            send_frame.check = crc16_ccitt(&send_frame, sizeof(send_frame) - 3);
            send_frame.len = sizeof(send_frame) - 2;
            memcpy(t.payload, &send_frame, sizeof(send_frame));
            t.len = sizeof(send_frame);
            send_message(&t);
        } else {
            send_frame.soh = frame.soh;
            send_frame.seq = (frame.seq + 1) % 64;
            send_frame.mark = frame.mark;
            if (frame.check != crc) {
                send_frame.type = 'N';
            } else {
                send_frame.type = 'Y';
                ok = 0;
            }
            send_frame.check = crc16_ccitt(&send_frame, sizeof(send_frame) - 3);
            send_frame.len = sizeof(send_frame) - 2;
            memcpy(t.payload, &send_frame, sizeof(send_frame));
            t.len = sizeof(send_frame);
            send_message(&t);
        }
    }

	return 0;
}
