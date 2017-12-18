#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "AutoCarve.h"

char *exts[] = { "PNG", "GIF", "JPG", "BMP", "TXT", "ZIP", "PDF", "EXE" };

extern FILE *logFile;
extern int findOnly;

/*
** This helper function takes an extension and returns the file type (which is really
** just the index into the exts array).
*/
int getTypeFromExtension(char *extension)
{
	int i;

	/* Loop through the array contents. */
	for (i = 0; i < 8; i++)
	{
		/* Check for a match. */
		if (strncmp(exts[i], extension, strlen(extension)) == 0)
		{
			return(i);
		}
	}

	/* Not found, so return -1.  */
	return(-1);
}

/*
** This function returns the file length for an open file.
** Could have used _filelength(_fileno(fp)) but that is not portable.
*/
int getFileLength(FILE *fp)
{
	int fileLength, curr;
	/* Seek to the end. */
	curr = fseek(fp, 0, SEEK_END);
	/* Record where we are. */
	fileLength = ftell(fp);
	/* Back to where we started. */
	fseek(fp, curr, SEEK_SET);

	/* Return the value. */
	return(fileLength);
}

/*
** This function opens the data file, allocates a memory buffer, and then
** reads the data into the memory buffer. The buffer is returned to the caller.
*/
unsigned char* loadFileData(char *filePath, int *fileLength)
{
	unsigned char *dataBuffer;

	/* Attempt to open the file. */
	FILE *fp = fopen(filePath, "rb");
	if (fp == NULL)
	{
		/* Alert user to the error. */
		printf("Could not open %s for reading.\n", filePath);
		exit(1);
	}

	/* Need the file length for memory allocation, and so that he caller
	**   has the value.
	*/
	*fileLength = getFileLength(fp);

	/* Allocate the memory buffer. */
	dataBuffer = (unsigned char *)malloc(*fileLength);

	/* Check to see if the allocation was successful. */
	if (dataBuffer == NULL)
	{
		/* Close the file handle before we bail out. */
		fclose(fp);
		/* Alert the user to the error. */
		printf("Could not allocate %d bytes for the file data buffer.\n", *fileLength);
		exit(2);
	}

	/* Read the data into the memory buffer. */
	fread(dataBuffer, sizeof(unsigned char), *fileLength, fp);
	/* Close the file handle. */
	fclose(fp);

	/* Return the data buffer which contains the read-in data. */
	return(dataBuffer);
}

/*
** This function saves a carved file. The files are simply numbered since we don't know what
** the original file name was. number=the number for this file, start=the starting byte offset of the data
** end=the ending byte offset of the data, type=PNGTYPE or JPGTYPE etc., databuffer=buffer
** for the entire loaded-in data, fileLength=the length of the data buffer.
*/
void saveFile(int number, int start, int end, int type, unsigned char *dataBuffer, int fileLength)
{
	char fileName[100];
	FILE *fp;
	
	/* Create the temp file name. */
	sprintf(fileName, "Carved%d.%s", number, exts[type]);

	if (findOnly != 0)
	{
		printf("Found Carved%d.%s", number, exts[type]);
		return;
	}

	/* Attempt to create the file for saving. */

	fp = fopen(fileName, "wb");
	/* Check for failure. */
	if (fp == NULL)
	{
		/* Alert user to the error. */
		printf("Could not save %s\n", fileName);
		return;
	}

	/* Make sure we will not try to write past the end of the data buffer. */
	if (end >= fileLength)
	{
		end = fileLength - 1;
	}

	/* Write the data. */
	fwrite(&dataBuffer[start], sizeof(unsigned char), end - start + 1, fp);
	/* Close the file handle. */
	fclose(fp);

	if (logFile != NULL)
	{
		char temp[200];
		sprintf(temp, "%s,%d\r\n", fileName, end - start + 1);
		fwrite(temp, strlen(temp), 1, logFile);
	}

	/* Alert user to success. */
	printf("Saved %s\n", fileName);
}

/*
** This is a helper function that reads the next line from the text file,
** and makes sure that it removes the '\n' that is normally the last character
** of the read-in line.
*/
void getNextLine(FILE *fp, char *line, int sizeOfLine)
{
	memset(line, 0, sizeOfLine);
	fgets(line, sizeOfLine, fp);

	/* NewLine handling in Linux and Windows are different */
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
	if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
	{
		line[strlen(line) - 1] = 0;
	}
#endif

#if defined(_linux) || defined(_LINUX) || defined(linux)
	if (strlen(line) > 0 && line[strlen(line) - 2] == '\r' && line[strlen(line) - 1] == '\n')
	{
		line[strlen(line) - 1] = 0;
		line[strlen(line) - 2] = 0;
	}
	if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
	{
		line[strlen(line) - 1] = 0;
	}
#endif
}

/* Here we have the line type prefixes. */
static char headerStartText[] = "HEADER=";
static char extensionStartText[] = "EXTENSION=";
static char validationStartText[] = "VALIDATION=";

/*
** Process the header line. Allocate a 100-byte buffer for the header, then parse
** the data and place the bytes into the return buffer (named header).
*/
unsigned char *getHeader(char *line, int *headerLength)
{
	int i, j, len;
	char currentToken[100];
	
	/* Allocate the header (return) buffer. */
	unsigned char *header = (unsigned char *)calloc(100, sizeof(unsigned char));
	if (header == NULL)
	{
		printf("Could not allocate the header buffer.\n");
		exit(204);
	}

	/* Here we have some local variables for parsing through the header data. */
	i = strlen(headerStartText);

	/* Start by setting the header length to zero. */
	*headerLength = 0;

	do
	{
		/* Clear the currentToken char buffer. This ensures it will be null terminated. */
		memset(currentToken, 0, sizeof(currentToken));
		j = 0;

		/* Now we go until we get to the next comma or the end of the line. */
		while (line[i] != 0 && line[i] != ',')
		{
			currentToken[j++] = line[i++];
		}

		/* If we found a comma and not the end of the line, we need to skip over the comma. */
		if (line[i] == ',')
		{
			i++;
		}

		/* We need the string length of currentToken so we know how to process it. */
		len = strlen(currentToken);

		/* This is a single character. */
		if (len == 1)
		{
			/* Store the literal single character. */
			header[*headerLength] = currentToken[0];
			(*headerLength)++;
		}

		/* This is a hex value such as 89h */
		else if (len > 0 && toupper(currentToken[len - 1]) == 'H')
		{
			/* Get rid of the trailing 'h' character. */
			currentToken[len - 1] = 0;
			/* Evaluate the hex value and store. */
			header[*headerLength] = (unsigned char)strtol(currentToken, NULL, 16);
			(*headerLength)++;
		}

		/* This is a non-hex value, and we will assume a decimal value. */
		else if (len > 0)
		{
			/* Evaluate the decimal value and store. */
			header[*headerLength] = (unsigned char)atoi(currentToken);
			(*headerLength)++;
		}

	}
	/* Keep doing this while we haven't gotten to the end of the line. */
	while (line[i] != 0);

	/* Return the parsed error. */
	return(header);
}

/*
** This is a simple helper function to copy the extension into the buffer.
*/
void getExtension(char *line, char *extension)
{
	strcpy(extension, &line[strlen(extensionStartText)]);
}

/*
** We use this helper function since we have four opportunities to report this same error.
*/
void reportValidationError(char *line)
{
	printf("The line: '%s' could not be parsed for four validation values.\nFor example, it should take the form VALIDATION=1,2,3,4\nThe four values are size,relative offset,min value,max value\n", line);
	exit(203);
}

/*
** A validate specification will have the form of VALIDATION=1,2,3,4
** The four values are all decimal values. They indicate the size in bytes,
** the relative byte offset, the minimum value, and the maximum value.
*/
void getValidation(char *line, PVALUEVALIDATION validation, int *numValidationSpecs)
{
	char *pt;

	/* Skip past the the VALIDATE= string.  */
	pt = &line[strlen(validationStartText)];
	validation[*numValidationSpecs].byteSize = atoi(pt);

	/* Look for the next comma. */
	pt = strstr(pt, ",") + 1;
	/* If pt is NULL then we did not find another comma. */
	if (pt == NULL)
	{
		/* Call the error reporting function. */
		reportValidationError(line);
	}
	/* Store the value. */
	validation[*numValidationSpecs].relativeStartOffset = atoi(pt);

	/* Look for the next comma. */
	pt = strstr(pt, ",") + 1;
	/* If pt is NULL then we did not find another comma. */
	if (pt == NULL)
	{
		/* Call the error reporting function. */
		reportValidationError(line);
	}
	/* Store the value. */
	validation[*numValidationSpecs].valueMin = atoi(pt);

	/* Look for the next comma. */
	pt = strstr(pt, ",") + 1;
	/* If pt is NULL then we did not find another comma. */
	if (pt == NULL)
	{
		/* Call the error reporting function. */
		reportValidationError(line);
	}
	/* Store the value. */
	validation[*numValidationSpecs].valueMax = atoi(pt);

	(*numValidationSpecs)++;
}

/*
** This function opens the format specification file, allocates an array of structs,
** reads in the lines, and calls the functions to parse the lines.
*/
FORMATSPECS *readFormatSpecs(int *numFormats)
{
	FORMATSPECS *ret;
	int formatIndex = -1;
	char line[500];

	*numFormats = 0;

	/* Attempt to open the file. */
	FILE *fp = fopen("FormatSpecs.txt", "r");

	/* Check for file open error. */
	if (fp == NULL)
	{
		/* Alert the user to the error. */
		printf("Could not open FormatSpecs.txt\n");
		exit(200);
	}

	/* First we will count the number of specified headers. */
	*numFormats = 0;
	while (!feof(fp))
	{
		/* Get the next line. */
		fgets(line, sizeof(line), fp);
		/* If it starts with HEADER= then we increment our counter. */
		if (strncmp(line, headerStartText, strlen(headerStartText)) == 0)
		{
			(*numFormats)++;
		}
	}

	/* If we did not find any format specifications, then we need to bail out. */
	if (*numFormats == 0)
	{
		printf("No headers found in FormatSpecs.txt\n");
		exit(201);
	}

	/* Allocate the return buffer. */
	ret = (PFORMATSPECS)calloc(*numFormats, sizeof(FORMATSPECS));
	/* Bail out if the allocation failed. */
	if (ret == NULL)
	{
		printf("Could not allocate the FORMATSPECS array with %d format specifications.\n", *numFormats);
		exit(202);
	}

	/* Now go back to the beginning of the file so we can read each record and store them. */
	fseek(fp, 0, SEEK_SET);

	while (!feof(fp))
	{
		/* Get the next line. */
		getNextLine(fp, line, sizeof(line));

		/* This lets us know that we have a new header specification. */
		if (strncmp(line, headerStartText, strlen(headerStartText)) == 0)
		{
			formatIndex++;
			ret[formatIndex].header = getHeader(line, &ret[formatIndex].headerLength);
		}

		/* This lets us know that we have a new extension specification. */
		else if (formatIndex >= 0 && strncmp(line, extensionStartText, strlen(extensionStartText)) == 0)
		{
			getExtension(line, ret[formatIndex].extension);
		}

		/* This lets us know that this line is a validation specification. */
		else if (formatIndex >= 0 && strncmp(line, validationStartText, strlen(validationStartText)) == 0)
		{
			getValidation(line, ret[formatIndex].validation, &ret[formatIndex].numValidationSpecs);
		}

	}

	/* Close the file handle. */
	fclose(fp);

	/* Return the array containing the file format specifications. */
	return (ret);
}
