#include <arch/x86/paging.h>

void kernel_page_table_install() {
    uint32_t *page_dir = (uint32_t*)PAGE_DIRECTORY;
    uint32_t *cur_page_entry = (uint32_t*)PAGE_DIRECTORY + PAGE_SIZE;
    for (uint32_t table_i = 0; table_i < PAGE_TABLE_SIZE; table_i++) {
        page_dir[table_i] = TOP20(cur_page_entry) | 1;
        for (uint32_t entry_i = 0; entry_i < PAGE_TABLE_SIZE; entry_i++) {
            cur_page_entry[entry_i] = LOW10(table_i) << 22 | LOW10(entry_i) << 12 | 1;
        }
        cur_page_entry += PAGE_SIZE;
    }
    enable_paging(page_dir);
}
