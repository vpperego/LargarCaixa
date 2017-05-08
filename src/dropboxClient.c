#include "../include/dropboxClient.h"

char userid[MAXNAME];
int client_socket;

/* From Assignment Specification
 * Connects the client to the server.
 * host - server address
 * port - server port
 */
int connect_server(char *host, int port)
{
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd;
    
    if( (server = gethostbyname(host)) == NULL) {
        perror("ERROR, no such host: ");
        exit(0);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR opening socket: ");
        exit(0);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting\n");
        exit(0);
    }
    
    char buffer[256];
    strcpy(buffer, userid);
    
    //send userid to server
    send_data(userid, sockfd, strlen(userid) * sizeof(char));
    
    return sockfd;
}

/* From Assignment Specification
 * Synchronizes the directory named "sync_dir_<username>" with the server.
 * The directory is located in the users home.
 */
void sync_client()
{
    
}

/* From Assignment Specification
 * Sends a file with the path "file" to the server.
 * Must be called in order to upload a file.
 * file - path/filename.ext
 */
void send_file(char *file)
{
    char *source = NULL;
    FILE *fp = NULL;
    if ( (fp = fopen(file, "r")) == NULL) {
        perror("ERROR FOPEN: ");
        return;
    }
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            long bufsize = ftell(fp);
            if (bufsize == -1) { /* Error */ }
            
            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));
            
            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }
            
            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            } else {
                source[newLen++] = '\0'; /* Just to be safe. */
            }
            send_data(source, client_socket, newLen * sizeof(char));
        }
        fclose(fp);
    }
    
    
    free(source);
}

/* From Assignment Specification
 * Fetch a file named "file" from the server.
 * Must be called in order to download a file.
 * file - filename.ext
*/
void get_file(char *file)
{
    
}

/* From Assignment Specification
 * Closes the connection to the server
 */
void close_connection()
{
    
}

int main(int argc, char *argv[]) {
    if (argc < CLIENT_ARGUMENTS) {
        fprintf(stderr,"usage %s %s\n", argv[0],ARGUMENTS);
        exit(0);
    }
    //save user name
    strcpy(userid, argv[1]);
    client_socket = connect_server(argv[2],atoi(argv[3]));
    start_client_interface();
    return 0;
}
