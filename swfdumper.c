
// swfdumper
// dump information from a file

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	char *begOfBuf;
	char *endOfBuf;
	unsigned long fileLen;

	// no file provided
	if (argc !=2) {
		printf("Just one file...\n");
		exit(1);
	}
	else {
		if (( fp = fopen(argv[1], "r")) == NULL) {
			printf("Can not open file %s\n", argv[1]);
			exit(1);
		}
		else {
			// get the file length
			fseek(fp, 0, SEEK_END);
			fileLen = ftell(fp);
			printf("File size on disk: %lu\n", fileLen);
			fseek(fp, 0, SEEK_SET);
			
			// 8 bytes are required for the header
			if (fileLen < 8) {
				printf("Incomplete header\n");
				exit(1);
			}

			// allocate buffer
			begOfBuf = malloc(fileLen + 1);
			if (begOfBuf == NULL) {
				printf("Allocation failed\n");
				exit(1);
			}
			else {
			// fill the buffer
			fread(begOfBuf, 1, fileLen, fp);
			}
			endOfBuf = begOfBuf + fileLen;
			//printf("first byte 0x%x\n", *begOfBuf);
			//printf("second byte 0x%x\n", *(begOfBuf+1));
			//printf("third byte 0x%x\n", *(begOfBuf+2));
		}
	}	

	// identify the file
	if ( (*(begOfBuf+1) != 0x57) || (*(begOfBuf+2)) != 0x53 ) {
		printf("File is not SWF *WS\n");
		exit(1);
	}

	// Identify the compression type
	switch (*begOfBuf) {
		case 0x46:
			printf("Not compressed\n"); 
			// length is 4-bytes following FWS
			break;
		case 0x43:
			printf("zlib compressed\n");
			break;
		case 0x5A:
			printf("LZMA compressed\n");
			break;
		default:
			printf("Not a SWF, invalid compression type\n");
			break;
	}

	// Identify the version
	printf("Version: %d\n", *(begOfBuf+3));
}
