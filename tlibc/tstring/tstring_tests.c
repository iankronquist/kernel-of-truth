#if !defined(__linux__)
	#error "This test suite is meant to be run on the local machine"
#endif
#include "tstring.h"
#include <stdio.h>
#include <stdbool.h>

bool test_createFromLiteral();
bool test_elem();
bool test_at();
bool test_length();
bool test_extend();

int main()
{
	puts("a");
	if(test_createFromLiteral())
	{
		puts("createFromLiteral succeeded");
	}
	else
	{
		puts("createFromLiteral failed");
	}
	return 0;
}

bool test_createFromLiteral()
{
	puts("b");
	tstring newStr;
	char* literalStr = "Hello, World!";
	int literalLength = 13;
	puts("c");
	createFromLiteral(literalStr, newStr);
	unsigned long prefixLength = 1;
	puts("g");
	for(unsigned long index = 0; index < literalLength; index++)
	{
	puts("h");
	printf("index %lu\n", index);
	printf("index + prefix %lu\n", index + prefixLength);
		if(!literalStr[index] == newStr[index + prefixLength])
		{
			return false;
		}
	}
	return true;
}

/*bool test_elem()
{
	
}*/
