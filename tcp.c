#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "cracker.h"
#include "tcp.h"

pthread_mutex_t lock;

/**
 * Checks if a string contains a valid IPv4.
 *
 * @return Return whether this is a valid IPv4.
 */
int is_valid_ipv4(char *ip)
{
    int num;
    int flag = 1;
    int counter = 0;
    char *p = strtok(ip, ".");

    while (p && flag) {
        num = atoi(p);

        if (num >= 0 && num <= 255 && (counter++ < 4)) {
            flag = 1;
            p = strtok(NULL, ".");

        } else {
            flag = 0;
            break;
        }
    }

    return flag && (counter == 3);
}


/**
 * This function creates a TCP server and starts serving.
 *
 * @return Return the status code.
 */
int create_server()
{
    int socket_desc, client_sock;
    struct sockaddr_in server, client;

    // Try to create the socket.
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc == -1) {
        printf("Could not create socket.\n");
    }

    // Create the sockaddr_in structure.
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("0.0.0.0"); // Or maybe use "INADDR_ANY"?
    server.sin_port = htons(8888);

    // Try to bind the server configuration.
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Binding failed.\n");
        return 1;
    }

    // Start listening on connection.
    listen(socket_desc, 3);

    printf("Setting up TCP server on 0.0.0.0:8888\n");

    int c = sizeof(struct sockaddr_in);
    pthread_t thread_id;

    // Listen in on connections.
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))) {
        // Create a thread to handle the connection.
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0) {
            printf("Could not create handler\n");
            return 1;
        }
    }

    // Accept failed.
    if (client_sock < 0) {
        return 1;
    }

    return 0;
}

/**
 * This will handle connection for each client.
 *
 * @param client_sock The client socket. 
 */
void *connection_handler(void *client_sock)
{
    // Get the socket descriptor.
    int socket = *(int*)client_sock;
    char msg_line[256];

    // Read the next word from the wordlist.
    pthread_mutex_lock(&lock);
    manage_read(msg_line);
    pthread_mutex_unlock(&lock);

    //msg_line[strlen(msg_line) - 1] = '\0';

    // Send the word back to the client.
    write(socket, msg_line, strlen(msg_line));

    // Close the socket.
    close(socket);
}

/**
 * This will call the server to get a new word.
 *
 * @param client_sock The client socket.
 * @param msg_line The buffer to store words into.
 */
void get_next_from_master(char *ip, char msg_line[256])
{
    int client_sock;
    struct sockaddr_in server_addr;
    socklen_t addr_size;

    // Create the socket.
    client_sock = socket(PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;

    // Set the target port number by using htons function to use proper byte order.
    server_addr.sin_port = htons(8888);

    // Set IP address of the master server.
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Initialize all of the bits of the padding field to 0.
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    addr_size = sizeof server_addr;

    // Connect to the server.
    connect(client_sock, (struct sockaddr *)&server_addr, addr_size);

    // Read the response.
    recv(client_sock, msg_line, 256, 0);

    // Trim the end.
    msg_line[strlen(msg_line) - 1] = '\0';
}
