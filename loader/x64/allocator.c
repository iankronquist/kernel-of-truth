#include <loader/allocator.h>
#include <truth/types.h>
#include <truth/memory.h>

static void *Boot_Allocator_Next_Page = (void *)0x1000;
static void *Boot_Allocator_Last_Page = NULL;

void *boot_allocator(size_t pages) {
    void *page = Boot_Allocator_Next_Page;
    if (Boot_Allocator_Next_Page == Boot_Allocator_Last_Page) {
        return NULL;
    }

    Boot_Allocator_Next_Page += pages * Page_Small;
    return page;
}

void boot_allocator_init(uint64_t kilobytes) {
    Boot_Allocator_Last_Page = (void *)(kilobytes * KB);
}
