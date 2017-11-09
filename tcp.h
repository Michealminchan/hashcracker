#ifndef TCP_H
#define TCP_H

// Checks if an IP is a valid IPv4.
int is_valid_ipv4(char *);

// This function sets up the server.
int create_server(void);

// The thread TCP connection handler.
void *connection_handler(void *);

#endif
