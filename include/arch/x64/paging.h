#pragma once

#include <truth/types.h>

struct page_table;

enum status checked map_page(void *virtual_address, phys_addr phys_address,
        enum memory_attributes perms);

void unmap_page(void *virtual_address, bool free_physical_memory);

enum status map_external_page(struct page_table *page_table,
        void *virtual_address, phys_addr phys_address, enum memory_attributes
        perms);

void unmap_external_page(struct page_table *page_table, void *virtual_address,
        bool free_physical_memory);

void init_page_table(struct page_table *page_table);

void switch_page_table(struct page_table *page_table);

void debug_paging(void);
