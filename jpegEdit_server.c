#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define HOSTNAME "www.hostname.com"
#define PORT 50175
#define MAXSIZE 2 * 1024 * 1024 //2 MB

int main(int argc, const char * argv[]) {

    int sockfd = 0, connfd, c_len = 0, h_len = 0, rv, f_size, len;
    struct sockaddr_in addr;
    char f_buffer[MAXSIZE] = {0};
    char lil_buffer[1024] = {0};
    const char *filename;
    
    if(argc != 1)
    {
        printf("Error with arguments\n");
    }
    
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("Error initializing socket\n");
        return 1;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    int b = 0;
    
    b = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if(b < 0)
    {
        perror("Error setting up port\n");
        return 1;
    }
    
    //Listen
    int l = 0;
    
    l = listen(sockfd, 1);
    puts("Listening...");//debug
    
    if(l < 0)
    {
        perror("Error listening for port");
        return 1;
    }
    
    struct sockaddr_in cliaddr;
    
    connfd = accept(sockfd, (struct sockaddr *)&cliaddr, (uint32_t*)sizeof(struct sockaddr_in));
                    
    if(connfd < 0)
    {
        perror("Error accepting connection");
        return 1;
    }
                    
    if (recv(sockfd, lil_buffer, sizeof(lil_buffer), 0) < 0)
    {
        fprintf(stderr, "error receiving size from server");
    }
                    
        printf("file size response: %s\n", lil_buffer); //Debug
                    
                    
                    
                    
}
