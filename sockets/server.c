#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

// realigning some variables
#define QSIZE          5
#define BSIZE          256
#define SOCKET_ADDRESS "mysock"
#define UNIX_PATH_MAX  108

/*
 * Convert a null-terminated sting (one whose end is denoted by a byte
 * containing '\0') to all upper case letters by starting at the
 * beginning and going until the null byte is found.
 */
void convert_string (char *cp)
{
    char *currp;	// pointer to current position in the input string 
    int   c;      // return value of toupper is the converted letter 

    for (currp = cp; *currp != '\0'; currp++) {
        c = toupper (*currp);
        *currp = (char) c;
    }
}

int main(int argc, char *argv[])
{
    // more realignment
    int    handshake_sockfd, session_sockfd, ret;
    struct sockaddr_un saun;
    char   buf[BSIZE];
    
    // filling in the struct
    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, SOCKET_ADDRESS);

    // initialize the socket
    handshake_sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (handshake_sockfd < 0) {
        fprintf(stderr, "\nError Opening Socket, ERROR#%d\n", errno);
        return EXIT_FAILURE;
    }

    unlink(SOCKET_ADDRESS);

    ret = bind(handshake_sockfd, (struct sockaddr *) &saun, sizeof(saun));
    if (ret < 0) {
        fprintf(stderr, "\nError Binding Socket, ERROR#%d\n", errno);
        return EXIT_FAILURE;
    }

    // listen for an incoming connection, throw error if none found
    ret = listen(handshake_sockfd, QSIZE);
    if (ret < 0) {
        fprintf(stderr, "\nError Listening on Socket, ERROR#%d\n", errno);
        return EXIT_FAILURE;
    }

    // found one? great! lets accept
    session_sockfd = accept(handshake_sockfd, NULL, NULL);
    if (session_sockfd < 0) {
        fprintf(stderr, "\nError Accepting Socket, ERROR#%d\n", errno);
        return EXIT_FAILURE;
    }

    // The meat of the program.
    // Assuming we have successful connection, this will output the message 
    // to the socket we set up. 
    while (read(session_sockfd, buf, BSIZE) > 0) {
        printf("RECEIVED:\n%s", buf);
        convert_string(buf);
        write(session_sockfd, buf, BSIZE);
        printf("SENDING:\n%s\n", buf);
    }


    close(session_sockfd);
    close(handshake_sockfd);
}
