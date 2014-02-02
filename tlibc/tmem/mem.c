
int memcmp(const void* a, const void* b, size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		if(a[i] > b[i])
		{
			return -1;
		}
		else if(a[i] < b[i])
		{
			return 1;
		}
	}
	return 0;
}

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

void* memset(void* buffer, int value, size_t size)
{
	unsigned char* bufferc = (unsigned char*) buffer;
	for(size_t i = 0; i < size; i++)
	{
		buffer[i] = (unsigned char) size;
	}
	return buffer;
}

void* memmove(void* destination, void* source, size_t size)
{
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	if ( dst < src )
	{
		for ( size_t i = 0; i < size; i++ )
		{
			dst[i] = src[i
		}
	}
	else
	{
		for ( size_t i = size; i != 0; i-- )
		{
			dst[i-1] = src[i-1];
		}
	}
	return dstptr;
}
