#include <libk/kstrlen.h>

size_t strlen(const char* str) {
	size_t ret = 0;
	while(str[ret] != 0) {
		ret++;
	}
	return ret;
}
