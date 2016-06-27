#include <libk/kmemcpy.h>

void* memcpy(void* destination, const void* source, size_t size)
{
	unsigned char* destinationc = (unsigned char*)destination;
	unsigned char* sourcec = (unsigned char*)source;
	
	for(size_t i = 0; i < size; i++)
	{
		destinationc[i] = sourcec[i];
	}
	
	return destination;
}
