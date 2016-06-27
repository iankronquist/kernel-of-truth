#include <libk/kmemcmp.h>

int memcmp(const void* a, const void* b, size_t size)
{
	const char* ac = (char*)a;
	const char* bc = (char*)b;
	
	for(size_t i = 0; i < size; i++) 
	{
		if(ac[i] > bc[i]) 
		{
			return -1;
		}
		else if(ac[i] < bc[i])
		{
			return 1;		
		}
	}
	
	return 0;
}
