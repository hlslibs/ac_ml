/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Math Library                                       *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Fri Oct 29 16:53:36 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.5.0                                               *
 *                                                                        *
 *  Copyright , Mentor Graphics Corporation,                     *
 *                                                                        *
 *  All Rights Reserved.                                                  *
 *  
 **************************************************************************
 *  Licensed under the Apache License, Version 2.0 (the "License");       *
 *  you may not use this file except in compliance with the License.      * 
 *  You may obtain a copy of the License at                               *
 *                                                                        *
 *      http://www.apache.org/licenses/LICENSE-2.0                        *
 *                                                                        *
 *  Unless required by applicable law or agreed to in writing, software   * 
 *  distributed under the License is distributed on an "AS IS" BASIS,     * 
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or       *
 *  implied.                                                              * 
 *  See the License for the specific language governing permissions and   * 
 *  limitations under the License.                                        *
 **************************************************************************
 *                                                                        *
 *  The most recent version of this package is available at github.       *
 *                                                                        *
 *************************************************************************/
#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>

static struct sockaddr_in keyboard_socket_addr;
static int keyboard_socket;
static int screen_socket;
static int bail_out;

void make_raw(void)
{
    struct termios terminal_attributes;
    int r;

    r = tcgetattr(STDIN_FILENO, &terminal_attributes);
    if (r != 0) {
        fprintf(stderr, "tcgetattr() failed: %d \n", r);
        perror("terminal_emualtor");
        bail_out = 1;
        return;
    }

    cfmakeraw(&terminal_attributes);

    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_attributes);
    if (r != 0) {
        fprintf(stderr, "tcsetattr() failed: %d \n", r);
        perror("terminal_emualtor");
        bail_out = 1;
        return;
    }

    return;
}


int open_screen_socket(int screen_socket_number)
{
    int addrlen;
    int r;
    struct sockaddr_in scr_sock_addr;

    //printf("Socket address (screen) is %d \n", screen_socket_number);

    scr_sock_addr.sin_family = AF_INET;
    scr_sock_addr.sin_addr.s_addr = INADDR_ANY;
    scr_sock_addr.sin_port = htons(screen_socket_number);

    screen_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (screen_socket == -1) {
        fprintf(stderr, "Unable to create output terminal socket \n");
        perror("terminal_emulator");
        return -1;
    }

    r = bind(screen_socket, (struct sockaddr *) &scr_sock_addr, sizeof(scr_sock_addr));
    if (r == -1) {
        fprintf(stderr, "Unable to bind output terminal socket \n");
        perror("terminal_emulator");
        return -1;
    }
 
    return 0;
}

int open_keyboard_socket(int keyboard_socket_number)
{
    int addrlen;
    int r;
    char hostname[100];
    struct hostent *hp;

    //printf("Socket address (in) is %d \n", keyboard_socket_number);

    keyboard_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (keyboard_socket == -1) {
        fprintf(stderr, "terminal_emulator: Unable to create keyboard terminal socket \n");
        perror("terminal_emulator");
        return -1;
    }

    r = gethostname(hostname, sizeof(hostname));
    if (r == -1) {
        fprintf(stderr, "terminal_emulator: Unable to get hostname \n");
        perror("terminal_emulator");
        return -1;
    }

    hp = gethostbyname(hostname);
    if (hp == 0) {
        fprintf(stderr, "terminal_emulator: Unable to get host address \n");
        perror("terminal_emulator");
        return -1;
    }

    keyboard_socket_addr.sin_family = AF_INET;
    keyboard_socket_addr.sin_addr.s_addr = ((struct in_addr *)( hp->h_addr))->s_addr;
    keyboard_socket_addr.sin_port = htons(keyboard_socket_number);
 
    return 0;
}


void screen_thread()
{
    int r;
    int i;
    char buffer[100];

    while (!bail_out) {
        r = recv(screen_socket, buffer, sizeof(buffer), 0);
        if (r == -1) {
            fprintf(stderr, "terminal_emulator: Unable to read socket \n");
            perror("terminal_emulator");
            bail_out = 1;
            return;
        }
        for (i = 0; i < r; i++) {
            if (buffer[i] == 0x04) {
                bail_out = 1;
            }
            putchar(buffer[i]);
        }
        fflush(stdout);
    }
}


void keyboard_thread()
{
    char buffer[100];
    int r;

    buffer[0] = 0xFF;

    r = sendto(keyboard_socket, buffer, 1, 0, (struct sockaddr *) &keyboard_socket_addr, sizeof(struct sockaddr));
    if (r == -1) {
        fprintf(stderr, "terminal_emulator: Unable to send sync character to terminal \n");
        perror("terminal_emulator");
        bail_out = 1;
        return; 
    }

    while (!bail_out) {
	r = getchar();
        if (r == EOF) {
            fprintf(stderr, "terminal_emulator: Unable to get character from getchar() \n");
            perror("terminal_emulator");
            bail_out = 1;
            return; 
        }
        buffer[0] = (unsigned char) r;
        r = sendto(keyboard_socket, buffer, 1, 0, (struct sockaddr *) &keyboard_socket_addr, sizeof(struct sockaddr));
        if (r == -1) {
            fprintf(stderr, "terminal_emulator: Unable to send character to terminal \n");
            perror("terminal_emulator");
            bail_out = 1;
            return; 
        }
    }
}

main(int argc, char **argv)
{
    int r;
    int i;
    char buffer[100];
    int socket_number;
    pthread_t keyboard_thread_handle;
    pthread_t screen_thread_handle;

    if (argc != 2) {
        fprintf(stderr, "terminal_emulator: Usage: %s <socket number> \n", argv[0]);
        return;
    }

    make_raw();
    bail_out = 0;

    socket_number = atoi(argv[1]);

    open_screen_socket(socket_number);
    open_keyboard_socket(socket_number + 1);

    r = pthread_create(&keyboard_thread_handle, NULL, keyboard_thread, NULL);
    if (r != 0) {
        fprintf(stderr, "terminal_emulator: Unable to create thread for keyboard stream \n");
        perror("terminal_emulator");
        return;
    }

    r = pthread_create(&screen_thread_handle, NULL, screen_thread, NULL);
    if (r != 0) {
        fprintf(stderr, "terminal_emulator: Unable to create thread for screen stream \n");
        perror("terminal_emulator");
        return;
    }

    pthread_join(screen_thread_handle, NULL);
    pthread_cancel(keyboard_thread_handle);  // need to wake thread up from getchar() call

    pthread_join(keyboard_thread_handle, NULL);

    fprintf(stderr, "terminal_emulator: Exiting... \n");
}


