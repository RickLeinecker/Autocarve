#ifndef __AUTOCARVE_H__
#define __AUTOCARVE_H__

typedef struct tagVALUEVALIDATION
{
	int byteSize;
	int relativeStartOffset;
	int valueMin, valueMax;
} VALUEVALIDATION, *PVALUEVALIDATION;

typedef struct tagFORMATSPECS
{
	unsigned char *header;
	int headerLength;
	int lengthSpecOffset, lengthSpecSize;
	VALUEVALIDATION validation[10];
	int numValidationSpecs;
	char extension[10];
	int type;
} FORMATSPECS, *PFORMATSPECS;

#define PNGTYPE 0
#define GIFTYPE 1
#define JPGTYPE 2
#define BMPTYPE 3
#define TEXTTYPE 4
#define ZIPTYPE 5
#define PDFTYPE 6
#define EXETYPE 7

#define CONSECUTIVE_TEXT 1100

char *getNextHeader(int index, int *type, int *start, int *add, unsigned char *dataBuffer, int fileLength, PFORMATSPECS formatSpecs, int numSpecs);
unsigned char* loadFileData(char *filePath, int *fileLength);
void saveFile(int number, int start, int end, int type, unsigned char *dataBuffer, int fileLength);
FORMATSPECS *readFormatSpecs(int *numFormats);
int getTypeFromExtension(char *extension);

#endif
