#include <arch/x86/paging.h>

page_frame_t alloc_frame() {
    return 0;
}

int inner_map_page(uint32_t *unused(page_dir),
        page_frame_t unused(physical_page), void *unused(virtual_address),
        uint16_t unused(permissions)) {
    return 0;
}

int inner_unmap_page(uint32_t *unused(page_entries),
        void *unused(virtual_address), bool unused(should_free_frame)) {
    return 0;
}
