
// swfdump
// dump information about a swf file

#include <stdio.h>
#include <stdlib.h>

int idFile(FILE *inFile)
{
	// identify the file
	int i;
	int c;
	int magic[4];

	printf("File magic:");

	for (i=0; i<4; i++) {
		magic[i] = getc(inFile);
		if (magic[i] == EOF) {
			printf("EOF not swf\n");
			exit(1);
		}
		printf(" 0x%x", magic[i]);
	}
	printf("\n");
	
	if ( (magic[1] != 87) || (magic[2] != 83) ) {
		printf("Not a swf file '*WS'\n");
		exit(1);
	}

	switch (magic[0]) {
		case 67:
			printf("zlib compressed\n");
			break;
		case 90:
			printf("LZMA compressed\n");
			break;
		case 70:
			printf("Not compressed\n");
			break;
		default:
			printf("Not a swf file, invalid compression\n");
			exit(1);
	}

	printf("Version: %d\n",magic[3]);

	return magic[0];
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int compressionType;
	int fileLength;

	// no file provided
	if (argc != 2) {
		printf("Just one file...\n");
		return 1;
	}
	
	else {
		if ((fp = fopen(argv[1], "r")) == NULL) {
			printf("Can not open file %s\n", argv[1]);
			return 1;
		}
		else {
			compressionType = idFile(fp);
			printf("Compression type = 0x%x\n", compressionType);
		}
	}

	// Verify the file length
	// length is 4-byte
	

	// Done
	fclose(fp);
	return 0;
}
