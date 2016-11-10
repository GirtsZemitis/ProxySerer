/*
   Usage:./httpServer port (E.g. ./httpServer 10000 )
*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

char *giveLengthToHeader(char *str1, int length) {

    char *new_str;
    char str2[20];
    sprintf(str2, "%d", length);
    if ((new_str = malloc(strlen(str1) + strlen(str2) + 1)) != NULL) {
        new_str[0] = '\0';   // ensures the memory is an empty string
        strcat(new_str, str1);
        strcat(new_str, " ");
        strcat(new_str, str2);
        strcat(new_str, "\n\n");
    }
    return new_str;
    /*This is for actual parsing algorythm to make everything more generic and dynamic
     * as of right now didnt saw any requirements to do this in project 1.*/
}

int main(int argc, char *argv[]) {
    int sockfd, clientsockfd; //descriptors rturn from socket and accept system calls
    int portno; // port number
    socklen_t clilen;

    char requestFromClient[256];

    /* sockaddr_in: Structure Containing an Internet Address */
    struct sockaddr_in serv_addr, cli_addr;

    int n;
    /*if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n"
                "Provide port by writing ./httpServer [portNumber]\n");

    }*/

    /*Create a new socket
      AF_INET: Address Domain is Internet
      SOCK_STREAM: Socket Type is STREAM Socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    /* atoi converts from String to Integer */
    portno = 8284;
    serv_addr.sin_family = AF_INET;
    /* for the server the IP address is always the address that the server is running on */
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno); //convert from host to network byte order

    /* Bind the socket to the server address */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* Listen for socket connections. Backlog queue (connections to wait) is 5 */
    listen(sockfd, 5);

    clilen = sizeof(cli_addr);


    while (1) {


        /* accepting client socket */
        clientsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        bzero(requestFromClient, 1024);
        n = read(clientsockfd, requestFromClient, 1024); //BLOCK!!!

        /*printf("Printing Request\n");
        printf(requestFromClient);*/


        char* token;
        char* string;

        string = strdup(requestFromClient);


        string = strstr(string, "/?q=");
        if (string != NULL){
            token = strsep(&string, "=");
            token = strsep(&string, " ");
            printf("%s\n", token);
        } else {
            printf("Nothing to forward\n");
        }

        char *response_200_html =
                "HTTP/1.1 200 OK\n"
                        "Content-Type: text/html\n"
                        "Content-Length: 1000\n"
                        "\n";

        FILE *fp;
        fp = fopen("index.html", "rb");
        int headerSent = 1;

        while (1) {

            unsigned char fileForClient[256] = {0};
            int readFile = fread(fileForClient, 1, 256, fp);
            /* Header flag */


            if (readFile > 0) {
                if (headerSent == 1) {
                    headerSent = 0;
                    printf("Sending Header\n");
                    send(clientsockfd, response_200_html, strlen(response_200_html), 0);
                }
                /*Sending actual file here*/
                printf("Sending File\n");
                write(clientsockfd, fileForClient, readFile);
            }

            if (readFile < 256) {
                if (feof(fp))
                    printf("End of file\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }


        close(clientsockfd);
    }

    return 0;
}
