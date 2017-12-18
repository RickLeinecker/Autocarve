#include <string.h>
#include <memory.h>
#include "AutoCarve.h"

char isText(unsigned char c)
{
	if (c == ' ' || c == '\t' || c == 13 || c == 10 || c == '/' || c == '\\')
	{
		return(1);
	}

	if (c >= '0' && c <= '9')
	{
		return(1);
	}

	if (c >= '!' && c <= '~')
	{
		return(1);
	}

	return(0);
}

/*
** This function examines &dataBuffer[index] to see if it is the start of a new file.
*/
char *getNextHeader(int index, int *type, int *start, int *add, unsigned char *dataBuffer, int fileLength, PFORMATSPECS formatSpecs, int numSpecs)
{
	int i, j;

	*add = 0;

	/* Set start to the current index. If we find a header here, this will be the start of
	** the save.
	*/
	*start = index;

	for (i = 0; i < numSpecs; i++)
	{
		/* Compare our current position to each of the headers in the format specs. */
		if (memcmp(&dataBuffer[index], formatSpecs[i].header, formatSpecs[i].headerLength) == 0)
		{
			/* We may not have any validation specifications, this is where the header
			** such as GIF89a is enough for a positive ID.
			*/
			if (formatSpecs[i].numValidationSpecs == 0)
			{
				if (formatSpecs[i].lengthSpecSize == 2 )
				{
					short tmp = 0;
					memcpy(&tmp, &dataBuffer[index + formatSpecs[i].lengthSpecOffset], 2);
					*add = (int)tmp;
				}
				else if (formatSpecs[i].lengthSpecSize == 4)
				{
					memcpy(add, &dataBuffer[index + formatSpecs[i].lengthSpecOffset], 4);
				}

				/* Get the type and return the start of the header. */
				*type = getTypeFromExtension(formatSpecs[i].extension);
				return((char *)&dataBuffer[index]);
			}

			/* Default the value to 1 for specifications that are within range. */
			int specsOK = 1;

			/* Loop through all of the specifications. */
			for (j = 0; j < formatSpecs[i].numValidationSpecs; j++)
			{
				/* This is a short. */
				if (formatSpecs[i].validation[j].byteSize == 2)
				{
					short value;
					/* Get the value from memory. */
					memcpy(&value, &dataBuffer[index + formatSpecs[i].validation[j].relativeStartOffset], formatSpecs[i].validation[j].byteSize);

					/* Now check to make sure that we are in range. */
					if (value < formatSpecs[i].validation[j].valueMin || value > formatSpecs[i].validation[j].valueMax)
					{
						/* We set to 0 to indicate failure and break from the loop. */
						specsOK = 0;
						break;
					}
				}

				/* This is a long (int) */
				else if (formatSpecs[i].validation[j].byteSize == 4)
				{
					long value;
					/* Get the value from memory. */
					memcpy(&value, &dataBuffer[index + formatSpecs[i].validation[j].relativeStartOffset], formatSpecs[i].validation[j].byteSize);

					/* Now check to make sure that we are in range. */
					if (value < formatSpecs[i].validation[j].valueMin || value > formatSpecs[i].validation[j].valueMax)
					{
						/* We set to 0 to indicate failure and break from the loop. */
						specsOK = 0;
						break;
					}
				}

			}

			/* Here we check to see if all of the specs passed muster. */
			if (specsOK)
			{
				/* Get the type and return the start of the header. */
				*type = getTypeFromExtension(formatSpecs[i].extension);
				return((char *)&dataBuffer[index]);
			}
		}
	}

	/* Count the number of consecutive text characters. */
	int textCount = 0;
	while (index + textCount + 1 < fileLength &&
		isText(dataBuffer[index + textCount + 1]))
	{
		textCount++;
	}

	/* If we have a count that is past a threshold, then we conclude that this
	** is a test file.
	*/
	if (textCount > CONSECUTIVE_TEXT)
	{
		*type = TEXTTYPE;
		*add = index + textCount + 1;
		*start = index + 2;
		return((char *)&dataBuffer[index]);
	}

	return(NULL);
}

