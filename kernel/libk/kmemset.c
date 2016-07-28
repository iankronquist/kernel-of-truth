#include <libk/kmemset.h>

void* memset(void* buffer, int value, size_t size)
{
	unsigned char* bufferc = (unsigned char*) buffer;
	for(size_t i = 0; i < size; i++)
	{
		bufferc[i] = (unsigned char) value;
	}
	return buffer;
}
