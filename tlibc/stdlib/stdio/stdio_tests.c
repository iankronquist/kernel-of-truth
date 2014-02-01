#define TEST
//This is meant to be run locally.
//#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "stdio.h"
#define BOOL int
#define TRUE 1
#define FALSE 0


char* desired_output;
//Overrides term_putchar() for testing purposes
int term_putchar(char symbol);
BOOL test_putchar();

int main()
{	
	if( ! test_putchar())
	{
		assert(0); // I don't have puts to write a message!
	}
	else if( ! test_putchar())
	{
		assert(0); // I don't have puts to write a message!
	}
	return 0;
}

BOOL test_putchar()
{
	desired_output = "\0a\0";
	desired_output[2] = (char)(127);
	term_putchar('\0');
	
	term_putchar('a');

	term_putchar((char)(127));
	
	return TRUE;
}

BOOL test_printf_int()
{
	desired_output = "number 1234";
	printf("number %i", 1234);

	desired_output = "number 0somethingelse";
	printf("number %isomethingelse", 0);

	return TRUE;
}

BOOL test_printf_char()
{
	desired_output = "number \0";
	printf("number %c", '\0');

	desired_output = "number Asomethingelse";
		printf("number %comethingelse", 'A');

	return TRUE;
}

BOOL test_puts(char* string)
{
	desired_output = "Hello, World!";
	int retval = puts("Hello, World!");
	assert(retval == '!');
	return TRUE;
	
}

BOOL test_putvalue()
{
	desired_output = "123";
	putvalue(123);
	
	desired_output = "0";
	putvalue(0);

	desired_output = "-100";
	putvalue(-100);

	return TRUE;
}

//Overrides term_putchar() for testing purposes
int term_putchar(char symbol)
{
	assert(0);
	assert(symbol == *desired_output);
	desired_output++;
	return (int)symbol;
}



