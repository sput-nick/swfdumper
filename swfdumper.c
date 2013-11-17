
// swfdumper
// dump information from a SWF file

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <zlib.h>

#define CHUNK 0x4000 // 256k, zlib.net suggets 128k or 256k

// Decompres the zlib compressed file
// zlib.net/zlib_how.html
int inf(FILE *source, FILE *dest)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	// allocate inflate state
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	// decompress until deflate stream ends or end of file
	do {
		strm.avail_in = fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;
		// run inflate() on input until output buffer not full
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

		// done when inflate() says it's done
	} while (ret != Z_STREAM_END);

	// clean up and return
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void zerr(int ret)
{
	fputs("zlib: ", stderr);
	switch(ret) {
	case Z_ERRNO:
		if (ferror(stdin))
			fputs("error reading stdin\n", stderr);
		if (ferror(stdout))
			fputs("error writing stdout\n", stderr);
		break;
	case Z_STREAM_ERROR:
		fputs("invalid compression level\n", stderr);
		break;
	case Z_DATA_ERROR:
		fputs("invalid or incomplete deflate data\n", stderr);
		break;
	case Z_MEM_ERROR:
		fputs("out of memory\n", stderr);
		break;
	case Z_VERSION_ERROR:
		fputs("zlib version mismatch\n", stderr);
	}

}

int main(int argc, char *argv[])
{
	FILE *fp;
	FILE *sourceFile; // source for zlib inf()
	FILE *destFile;   // dest for zlib inf()
	int ret; // zlib return
	uint8_t *begOfBuf;
	uint8_t *endOfBuf;
	uint8_t *begDecompBuf;
	uint8_t *endDecompBuf;
	uint8_t i;
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
			begOfBuf = calloc(1,(fileLen));
			if (begOfBuf == NULL) {
				printf("Allocation failed\n");
				exit(EXIT_FAILURE);
			}
			else {
			// fill the buffer
			fread(begOfBuf, 1, fileLen, fp);
			fclose(fp);
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
		free(begOfBuf);
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
			isCompressed = 2;
			break;
		default:
			printf("Not a SWF, invalid compression type\n");
			break;
	}

	// Identify the version
	printf("Version: %d\n", *(begOfBuf+3));

	// Get the length reported by the file
	// 4-bytes following the version
	fileLenVar |= *(begOfBuf+4);
	fileLenVar |= *(begOfBuf+5) << 8;
	fileLenVar |= *(begOfBuf+6) << 16;
	fileLenVar |= *(begOfBuf+7) << 24;
	printf("fileLenVar: %d\n", fileLenVar);


	// If the file is compressed then it will need
	// decompressed before continuing analysis
	if (isCompressed == 1) {
		sourceFile = fopen("temp_in.z", "w");
		destFile = fopen("temp_out.z", "w");
		
		if ( sourceFile == NULL){
			printf("Error opening file\n");
			free(begOfBuf);
			exit(EXIT_FAILURE);
		}
		if ( destFile == NULL) {
			printf("Error opening file\n");
			free(begOfBuf);
			exit(EXIT_FAILURE);
		}

		// put the compressed data into the file
		fwrite( (begOfBuf+8), 1, (fileLen-8), sourceFile);
		// close the file
		fclose(sourceFile);
		// open the compressed data for reading
		sourceFile = fopen("temp_in.z", "r");
		ret = inf(sourceFile, destFile);
		if (ret != Z_OK){
			printf("ret != Z_OK: %d\n", ret);
			zerr(ret);
			fclose(sourceFile);
			fclose(destFile);
			free(begOfBuf);
			exit(EXIT_FAILURE);
		}

		// put the header and decompressed data into a new buffer
		destFile = fopen("temp_out.z", "r");
		// get the length
		fseek(destFile, 0, SEEK_END);
		fileLen = ftell(destFile);
		printf("Decompressed data: %d\n", fileLen);
		fseek(destFile, 0, SEEK_SET);
		// make a new buffer
		begDecompBuf = calloc(1,fileLen);
		if (begDecompBuf == NULL) {
			printf("Allocation failed\n");
			exit(EXIT_FAILURE);
		}
		// copy the header from begOfBuf
		for (i=0; i<8; i++) {
			*(begDecompBuf+i) = *(begOfBuf+i);
		}
		// copy the decompressed data to begDecompBuf
		fread((begDecompBuf+8), 1, fileLen, destFile);
		fclose(destFile);
		endDecompBuf = begDecompBuf+fileLen-8;
		
		// set original buffers to the new decompressed buffer
		free(begOfBuf);
		begOfBuf = begDecompBuf;
		endOfBuf = endDecompBuf;
	}

	for (i=0; i<32; i++) {
		printf("Byte %d: 0x%x\n",i,*(begOfBuf+i));
	}

	printf("Continue parsing\n");

	free(begOfBuf);
	return EXIT_SUCCESS;
}
