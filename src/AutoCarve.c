
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "AutoCarve.h"

/* We use these two variables for the read-in data. */
int fileLength;
unsigned char *dataBuffer;

/*
** Show the program usage.
*/
void usage(void)
{
	printf("Usage: AutoCarve infile\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int index = 0, type, number = 0, start, end, tmpType, tmpStart, add, tmpAdd;
	char *current, *next;

	/* Here we got no file name so we bail out. */
	if (argc < 2)
	{
		usage();
	}

	/* Call the function that loads the data from the given file name. */
	dataBuffer = loadFileData(argv[1], &fileLength );

	int numSpecs;
	/* Call the function that loads and parses the format specifications. */
	PFORMATSPECS formatSpecs = readFormatSpecs(&numSpecs);

	/* Go through each byte of the file. */
	while (index < fileLength)
	{
		/* Here we get the next header that can be found from the file data. */
		current = getNextHeader(index, &type, &start, &add, dataBuffer, fileLength, formatSpecs, numSpecs);

		/* Here we found a header at this byte offset. */
		if (current != NULL)
 		{

			/* Move to the offset past this header (or data for a text file) */
			if (type == TEXTTYPE)
			{
				index += (add - index);
			}
			else
			{
				index++;
			}

			do
			{
				/* Look for another header at this offset. */
				next = getNextHeader(index, &tmpType, &tmpStart, &tmpAdd, dataBuffer, fileLength, formatSpecs, numSpecs);
				end = index;
				index++;
			} 

			/* Keep going until we either get to the end of the file or find the next header. */
			while (index < fileLength && next == NULL);

			index--;

			/* Save this file to disk. */
			saveFile(number, start, end, type, dataBuffer, fileLength );
			/* Increment our number counter. */
			number++;

			/* This means we found the end of the file. */
			if (next == NULL)
			{
				index = fileLength;
			}
		}
		/* Set to end of file. */
		else
		{
			index = fileLength;
		}
	}
	return 0;

}

