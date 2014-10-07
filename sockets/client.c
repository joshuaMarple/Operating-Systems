#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BSIZE          256
#define NSTRS          3
#define SOCKET_ADDRESS "mysock"
#define UNIX_PATH_MAX  108 // going to realign these so they look cleaner.

void convert_string (char *cp)
{
    // why were we using a multiline comment for a single line comment..?
    char *currp;    // pointer to current position in the input string 
    int   c;      // return value of toupper is the converted letter 

    for (currp = cp; *currp != '\0'; currp++) {
        c = toupper (*currp);
        *currp = (char) c;
    }
}

int main(int argc, char *argv[])
{
    // moved this inside the main, cause non static global vars are bad, mmk?
    /*
    * This is the set of strings we will send through the socket for
    * conversion
    */
    char *strs[NSTRS] = {
        "this is the first string from the client\n",
        "this is the second string from the client\n",
        "this is the third string from the client\n"
    };

    // realign these bad boys too
    int      sockfd, ret, i;
    struct   sockaddr_un saun;
    char     buf[BSIZE];
        
    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, SOCKET_ADDRESS);
    
    // create the socket, make sure it doesn't fail
    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "\nError Opening Socket, ERROR#%d\n", errno);
        return EXIT_FAILURE;
    }

    // attempt connection (be sure to start server first)
    ret = connect(sockfd, (struct sockaddr *) &saun, sizeof(saun));
    if (ret < 0) {
        fprintf(stderr, "\nError Connecting Sockets, ERROR#%d\n", errno);
        return EXIT_FAILURE;
    }

    // The meat of the program.
    // Assuming we have successful connection, this will output the message 
    // to the socket we set up. 
    for (i = 0; i < NSTRS; i++) {
        printf("SENDING:\n%s", strs[i]);
        write(sockfd, strs[i], BSIZE);
        read(sockfd, buf, BSIZE);
        convert_string(buf);
        printf("RECEIVED:\n%s\n", buf);
    }

    close(sockfd);
}
