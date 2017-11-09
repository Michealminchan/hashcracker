#ifndef CRACKER_H
#define CRACKER_H

// Manages the read.
void manage_read(char[256]);

// This is the thread callback for the worker.
void *worker_callback(void *);

// Get the next word from the master.
void get_next_from_master(char *, char[256]);

#endif
