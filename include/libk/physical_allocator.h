#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <contrib/multiboot.h>

#include <libk/kabort.h>
#include <libk/kassert.h>
#include <libk/kmem.h>
#include <libk/kputs.h>
#include <libk/lock.h>

#ifdef ARCH_X86
#include <truth/private/memlayout.h>
#endif

#ifdef ARCH_USERLAND
#include "tests/memlayout.h"
#endif


#define BIT_INDEX(x) (1 << ((x) % 8))
#define BYTE_INDEX(x) ((x)/8)
#define PAGE_FRAME_CACHE_SIZE 32
#define PAGE_FRAME_MAP_SIZE(x) (x/8/PAGE_SIZE)

// Build the page frame bitmap.
void physical_allocator_init(size_t phys_memory_size);

/* Free a physical page @frame */
void free_frame(page_frame_t frame);

/* Allocate a page frame.
 * @return a new page frame marked as used.
 */
page_frame_t alloc_frame();

/* Mark a single page @frame as used. */
void use_frame(page_frame_t frame);

/* Check if a physical address @frame is free.
 * @return true if it's free, false otherwise
 */
bool is_free_frame(page_frame_t frame);

/* Mark a range of physical addresses as in use.
 * Starts including the @begin page, and goes up to but does not include the
 * @end page.
 */
void use_range(page_frame_t begin, page_frame_t end);

#endif
