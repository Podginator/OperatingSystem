#ifndef _STRING_H_
#define _STRING_H_

#include <size_t.h>
#include <_null.h>
#include <errno.h>
#include <stdint.h>

int strcmp(const char* str1, const char* str2); 
int strncmp(const char* str1, const char* str2, size_t size);
int strcasecmp(const char* str1, const char* str2);
int strncasecmp(const char* str1, const char* str2, size_t size);
char* toLower(char* string);
char* toUpper(char* string);
char * strcpy(char * destination, const char * source);
char * strpad(char * destination,  size_t desSize, const char* source,  size_t sourceSize, char padChar);
char* strcat(char* destination, const char* str1, const char* str2); 
errno_t strcpy_s(char *destination, size_t numberOfElements, const char *source);
size_t strlen(const char* source);
size_t strnlen(const char* source);
void * memcpy(void * destination, const void * source, size_t count);
errno_t memcpy_s(void *destination, size_t destinationSize, const void *source, size_t count);
void * memset(void *destination, char val, size_t count);
unsigned short * memsetw(unsigned short * destination, unsigned short val, size_t count);
int atoi(const char* intString);
int strchr(const char* haystack, char needle);
char CharToUpper(char character);
int strcount(const char* haystack, char needle);



#endif