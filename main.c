/**
 * Build with: gcc main.c -o hmac -lssl -lcrypto
 * Example usage:
 *  ./hmac wordlist.txt sha1hash
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

clock_t begin;
pthread_mutex_t lock;

static int verbose_flag = 0;
static int newlines_flag = 0;
static char *format;

FILE *blob;
char *wordlist;
char *target_hash;
char *all_chars = "0123456789abcdef";
int count = 0;
void (*hash_fun_ptr)(char *, char *);
int found = 0;
int applied_hashing = 1;

static struct option long_options[] = {
    { "verbose",  no_argument,       0,               1  },
    { "format",   required_argument, 0,              'f' },
    { "wordlist", required_argument, 0,              'w' },
    { "target",   required_argument, 0,              't' },
    { "amount",   required_argument, 0,              'a' },
    { "newlines", no_argument,       0,              'n' },
    { "help",     no_argument,       0,              'h' },
    {  0,         0,                 0,               0  }
};

/**
 * Convert the hash to hex and put it in the buffer.
 *
 * @param hash The hash to convert to hex.
 * @param output_buffer The output buffer.
 * @param length The length of the output buffer.
 */
void hash_to_hex(unsigned char *hash, char *output_buffer, int length)
{
    int i = 0;

    for(i = 0; i < length; i++) {
        sprintf(output_buffer + (i * 2), "%02x", hash[i]);
    }

    output_buffer[length * 2] = 0;
}

/**
 * Create an SHA256 hash.
 *
 * @param string The string to hash.
 * @param buffer The output buffer.
 */
void sha256(char *string, char output_buffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);

    // Produce the hex 
    hash_to_hex(hash, output_buffer, SHA256_DIGEST_LENGTH);
}

/**
 * Create an MD5 hash.
 *
 * @param string The string to hash.
 * @param buffer The output buffer.
 */
void md5(char *string, char output_buffer[33])
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, string, strlen(string));
    MD5_Final(hash, &md5);

    // Produce the hex 
    hash_to_hex(hash, output_buffer, MD5_DIGEST_LENGTH);
}

/**
 * Returns the size of the blob.
 * @param char[] The message line.
 */
void manage_read(char msg_line[100])
{
    // Read a line from the blob.
    fgets(msg_line, 100, blob);

    // Return if the end of the line was reached.
    if (msg_line == NULL) {
        return;
    }
}

/**
 * This is the worker callback.
 * @param ptr The input to the worker's callback.
 * 
 */
void *worker_callback(void *ptr)
{
    char msg_line[256];

    pthread_mutex_lock(&lock);
    manage_read(msg_line);
    pthread_mutex_unlock(&lock);

    while (msg_line && found == 0) {
        if (found == 1) {
            pthread_exit(0);
            return 0;
        }

        if (strlen(msg_line) == 0) {
            pthread_mutex_lock(&lock);
            manage_read(msg_line);
            pthread_mutex_unlock(&lock);
            continue;
        }

        // Eliminate the newline at the end.
        if (newlines_flag == 0 && msg_line) {
            msg_line[strlen(msg_line) - 1] = '\0';
        }

        // Allocate the appropriate memory for the 
        char hash[strcmp(format, "sha256") == 0 ? 65 : 33];
        hash_fun_ptr(msg_line, hash);

        // Re-run the hash function on, but on the hash itself this time.
        if (applied_hashing > 1) {
            for (int m = 0; m < applied_hashing - 1; m++) {
                hash_fun_ptr(hash, hash);
            }
        }

        if (verbose_flag != 0) {
            printf("%s -> %s%c", hash, msg_line, newlines_flag == 0 ? '\n' : '\0');
        }

        count++;

        // Compare the produced hash to the target hash.
        if (strcmp(target_hash, hash) == 0) {
            if (newlines_flag != 0) {
                printf("The result is (with a newline): %s", msg_line);
            } else {
                printf("The result is (without a newline): %s\n", msg_line);
            }

            // Get the execution time.
            clock_t end = clock();
            double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

            printf("Found it after %i tries and %f seconds!\n", count, time_spent);

            // Close the blob file.
            if (blob) {
                fclose(blob);
            }

            exit(0);

            // The thing below causes an error when run. Figure dat shit out.

            // Signal the other threads that we're donezo.
            pthread_mutex_lock(&lock);
            found = 1;
            pthread_mutex_unlock(&lock);

            break;
        }

        pthread_mutex_lock(&lock);
        manage_read(msg_line);
        pthread_mutex_unlock(&lock);
    }

    // Exit the thread.
    pthread_exit(0);

    return 0;
}

/**
 * Prints the help message.
 */
void print_help()
{
    printf("hashchcker\n\n");
    printf("Usage:   hashchcker [options]\n\n");
    printf("Options: --wordlist, -w [filename]: The wordlist to check against\n");
    printf("         --format,   -f [hash_fun]: The hash function (md5|sha256)\n");
    printf("         --target,   -t [hash]:     The hash to check\n");
    printf("         --newlines, -n:            Whether to hash with newlines\n");
    printf("         --amount,   -a:            Amount of hashing\n");
    printf("         --help,     -h:            Prints this message\n\n");
}

/**
 * Main entry to the program.
 *
 * @param c The argument flag.
 * @param option_index The index of the option.
 */
void process_args(int c, int option_index)
{
    switch(c) {
        case 0:
            break;
        case 'f':
            format = optarg;

            if (strcmp(format, "sha256") == 0) {
                hash_fun_ptr = &sha256;
            }
            else if (strcmp(format, "md5") == 0) {
                hash_fun_ptr = &md5;
            }
            else {
                printf("Unknown format %s\n", format);
                exit(1);
            }

            break;
        case 'w':
            wordlist = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
            strcpy(wordlist, optarg);
            break;
        case 't':
            target_hash = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
            strcpy(target_hash, optarg);
            break;
        case 'n':
            newlines_flag = 1;
            break;
        case 'a':
            applied_hashing = atoi(optarg);

            if (applied_hashing < 1) {
                applied_hashing = 1;
            }

            break;
        case 'v':
            verbose_flag = 1;
            break;
        case 'h':
            print_help();
            exit(1);
    }
}

/**
 * Main entry to the program.
 *
 * @param argc The argument count.
 * @param argv The arguments list.
 * @return The status code of the program.
 */
int main(int argc, char **argv)
{
    begin = clock();
    int option_index = 0;

    // Get the number of cores.
    long int cores = sysconf(_SC_NPROCESSORS_ONLN);

    printf("Creating %ld threads\n", cores);

    pthread_mutex_init(&lock, NULL);
    int c;

    while ((c = getopt_long(argc, argv, "f:w:t:a:nhv", long_options, &option_index)) != -1) {
        process_args(c, option_index);
    }

    // If something is missing print the help message.
    if (!wordlist || !format || !target_hash) {
        print_help();
        return 1;
    }

    printf("Reading %s and producing %s\n", wordlist, format);

    if (newlines_flag != 0) {
        printf("With newlines");
    }

    if (applied_hashing > 1) {
        printf(" and hashing %i times.", applied_hashing);
    }

    printf("\n");

    FILE *msgs = fopen(wordlist, "r");
    blob = msgs;

    if (msgs == NULL) {
        printf("Invalid wordlist file\n");
        return 1;
    }

    // Create as many threads as there are cores.
    for (int i = 0; i < cores; i++) {
        pthread_t worker;

        // Create the thread.
        pthread_create(&worker, NULL, worker_callback, NULL);
    }

    pthread_exit(NULL);

    return 0;
}

