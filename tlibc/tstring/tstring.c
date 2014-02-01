#include "tstring.h"
#if !defined(__linux__)
	#include "../assert/assert.h"
#else
	#include <assert.h>
	#include <stdio.h>
#endif


typedef unsigned long ulong;
const char HIGHBIT = 1 << (sizeof(char) - 1);

void createFromLiteral(char* literal, tstring* str)
{
	ulong literalLength = 0;
	for(; literal[literalLength] != '\0'; literalLength++);
	printf("lit len %lu\n", literalLength);
	puts("d");
	ulong prefixLength;
	puts("e");
	addPrefixToTstring(str, &prefixLength, literalLength);
	puts("f");
	char newStr[prefixLength + literalLength];
	for(ulong index = 0; index < literalLength; index++)
	{
		newStr[index + prefixLength] = literal[index];
	}
	str = newStr;
	printf("newstrp %p \n", &newStr);
	printf("strp    %p \n", &str);
	assert(str == newStr);
}

char elem(tstring str, int index)
{
	ulong initialIndex = prefixLength(str);
	assert(initialIndex + index <= length(str));
	assert(initialIndex < initialIndex + index);
	return str[initialIndex + index];
}

ulong prefixLength(tstring a)
{ 
	long prefix;
	// TODO: this same number is calculated when getting the length
	// can this be made more efficient?
	for(prefix = 0; a[prefix] && HIGHBIT; prefix++);
	return prefix;
}

char at(tstring str, int index)
{
	return elem(str, index);
}

ulong length(tstring str)
{
	long lengthValue = 0;
	for(long i = 0; str[i] && HIGHBIT; i++)
	{
		// check for overflow.
		// TODO convert to if statement and recover
		// from malformed tstring.
		assert(lengthValue < lengthValue + (str[i] & ~HIGHBIT));
		lengthValue += str[i] & ~HIGHBIT;
	}
	return lengthValue;
}

ulong realLength(tstring a)
{
	return prefixLength(a) + length(a);
}

void addPrefixToTstring(tstring newEmptyStr, ulong* prefixLength, 
	ulong dataLength)
{	
	
	ulong newPrefixLength = (dataLength/(sizeof(char)*8 - 1)) + 1;
	char newString[newPrefixLength + dataLength];
	ulong l;
	for(l = 0; l < newPrefixLength; l++)
	{
		newString[l] = (char)~0;
	}
	newString[l++] = (dataLength) % (sizeof(char)*8 - 1);
	newEmptyStr = (tstring)newString;
	*prefixLength = l;
}

void extend(tstring str, int extra_space, tstring new_str)
{
	ulong currentLength = length(str);
	ulong currentLengthPrefixSize = prefixLength(str);
	ulong newDataLength = currentLength + extra_space;
	assert(currentLength < newDataLength);
	ulong currentStrIndex = 0;
	ulong l;
	tstring newString;
	addPrefixToTstring(newString, &l, newDataLength);
	for(; l < currentLength; l++)
	{
		newString[l] = str[currentLengthPrefixSize + currentStrIndex];
		currentStrIndex++;
	}
	new_str = (tstring)newString;
}

void concat(tstring a, tstring b, tstring newString)
{
	ulong bLength = length(b);
	extend(a, bLength, newString);
	ulong newStringLength = length(newString);
	for(ulong l = 0; l < bLength; l++)
	{
		newString[newStringLength - 1 - l] = b[bLength - 1 - l];
	}
}

void copy(tstring a, tstring b)
{
	ulong aSize = realLength(a);
	char newString[aSize];
	for(ulong i = 0; i < aSize; i++)
	{
		newString[i] = a[i];
	}
	b = newString;
}

int compare(tstring a, tstring b)
{
	ulong aLength = length(a);
	if(aLength != length(b))
	{
		return 0;
	}
	else if(aLength == 0)
	{
		return 0;
	}
	for(int i = 0; i < aLength; i++)
	{
		if(! a[i] == b[i])
		{
			return 0;
		}
	}
	return 1;
}

