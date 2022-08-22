/**************************************************************************
 *                                                                        *
 *  Catapult(R) Machine Learning Reference Design Library                 *
 *                                                                        *
 *  Software Version: 1.5                                                 *
 *                                                                        *
 *  Release Date    : Fri Oct 29 16:53:36 PDT 2021                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 1.5.0                                               *
 *                                                                        *
 *  Copyright 2021 Siemens                                                *
 *                                                                        *
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
#pragma once

#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#ifdef VSIM
#include <svdpi.h>
#include "dpi_functions.h"
#endif
#ifdef VELOCE
#include <svdpi.h>
#include "tbxbindings.h"
#endif

#define KEY_BUFFER_SIZE 2000

static int terminal_socket;
static int recv_terminal_socket;
static struct sockaddr_in socket_addr;

static unsigned char key_buffer[KEY_BUFFER_SIZE];
static int key_count;

static FILE *lf = NULL;

static int create_terminal_process(int socket_number)
{
    int r;
    char socket_string[20];
    char terminal_emulator_command[1000];
    char *args[] = {(char *) "/usr/bin/xterm", 
                    (char *) "-fg", (char *) "green", 
                    (char *) "-bg", (char *) "black", 
                    (char *) "-T", (char *) "Catapult Platform Virtual Console", 
                    (char *) "-e", terminal_emulator_command, socket_string, 0};

    r = fork();

    printf("Fork returned %d \n", r);

    if (r == -1) {
        fprintf(stderr, "Unable to create child process for terminal \n");
        perror("Terminal");
        return 1;
    }

    if (r != 0) {
       return 0;
    }

    // from here on down we are in the child process

    sprintf(terminal_emulator_command, "./terminal_emulator");

    sprintf(socket_string, "%d", socket_number);

    r = execv(args[0], args);
    if (r == -1) { // or if you get here
        printf("Failure to launch terminal process \n");
        perror("Terminal");
        return 1;
    }

    fprintf(stderr, "You should never see this \n");
    return 0;
}


int send_char_to_terminal(unsigned char out_char)
{
    unsigned char buffer[2];
    int r;

    buffer[0] = out_char;

    if (lf) { fprintf(lf, "%c", out_char); fflush(lf); }
    r = sendto(terminal_socket, buffer, 1, 0, (struct sockaddr *) &socket_addr, sizeof(struct sockaddr));
    if (r == -1) {
        fprintf(stderr, "Unable to send character to terminal \n");
        perror("Terminal");
        return 1;
    }
    return 0;
}


unsigned char terminal_key_ready(void)
{
    return key_count;
}


static unsigned char get_sync_char(void)
{
    while (key_count < 1) usleep(100);
    key_count--;
}


unsigned char get_key_from_terminal(void)
{
    int i;
    unsigned char return_value;

    if (key_count == 0) return (char) 0;

    return_value = key_buffer[0];

    for (i=0; i<key_count-1; i++) {
        key_buffer[i] = key_buffer[i+1];
    }

    key_count--;

    return return_value;
}


static void *get_char_thread(void *not_used)
{
    char buffer[100];
    int r;

    while (1) {
        r = recv(recv_terminal_socket, buffer, sizeof(buffer), 0);
        if (r == -1) {
            fprintf(stderr, "Unable to get character from terminal \n");
            perror("Terminal");
            return NULL;
        }

        if (key_count < KEY_BUFFER_SIZE) key_buffer[key_count++] = buffer[0];
            // drops keyclicks if buffer is full

    }
    return NULL;
}

static void shut_down_terminal_process(void)
{
    if (lf) fclose(lf);
    send_char_to_terminal(0x04);
}

int start_external_terminal(void)
{
    int r;
    int socket_number;
    char hostname[100];
    struct hostent *hp;
    struct sockaddr_in recv_socket_addr;
    pthread_t recv_thread_handle;
    char *logfile_name;

    srand(time(NULL));

    do {
        socket_number = rand() & 0xFFFE;
    } while (socket_number<1024);

    printf("Socket address (so) is %d \n", socket_number);

    terminal_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (terminal_socket == -1) {
        fprintf(stderr, "Unable to create terminal socket \n");
        perror("Terminal");
        return 1;
    }

    r = gethostname(hostname, sizeof(hostname));
    if (r == -1) {
        fprintf(stderr, "Unable to get hostname \n");
        perror("Terminal");
        return 1;
    }

    hp = gethostbyname(hostname);
    if (hp == 0) {
        fprintf(stderr, "Unable to get host address \n");
        perror("Terminal");
        return 1;
    }

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = ((struct in_addr *)( hp->h_addr))->s_addr;
    socket_addr.sin_port = htons(socket_number);

    // create socket in the other direction
    recv_socket_addr.sin_family = AF_INET;
    recv_socket_addr.sin_addr.s_addr = INADDR_ANY;
    recv_socket_addr.sin_port = htons(socket_number+1);

    recv_terminal_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (recv_terminal_socket == -1) {
        printf("Unable to create output terminal socket \n");
        perror("terminal_emulator");
        return -1;
    }

    r = bind(recv_terminal_socket, (struct sockaddr *) &recv_socket_addr, sizeof(recv_socket_addr));
    if (r == -1) {
        printf("Unable to bind output terminal socket \n");
        perror("terminal_emulator");
        return -1;
    }

    r = create_terminal_process(socket_number);
    if (r) {
        return r;
    }

    key_count = 0;

    r = pthread_create(&recv_thread_handle, NULL, get_char_thread, NULL);
    if (r != 0) {
        fprintf(stderr, "terminal_emulator: Unable to create thread for input stream \n");
        perror("terminal_emulator");
        return r;
    }
 
    get_sync_char();

    printf("Start terminal called! \n");

    logfile_name = getenv("TERMINAL_DEBUG_LOG");

    if (logfile_name) {
       lf = fopen(logfile_name, "w");
       if (lf == NULL) {
           printf("Unable to open %d for writing\n", logfile_name);
           perror("Terminal");
           printf("logging is disabled \n");
       } else {
           printf("Terminal output will be logged to %s \n", logfile_name);
       }
    }
    return 0;
}

class terminal {
public:
#ifdef GUI
   terminal(void)
   {
      start_external_terminal();
   }

   ~terminal(void)
   {
      shut_down_terminal_process();
   }

   void send_char(unsigned char c)
   {
      send_char_to_terminal(c);
   }

   bool key_ready(void)
   {
      return terminal_key_ready();
   }

   unsigned char get_key(void)
   {
      return get_key_from_terminal();
   }
#else 
   terminal (void) {}
   ~terminal(void) {}
   void send_char(unsigned char c) { fprintf(stderr, "%c", c); fflush(stderr); }
   bool key_ready(void) { return false; }
   unsigned char get_key(void) { return 0; }
#endif
};
