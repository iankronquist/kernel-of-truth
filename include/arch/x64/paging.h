#pragma once

#include <truth/types.h>
#include <truth/memory.h>

struct page_table {
    phys_addr physical_address;
};

enum status checked map_page(void *virtual_address, phys_addr phys_address,
        enum memory_attributes perms);

void unmap_page(void *virtual_address, bool free_physical_memory);

enum status map_external_page(struct page_table *page_table,
        void *virtual_address, phys_addr phys_address, enum memory_attributes
        perms);

void unmap_external_page(struct page_table *page_table, void *virtual_address,
        bool free_physical_memory);

void unmap_range(void *virt, size_t count, bool free_phys);

enum status map_range(void *virt, phys_addr phys, size_t count,
                      enum memory_attributes attrs);

enum status paging_update_range(void *virt, size_t count, enum memory_attributes attrs);

void page_table_switch(phys_addr physical_address);

enum status paging_init(void);

struct page_table *page_table_init(void);

struct page_table *page_table_new_current(void);
struct page_table *page_table_clone(struct page_table *pt);

void page_table_fini(struct page_table *pt);

void debug_paging(void);

phys_addr virt_to_phys(void *virtual_address);
