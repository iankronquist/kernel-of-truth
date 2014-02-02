#ifndef TEST
	#include "../../terminal.c"
#else
	#include "../tests/test_stdio.c"
#endif

int putchar(char symbol)
{
	#ifdef KERNEL_OF_TRUTH
		return term_putchar(symbol);
	#else
		//Apparently I write a system call...
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

int putvalue(int value)
{
	if(value < 0)
	{
		putchar('-');
	}
	int retVal;
	while(value != 0)
	{
		int remainder = value % 10;
		retVal = putchar((char)(remainder + '0'));
		value /= 10;
	}
	return retVal;
}

