// Implementation of string and memory functions
#include <string.h>

// Compare two strings
int strcmp(const char* str1, const char* str2)
{
	int result = 0;
	while (!(result = *(unsigned char*)str1 - *(unsigned char*)str2) && *str2)
	{
		++str1;
		++str2;
	}
	if (result < 0)
	{
		result = -1;
	}
	if (result > 0)
	{
		result = 1;
	}
	return result;
}

// Compare two strings without case. 
// @param str1 the first string
// @param str2 the second string
// @return 0 if equal, 1 if greater, -1 if less.
int strcasecmp(const char* str1, const char* str2)
{
	int result = 0;
	while (!(result = CharToUpper(*(unsigned char*)str1) - CharToUpper(*(unsigned char*)str2)) && *str2)
	{
		++str1;
		++str2;
	}
	if (result < 0)
	{
		result = -1;
	}
	if (result > 0)
	{
		result = 1;
	}
	return result;
}

// Takes a character and converts it to the upper case
// @param character the character to convert to upper
// @return Capped version, if lowercase letter otherwise the same
char CharToUpper(char character)
{
	if(character >= 'a' && character <= 'z')  //Only if it's a lower letter
		return character = 'A' + (character - 'a');
    
	return character;
}

char* toUpper(char* string)
{
    char* temp = string;
    while(*temp != '\0')
    {
        if(*temp >= 'a' && *temp <= 'z')  //Only if it's a lower letter
            *temp = 'A' + (*temp - 'a');
		temp++;
    }
    return string;
}

char* toLower(char* string)
{
 	char* temp = string;
    while(*temp != '\0')
    {
        if(*temp >= 65 && *temp <= 90)  //Only if it's a lower letter
            *temp +=  32;
		temp++;
    }
    return string;
}

int strncmp(const char* str1, const char* str2, size_t size) {
	
	int result = 0;
	int cnt = 1;	
	while (!(result = *(unsigned char*)str1 - *(unsigned char*)str2) && *str2 && (cnt < size))
	{
		++str1;
		++str2;
		++cnt;
	}
	
	if (result < 0)
	{
		result = -1;
	}
	
	if (result > 0)
	{
		result = 1;
	}
	
	return result;
}

// atoi 
// @param intString the string
int atoi(char* intString) 
{
	int res = 0;
	bool hitNum = false;
	for (; *intString != '\0'; intString++) 
	{
		if ( (*intString >= '0') && (*intString <= '9'))
		{
			res = (res * 10) + ( *intString - '0');
			hitNum = true;
		} 
		else 
		{
			if (hitNum) 
			{
				break;
			}
		}
	}
	return res;
}

// Copy string source to destination
char * strcpy(char * destination, const char * source)
{
    char * destinationTemp = destination;
    while (*destination++ = *source++);
    return destinationTemp;
}

// Concatenate strings together 
char* strcat(char* destination, const char* str1, const char* str2)
{
	char* temp = destination;
	char* copyTemp = str1;
	
	while(*copyTemp != '\0')
    {
        *temp = *copyTemp;
		temp++;
		copyTemp++;
    }

	copyTemp = str2;
	while(*copyTemp != '\0')
    {
        *temp = *copyTemp;
		temp++;
		copyTemp++;
    }

	*temp = 0;

	return destination;
} 


// Safe version of strcpy

errno_t strcpy_s(char *destination, size_t numberOfElements, const char *source)
{
	if (destination == NULL || source == NULL)
	{
		return EINVAL;
	}
	if (numberOfElements == 0)
	{
		*destination = '\0';
		return ERANGE;
	}
	char * sourceTemp = (char *)source;
	char * destinationTemp = destination;
	char charSource;
	while (numberOfElements--)
	{
		charSource = *sourceTemp++;
		*destinationTemp++ = charSource;
		if (charSource == 0)
		{
			// Copy succeeded
			return 0;
		}
	}
	// We haven't reached the end of the string
	*destination = '\0';
	return ERANGE;
}


// Return length of string

size_t strlen(const char* source) 
{
	size_t len = 0;
	while (source[len] != 0)
	{
		len++;
	}
	return len;
}

// Copy count bytes from src to dest

void * memcpy(void * destination, const void * source, size_t count)
{
    const char *sp = (const char *)source;
    char *dp = (char *)destination;
    while (count != 0)
	{
		*dp++ = *sp++;
		count--;
	}
    return destination;
}

// Safe verion of memcpy

errno_t memcpy_s(void *destination, size_t destinationSize, const void *source, size_t count)
{
	if (destination == NULL)
	{
		return EINVAL;
	}
	if (source == NULL)
	{
		memset((void *)destination, '\0', destinationSize);
		return EINVAL;
	}
	if (destinationSize < count == 0)
	{
		memset((void *)destination, '\0', destinationSize);
		return ERANGE;
	}
	char * sp = (char *)source;
	char * dp = (char *)destination;
	while (count--)
	{
		*dp++ = *sp++;
	}
	return 0;
}

// Set count bytes of destination to val

void * memset(void *destination, char val, size_t count)
{
    unsigned char * temp = (unsigned char *)destination;
	while (count != 0)
	{
		temp[--count] = val;
		
	}
	return destination;
}

// Returns a padded string in the destination buffer. 
// @param destination the destination buffer 
// @param desSize the destination size 
// @param source the source buffer 
// @param the source size 
// @return the character array
char * strpad(char * destination,  size_t desSize, const char* source,  size_t sourceSize, char padChar) 
{ 
	size_t i = 0;
	for (i = 0; i < sourceSize; i++)
	{
		destination[i] = source[i];
	}

	for (; i < desSize; i++)
	{
		destination[i] = padChar;
	}

	return destination;
}


// Set count bytes of destination to val

unsigned short * memsetw(unsigned short * destination, unsigned short val, size_t count)
{
    unsigned short * temp = (unsigned short *)destination;
    while(count != 0)
	{
		*temp++ = val;
		count--;
	}
    return destination;
}


// Return the position of a character in a string 
int strchr(char* haystack, char needle) 
{ 
	int result = 0;
	int cnt = 0;	
	while (!(result = (*haystack == needle)) && *haystack)
	{
		++haystack;
		++cnt;
	}

	return result == 1 ? cnt : -1;
}
