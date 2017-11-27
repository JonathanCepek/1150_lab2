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

    int sockfd = 0, c_len = 0, h_len = 0, rv, f_size, len;
    struct addrinfo hints, *servinfo, *p;
    char f_buffer[MAXSIZE] = {0};
    const char *filename;
    
    if(argc == 2)
    {
        filename = argv[1];
    }
    else
    {
        printf("\nUsage:\n");
        printf("JPEGEdit [filename] \nNote: use JPEG formatted files.\n");
    }
    
    //File handling
    FILE *fP;
    fP = fopen(filename, "r");
    
    fseek(fp, 0L, SEEK_END);
    f_size = ftell(fp);
    rewind(fP); //double check this works
    
    fread(&f_buffer, sizeof(f_buffer), 1, fP);
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(HOSTNAME, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    //connect to the first result we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("connect");
            close(sockfd);
            continue;
        }
        
        break; //getting here means we have successfully connected to a server
    }
    
    if (p == NULL)
    {
        fprintf(stderr, "failed to connect\n");
        exit(2);
        
    }
    
    //Send File Size
    len = send(sockfd, f_size, sizeof(file_size), 0);
    if (len < 0)
    {
        perror("send size");
        close(sockfd);
        continue;
    }

    //Send File
    len = send(sockfd, f_buffer, sizeof(f_buffer), 0);
    if (len < 0)
    {
        perror("send file");
        close(sockfd);
        continue;
    }
    
    
    
}
