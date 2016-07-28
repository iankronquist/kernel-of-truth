#include <libk/kmemmove.h>

void* memmove(void* destination, const void* source, size_t size)
{
	unsigned char* dst = (unsigned char*) destination;
	const unsigned char* src = (const unsigned char*) source;
	if ( dst < src )
	{
		for ( size_t i = 0; i < size; i++ )
		{
			dst[i] = src[i];
		}
	}
	else
	{
		for ( size_t i = size; i != 0; i-- )
		{
			dst[i-1] = src[i-1];
		}
	}
	return destination;
}
