#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define PORT 50180
#define MAXSIZE 2 * 1024 * 1024 //2 MB

//Current picture metadata variables
char man[100] = {0};
char model[100] = {0};
char date[100] = {0};
int width, height, iso;

struct header
{
    unsigned short jpeg;
    unsigned short app;
    unsigned short app_length;
    char exif[4];
    unsigned short nul;
    char endianness[2];
    unsigned short version;
    unsigned int offset;
};

struct tifftag //sizeof struct should be 12 bytes
{
    short id;
    short type;
    int items;
    int valoff;
};

//error checks jpeg for endianness and exif data
int checkHeader(struct header theHeader)
{
    //check for app0
    if (theHeader.app != 0xE1FF)
    {
        fprintf(stderr, "Error while reading app marker.\n");
        return 1;
    }
    
    if (strcmp(theHeader.endianness, "II*") != 0)
    {
        fprintf(stderr, "Error due to file endianness.\n");
        return 1;
    }
    
    if (strcmp(theHeader.exif, "Exif") != 0)
    {
        fprintf(stderr, "Error: no exif data found.\n");
        return 1;
    }
    
    return 0;
}

//Print tiff tags in second block
void nextBlockRead(FILE *fP, short counter)
{
    //Define Tag identifiers as const
    const short WIDTH = 0xA002, HEIGHT = 0xA003, ISO_SPEED = 0x8827, EXPOSURE_SPEED = 0x829A, F_STOP = 0x829D, FOCAL_LENGTH = 0x920A, DATE = 0x9003;
    char buf[100];
    int numerator, denominator;
    struct tifftag tag;
    
    int i, file_loc;
    for(i = 0; i < counter; i++)
    {
        fread(&tag, sizeof(struct tifftag), 1, fP);
        
        file_loc = ftell(fP);
        
        //Check for remaing cases
        if(tag.id == WIDTH)
        {
            //printf("\tWidth:\t\t\t%i pixels\n", tag.valoff);
            width = tag.valoff;
        }
        
        else if(tag.id == HEIGHT)
        {
            //printf("\tHeight:\t\t\t%-i pixels\n", tag.valoff);
            height = tag.valoff;
        }
        
        else if(tag.id == ISO_SPEED)
        {
            //printf("\tISO Speed:\t\tISO %-i\n", tag.valoff);
            iso = tag.valoff;
        }
        
        else if(tag.id == DATE)
        {
            fseek(fP, tag.valoff + 12, SEEK_SET);
            fread(&buf, sizeof(buf[0]), tag.items, fP);
            //printf("\tDate Taken:\t\t%-s\n", buf);
            date = buf;
        }
        
        fseek(fP, file_loc, SEEK_SET);
    }
    
    
}

void nextBlockWrite(FILE *fP, short counter)
{
    //Define Tag identifiers as const
    const short WIDTH = 0xA002, HEIGHT = 0xA003, ISO_SPEED = 0x8827, EXPOSURE_SPEED = 0x829A, F_STOP = 0x829D, FOCAL_LENGTH = 0x920A, DATE = 0x9003;
    char buf[100];
    int numerator, denominator;
    struct tifftag tag;
    
    int i, file_loc;
    for(i = 0; i < counter; i++)
    {
        fread(&tag, sizeof(struct tifftag), 1, fP);
        
        file_loc = ftell(fP);
        
        //Check for remaing cases
        if(tag.id == WIDTH)
        {
            //printf("\tWidth:\t\t\t%i pixels\n", tag.valoff);
            width = tag.valoff;
        }
        
        else if(tag.id == HEIGHT)
        {
            //printf("\tHeight:\t\t\t%-i pixels\n", tag.valoff);
            height = tag.valoff;
        }
        
        else if(tag.id == ISO_SPEED)
        {
            //printf("\tISO Speed:\t\tISO %-i\n", tag.valoff);
            iso = tag.valoff;
        }
        
        else if(tag.id == DATE)
        {
            fseek(fP, tag.valoff + 12, SEEK_SET);
            fread(&buf, sizeof(buf[0]), tag.items, fP);
            //printf("\tDate Taken:\t\t%-s\n", buf);
            date = buf;
        }
        
        fseek(fP, file_loc, SEEK_SET);
    }
    
    
}

int main(int argc, const char * argv[]) {

    int sockfd = 0, connfd, c_len = 0, h_len = 0, rv, f_size, len;
    struct sockaddr_in addr;
    char f_buffer[MAXSIZE] = {0};
    char lil_buffer[1024] = {0};
    const char *filename;
    //Tiff tag headers
    const short WIDTH = 0xA002, HEIGHT = 0xA003, ISO_SPEED = 0x8827, EXPOSURE_SPEED = 0x829A, F_STOP = 0x829D, FOCAL_LENGTH = 0x920A, DATE = 0x9003;
    
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
    len = sizeof(struct sockaddr_in);
    
    while(1)
    {
        connfd = accept(sockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
                    
        if(connfd < 0)
        {
            perror("Error accepting connection");
            return 1;
        }
                    
        if (recv(sockfd, lil_buffer, 1024, 0) < 0)
        {
            fprintf(stderr, "error receiving size from server\n");
        }
                    
        printf("file size response: %s\n", lil_buffer); //Debug
        
        f_size = atoi(lil_buffer);
        
        if (recv(sockfd, f_buffer, f_size, 0) < 0)
        {
            fprintf(stderr, "error receiving jpeg from server\n");
        }
        
        //write buffer to file
        
        FILE *wb;
        wb = fopen("rec.jpg", "wb");
        
        fprintf(wb, "%s", f_buffer);

        fclose(wb);
        
        //************************open file*******************************
        
        FILE *edit;
        edit = fopen("rec.jpg", "rb");
        
        
        //read header and tiff tag
        short counter, nextcounter;
        struct header head;
        struct tifftag tag;
        
        //Read in header
        fread(&head, sizeof(struct header), 1, fP);
        
        
        //Debug block to show header fields
        /*printf("jpeg marker: %x\n", head.jpeg);
         printf("app1 marker: %x\n", head.app);
         printf("app1 length: %i\n", head.app_length);
         printf("exif tag: %s\n", head.exif);
         printf("nul term: %i\n", head.nul);
         printf("endianness: %s\n", head.endianness);
         printf("version: %i\n", head.version);
         printf("offset: %i\n", head.offset);*/
        
        //Check if jpeg is valid
        int exit = checkHeader(head);
        if(exit != 0) return exit;
        
        //Read in counter
        
        fread(&counter, sizeof(counter), 1, fP);
        
        //Loop to read tiff tags and check for 3 important cases
        int i;
        int file_loc; //keeps track of where we are in the file_loc
        const short MAN = 0x010f, CAMERA_MODEL = 0x0110, EXIF_SUB = 0x8769;
        
        for(i = 0; i < counter ; i++)
        {
            fread(&tag, sizeof(struct tifftag), 1, fP);
            
            file_loc = ftell(fP);
            
            //Check if Manufacturer String
            //printf("%x\n",tag.id); //Debug IDs
            if(tag.id == MAN)
            {
                fseek(fP, tag.valoff + 12, SEEK_SET);
                fread(&buf, sizeof(buf[0]), tag.items, fP);
                //printf("\tManufacturer:\t\t%-s\n", buf);
                man = buf;
            }
            
            //Check if Camera Model string
            else if(tag.id == CAMERA_MODEL)
            {
                fseek(fP, tag.valoff + 12, SEEK_SET);
                fread(&buf, sizeof(buf[0]), tag.items, fP);
                //printf("\tModel:\t\t\t%-s\n", buf);
                model = buf;
            }
            
            //Check for end of first block
            else if(tag.id == EXIF_SUB)
            {
                fseek(fP, tag.valoff + 12, SEEK_SET); //seek to next block
                fread(&nextcounter, sizeof(nextcounter), 1, fP); //read in amount of tiff tags in block 2
                nextBlockRead(fP, nextcounter);
            }
            
            //reset file in for loop
            fseek(fP, file_loc, SEEK_SET);
    }
        //************************************
        // add to text file
		FILE *old_file;

		old_file= fopen("old_tiff.txt","w+");
		fprintf(old_file, "This is the original JPEG TIFF info: \n");
		fprintf(old_file, "\nManufacturer: %s", man);
		fprintf(old_file, "\nModel: %s", model);
		fprintf(old_file, "\nDate: %s", date);
		fprintf(old_file, "\nWidth: %d", width);
		fprintf(old_file, "\nHeight: %d", height);
		fprintf(old_file, "\nISO: %d", iso);
		
		fclose(old_file);
		
		man = "Pizza";
		model = "Pepperoni";
		date = "2017.11.27";
		width = 412;
		height = 343;
		iso = 7788;
		
		// put together the numbers for width, height and iso for one of my favorite Pgh pizza places
        
        
        
        //******************write**************
        //r+
        FILE *edit;
        edit = fopen("rec.jpg", "rb");
        
        
        //read header and tiff tag
        short counter, nextcounter;
        struct header head;
        struct tifftag tag;
        
        //Read in header
        fread(&head, sizeof(struct header), 1, fP);
        
        
        //Debug block to show header fields
        /*printf("jpeg marker: %x\n", head.jpeg);
         printf("app1 marker: %x\n", head.app);
         printf("app1 length: %i\n", head.app_length);
         printf("exif tag: %s\n", head.exif);
         printf("nul term: %i\n", head.nul);
         printf("endianness: %s\n", head.endianness);
         printf("version: %i\n", head.version);
         printf("offset: %i\n", head.offset);*/
        
        //Check if jpeg is valid
        int exit = checkHeader(head);
        if(exit != 0) return exit;
        
        //Read in counter
        
        fread(&counter, sizeof(counter), 1, fP);
        
        //Loop to read tiff tags and check for 3 important cases
        int i;
        int file_loc; //keeps track of where we are in the file_loc
        const short MAN = 0x010f, CAMERA_MODEL = 0x0110, EXIF_SUB = 0x8769;
        
        for(i = 0; i < counter ; i++)
        {
            fread(&tag, sizeof(struct tifftag), 1, fP);
            
            file_loc = ftell(fP);
            
            //Check if Manufacturer String
            //printf("%x\n",tag.id); //Debug IDs
            if(tag.id == MAN)
            {
                fseek(fP, tag.valoff + 12, SEEK_SET);
                fread(&buf, sizeof(buf[0]), tag.items, fP);
                //printf("\tManufacturer:\t\t%-s\n", buf);
                man = buf;
            }
            
            //Check if Camera Model string
            else if(tag.id == CAMERA_MODEL)
            {
                fseek(fP, tag.valoff + 12, SEEK_SET);
                fread(&buf, sizeof(buf[0]), tag.items, fP);
                //printf("\tModel:\t\t\t%-s\n", buf);
                model = buf;
            }
            
            //Check for end of first block
            else if(tag.id == EXIF_SUB)
            {
                fseek(fP, tag.valoff + 12, SEEK_SET); //seek to next block
                fread(&nextcounter, sizeof(nextcounter), 1, fP); //read in amount of tiff tags in block 2
                nextBlockWrite(fP, nextcounter);
            }
            
            //reset file in for loop
            fseek(fP, file_loc, SEEK_SET);
        }
                    
}
