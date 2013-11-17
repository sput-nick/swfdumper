
// swfdumper
// dump information from a SWF file

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	uint8_t *begOfBuf;
	//uint8_t *tmp_cursor;
	uint8_t *endOfBuf;
	unsigned int fileLen;
	uint32_t fileLenVar = 0;
	uint8_t isCompressed = 0; 

	// no file provided
	if (argc !=2) {
		printf("Just one file...\n");
		exit(EXIT_FAILURE);
	}
	else {
		if (( fp = fopen(argv[1], "r")) == NULL) {
			printf("Can not open file %s\n", argv[1]);
			exit(EXIT_FAILURE);
		}
		else {
			printf("\n%s\n", argv[1]);
			// get the file length
			fseek(fp, 0, SEEK_END);
			fileLen = ftell(fp);
			printf("File size on disk: %d\n", fileLen);
			fseek(fp, 0, SEEK_SET);
			
			// 8 bytes are required for the header
			if (fileLen < 8) {
				printf("Incomplete header\n");
				exit(EXIT_FAILURE);
			}

			// allocate buffer
			begOfBuf = malloc(fileLen + 1);
			if (begOfBuf == NULL) {
				printf("Allocation failed\n");
				exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
	}

	// Identify the compression type
	switch (*begOfBuf) {
		case 0x46:
			printf("Not compressed\n"); 
			// length is 4-bytes following version
			break;
		case 0x43:
			printf("zlib compressed\n");
			isCompressed = 1;
			break;
		case 0x5A:
			printf("LZMA compressed\n");
			isCompressed = 1;
			break;
		default:
			printf("Not a SWF, invalid compression type\n");
			break;
	}

	// Identify the version
	printf("Version: %d\n", *(begOfBuf+3));

	// Get the length reported by the file
	// 4-bytes following the version
	fileLenVar |= *(begOfBuf+4) << 0;
	fileLenVar |= *(begOfBuf+5) << 8;
	fileLenVar |= *(begOfBuf+6) << 16;
	fileLenVar |= *(begOfBuf+7) << 32;
	printf("fileLenVar: %d\n", fileLenVar);

	// If the file is compressed then it will need
	// decompressed before continuing analysis
	if (isCompressed == 1) {
		printf("Some code here will one day decompress this data.\n");
		printf("For now, just exit\n");
		exit(EXIT_FAILURE);
	}
	else {
		printf("Parsing code for data\n");
	}

	return EXIT_SUCCESS;
}
