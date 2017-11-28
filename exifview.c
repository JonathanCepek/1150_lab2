//EXIF viewer for CS449 by Jonathan Cepek

#include <stdio.h>

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

//-------------------------------------------------------------------FUNCTIONS------------------------------------------------------

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
void nextBlock(FILE *fP, short counter)
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
			printf("\tWidth:\t\t\t%i pixels\n", tag.valoff);
		}

		else if(tag.id == HEIGHT)
		{
			printf("\tHeight:\t\t\t%-i pixels\n", tag.valoff);
		}

		else if(tag.id == ISO_SPEED)
		{
			printf("\tISO Speed:\t\tISO %-i\n", tag.valoff);
		}

		else if(tag.id == EXPOSURE_SPEED)
		{
			fseek(fP, tag.valoff + 12, SEEK_SET);
			fread(&numerator, sizeof(numerator), 1, fP);
			fread(&denominator, sizeof(denominator), 1, fP);
			printf("\tExposure Time:\t\t%-i/%-i second\n", numerator, denominator);
		}

		else if(tag.id == F_STOP)
		{
			fseek(fP, tag.valoff + 12, SEEK_SET);
			fread(&numerator, sizeof(numerator), 1, fP);
			fread(&denominator, sizeof(denominator), 1, fP);
			printf("\tF-stop:\t\t\tf/%-.1d\n", numerator/denominator);
		}

		else if(tag.id == FOCAL_LENGTH)
		{
			fseek(fP, tag.valoff + 12, SEEK_SET);
			fread(&numerator, sizeof(numerator), 1, fP);
			fread(&denominator, sizeof(denominator), 1, fP);
			printf("\tFocal Length:\t\t%-.0f mm\n", (double)(numerator/denominator));
		}

		else if(tag.id == DATE)
		{
			fseek(fP, tag.valoff + 12, SEEK_SET);
      			fread(&buf, sizeof(buf[0]), tag.items, fP);
      			printf("\tDate Taken:\t\t%-s\n", buf);
		}

	fseek(fP, file_loc, SEEK_SET);
	}


}

//-------------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{	
	if(argc == 2)
	{
		FILE *fP;
		fP = fopen(argv[1], "rb");
		short counter, nextcounter;
		char buf[100];
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
      				printf("\tManufacturer:\t\t%-s\n", buf);
			}
			
			//Check if Camera Model string
			else if(tag.id == CAMERA_MODEL)
			{
				fseek(fP, tag.valoff + 12, SEEK_SET);
      				fread(&buf, sizeof(buf[0]), tag.items, fP);
				printf("\tModel:\t\t\t%-s\n", buf);
			}

			//Check for end of first block
			else if(tag.id == EXIF_SUB)
			{
				fseek(fP, tag.valoff + 12, SEEK_SET); //seek to next block
				fread(&nextcounter, sizeof(nextcounter), 1, fP); //read in amount of tiff tags in block 2
				nextBlock(fP, nextcounter);
			}

			//reset file in for loop
			fseek(fP, file_loc, SEEK_SET);
		}
	}
	else if (argc > 2)
	{
		printf("Too many arguments supplied.\n");
	}
	else
	{
		printf("Please run with a .jpg file as the argument\n");
	}

return 0;
}
