/*
   Usage:./httpServer port (E.g. ./httpServer 10000 )
*/
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>  /*DNS?*/

void error(char *msg) {
    perror(msg);
    exit(1);
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
        char *host;
        char temp2[50];
        string = strdup(requestFromClient);

        /*Getting the requested address*/
        string = strstr(string, "/?q=");
        if (string != NULL){
            token = strsep(&string, "=");
            token = strsep(&string, " ");
            host = token;

            //get rid of www
            if(strstr(token, "www.") != NULL) {
                strcpy (temp2, token);
                strtok(temp2, ".");
                host = strtok(NULL, " ");
            }

            printf("Connecting to %s\n", host);

            //turn host name into ip address
            struct hostent *he;
            if ((he = gethostbyname(host)) == NULL) {
                fprintf(stderr, "Could not get host\n");
                continue;
            } else {
                printf("Connecting to %s\n", he);
                printf("Token %s\n", token);
            }

            //create a socket on the proxy server
            int newSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (newSocket < 0) {
                perror("socket() failed");
                exit(0);
            }

            struct sockaddr_in addr_req;
            memset(&addr_req, 0, sizeof (addr_req));
            memcpy(&addr_req.sin_addr, he->h_addr_list[0], he->h_length);
            addr_req.sin_family = AF_INET;
            addr_req.sin_port = htons(80); //connect to port 80


            //attempt connection with assigned socket
            if (connect(newSocket, (struct sockaddr *)&addr_req, sizeof(addr_req)) < 0) {
                fprintf(stderr, "Could not connect to server\n");
                exit(0);
            } else {
                printf("Connected to host!\n");
            }

            //generate request based on passed address
            char *request = (char *)malloc(strlen(token)+23);
            sprintf(request, "GET %s HTTP/1.0\n\n", token);

            //send request to socket
            if (send(newSocket, request, strlen(request), 0) < 0) {
                fprintf(stderr, "[-] Failed to send message\n");
                close(newSocket);
                exit(0);
            }

            //handle response from the server
            char server_r[10000];
            if (recv(newSocket, server_r, 10000, 0) < 0) {
                fprintf(stderr, "[-] Failed to receive message\n");
                close(newSocket);
                exit(0);
            }
            printf("Received From Server: %s\n", server_r);


            /*Sending actual file here*/
            printf("Sending File\n");
            if (write(clientsockfd, server_r, strlen(server_r)) < 0) {
                fprintf(stderr, "ERROR writing to socket\n");
                exit(0);
            }


        } else {
            printf("Nothing to forward\n");

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
        }




        close(clientsockfd);
    }

    return 0;
}
