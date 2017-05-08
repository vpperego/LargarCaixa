#include "../include/dropboxSharedSocket.h"

//send data with file size
void send_data(char *data, int sockfd, int datalen){
    int n;
    int tmp = htonl(datalen);
    if ( (n = write(sockfd, (char*)&tmp, sizeof(tmp))) < 0 ) {
        perror("ERROR writing to socket: ");
        close(sockfd);
        exit(0);
    }
    if ( (n = write(sockfd, data, datalen)) < 0 ) {
        perror("ERROR writing to socket: ");
        close(sockfd);
        exit(0);
    }
}
