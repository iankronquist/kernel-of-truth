#ifdef KERNEL_OF_TRUTH
	#ifndef TEST
		#include "../../terminal.c"
	#else
		//#include "stdio_tests.c"
	#endif
#endif
#include "stdio_tests.c"
#include "stdio.h"

int putchar(char symbol)
{
	#ifdef KERNEL_OF_TRUTH
		return term_putchar(symbol);
	#else
		//Apparently I write some sort of system call... huh.
		//abort();
		return '\0';
	#endif
}

int puts(char* string)
{
	int i;
	for(i = 0; string[i] != '\0'; i++)
	{
		term_putchar(string[i]);
	}
	return string[i];
}

int printf(char* string, int value)
{
	int i;
	for(i = 0; string[i] != '\0'; i++)
	{
		if(string[i] != '%')
		{
			putchar(string[i]);
		}
		else
		{
			if(string[i++] != 'i')
			{
				return string[i];
			}
			putvalue(value);
		}
	}
	return (int)string[i];
}
/*
int printf(char* string, char value)
{
	int i;
	for(i = 0; string[i] != '\0'; i++)
	{
		if(string[i] != '%')
		{
			putchar(string[i]);
		}
		else
		{
			if(string[i++] != 'i')
			{
				return string[i];
			}
			putchar(value);
		}
	}
	return (int)string[i];
}
*/

int putvalue(int value)
{
	if(value < 0)
	{
		putchar('-');
	}
	while(value != 0)
	{
		int remainder = value % 10;
		putchar((char)(remainder + '0'));
		value /= 10;
	}
	// TODO: what's a better way to get this return value?
	return 0;
}



/*
long putvalue(long value)
{
	while(value != 0)
	{
		long remainder = value % 10;
		putchar((char)(remainder + '0'));
		value /= 10;
	}
	// TODO: what's a better way to get this return value?
	return 0;
}

float putvalue(float value)
{
	while(value != 0)
	{
		float remainder = value % 10;
		putchar((char)(remainder + '0'));
		value /= 10;
	}
	// TODO: what's a better way to get this return value?
	return 0;
}
*/

