#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <math.h>

#define MYPORT 50175 
#define HEADERS 50//int size of the approximate size of headers being sent in request line

void *handle_connection(void *connfd);

struct client
{
	int cli_connfd;
	struct sockaddr_in cliaddr;
};

int main()
{
	int sfd, connfd, *new_fd, len;
	struct sockaddr_in addr;
	sfd = socket(PF_INET, SOCK_STREAM, 0);
	
	if(sfd < 0)
	{
		perror("Error initializing socket\n");
		return 1;
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MYPORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	int b = 0;
	
	b = bind(sfd, (struct sockaddr *)&addr, sizeof(addr));
	
	if(b < 0)
	{
		perror("Error setting up port\n");
		return 1;
	}
	
	int l = 0;
	
	l = listen(sfd, 10);
	puts("Listening...");//debug	
		
	if(l < 0)
	{
			perror("Error listening for port");
			return 1;
	}
	
		len = sizeof(struct sockaddr_in);
		
		struct client *new_client = (struct client*)malloc(sizeof(struct client));
	
		connfd = accept(sfd, (struct sockaddr *)&(new_client->cliaddr), &len);
		new_client->cli_connfd = connfd;
		
		if(connfd < 0)
		{
			perror("Error accepting connection");
			return 1;
		}
		
		handle_connection((void *)new_client);
		
	}	

return 0;
}

void *handle_connection(void *cfd)
{
	struct client this_client = *(struct client *)cfd;
	int connfd = this_client.cli_connfd; //stores connection file descriptor to new thread stack
	char buffer[2000]; //holds entire get request
	char file_path[2000]; //holds path to file
	recv(connfd, buffer, 2000, 0);
 
	if(strncmp(buffer, "GET /", 5) == 0)
	{
		//find webpage
		int i;
		for (i=0 ; i < 2000 ; i++)
		{
			if(buffer[i+5] == '\r' && buffer[i+6] == '\n' || buffer[i+5] == ' ')
			{
				//end found
				file_path[i] = '\0';
				break;
			}
			else
			{
				file_path[i] = buffer[i+5];
			}
		}
		
		FILE *fp;
		fp = fopen(file_path, "r");
		
		if(fp != NULL)
		{
			//file exists
			//local variables for sending file
			int *int_c, f_size;
			char *file_copy, *reply;
			time_t curtime;
			struct tm *loctime;
			char time_str[35];
			
			//get file length
			fseek(fp, 0, SEEK_END);
			f_size = ftell(fp);
			file_copy = (char *)malloc(f_size+1); //allocate space for copy of file_copy
			//copy file contents to file_copy
			fseek(fp, 0, SEEK_SET);
			fread(file_copy,1,f_size, fp); //blocking, so no mutex needed
			fclose(fp);
			
			//get date and time
			curtime = time (NULL);
			loctime = localtime(&curtime);
			strftime(time_str,35,"Date: %a, %d %b %Y %T %Z", loctime);
			
			//allocate space for request
			//this malloc is poorly implemented for a case using more than the 4 headers we are using
			reply = (char *)malloc(sizeof(time_str) + HEADERS + sizeof(file_path) + sizeof(file_copy) + 20);
			
			//create and send ok request 200
			strcpy(reply, "HTTP/1.1 200 OK\r\n"); //reply line
			strcat(reply, time_str); //Date header
			strcat(reply, "\r\nContent-Length: ");
			//convert int representation of size to string
			int nDigits = (floor(log10(abs(f_size))) + 1);
			char file_size[nDigits+2];
			sprintf(file_size, "%d", f_size);
			strcat(reply, file_size); //Content-Length header
			strcat(reply,"\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n");
			strcat(reply, file_copy);
						
			send(connfd, reply, strlen(reply), 0);
			
			free(reply);
			free(file_copy);		
		}
		else
		{
			//send file not found error 404
			char error_404[25];
			strcpy(error_404, "HTTP/1.1 404 Not Found\r\n\r\n"); //carriage return should be included with linefeed for linux but not on windows?
			send(connfd, error_404, strlen(error_404), 0);
		}
			
	}
	FILE *stats_fp;
	stats_fp = fopen("stats.txt", "a");
	if(stats_fp != NULL)
	{
		//append GET request
		fprintf(stats_fp, "%s", buffer); //add client addr
		//get client ip from argument
		unsigned short port;
		char *ipstring;
		port = ntohs(this_client.cliaddr.sin_port);
		ipstring = inet_ntoa(this_client.cliaddr.sin_addr);
		//write client info
		fprintf(stats_fp, "Client: %s:%u\n\n", ipstring, port);
		fputs("**********************\n", stats_fp);
	}
	else
	{
		//something went wrong
		puts("something went wrong");
	}
	fclose(stats_fp);
	free(cfd);
	return NULL;
}
