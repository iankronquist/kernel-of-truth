#include <truth/panic.h>
#include <truth/heap.h>
#include <truth/slab.h>
#include <truth/log.h>
#include <truth/drive.h>

#include "ext2.h"

struct drive_buffer *ext2_read_block(int device, unsigned int block) {
    void *data = slab_alloc(Drive_Buffer_Size, Memory_Writable);
    if (data == NULL) {
        logf(Log_Error, "Could not allocate drive buffer slab\n");
        return NULL;
    }

    struct drive_buffer *b = kmalloc(sizeof(struct drive_buffer));
    if (b == NULL) {
        logf(Log_Error, "Could not allocate drive buffer struct\n");
        return NULL;
    }
    b->flags = 0;
    b->dev = device;
    b->block_number = block;
    b->next = NULL;
    b->data = data;

    ata_request(b);
    while ((b->flags & (Drive_Buffer_Read | Drive_Buffer_Dirty)) != Drive_Buffer_Read){
        scheduler_yield();
    }
    return b;
}

bool ext2_verify(struct ext2_superblock *sb) {
    return sb->s_magic == EXT2_Magic;
}



void ext2_get_block_descriptor() {

}

void ext2_show_superblock(void) {
    struct drive_buffer *b = ext2_read_block(0, 0);
    if (b == NULL) {
        logf(Log_Error, "Could not allocate drive buffer\n");
    }
    struct ext2_superblock *sb = (struct ext2_superblock *)&b->data[Drive_Sector_Size * 2];
    logf(Log_Info, "Is valid super block? %s\n", ext2_verify(sb) ? "Yes" : "No");
    logf(Log_Debug, "Superblock:\n"
    "\ts_inodes_count: %x\n"
    "\ts_blocks_count: %x\n"
    "\ts_r_blocks_count: %x\n"
    "\ts_free_blocks_count: %x\n"
    "\ts_free_inodes_count: %x\n"
    "\ts_first_data_block: %x\n"
    "\ts_log_block_size: %x\n"
    "\ts_log_frag_size: %x\n"
    "\ts_blocks_per_group: %x\n"
    "\ts_frags_per_group: %x\n"
    "\ts_inodes_per_group: %x\n"
    "\ts_mtime: %x\n"
    "\ts_wtime: %x\n"
    "\ts_mnt_count: %x\n"
    "\ts_max_mnt_count: %hx\n"
    "\ts_magic: %hx\n"
    "\ts_state: %hx\n"
    "\ts_errors: %hx\n"
    "\ts_minor_rev_level: %hx\n"
    "\ts_lastcheck: %x\n"
    "\ts_creator_os: %x\n"
    "\ts_rev_level: %x\n"
    "\ts_def_resuid: %hx\n"
    "\ts_def_reguid: %hx\n",
    sb->s_inodes_count,
    sb->s_blocks_count,
    sb->s_r_blocks_count,
    sb->s_free_blocks_count,
    sb->s_free_inodes_count,
    sb->s_first_data_block,
    sb->s_log_block_size,
    sb->s_log_frag_size,
    sb->s_blocks_per_group,
    sb->s_frags_per_group,
    sb->s_inodes_per_group,
    sb->s_mtime,
    sb->s_wtime,
    sb->s_mnt_count,
    sb->s_max_mnt_count,
    sb->s_magic,
    sb->s_state,
    sb->s_errors,
    sb->s_minor_rev_level,
    sb->s_lastcheck,
    sb->s_creator_os,
    sb->s_rev_level,
    sb->s_def_resuid,
    sb->s_def_reguid);
}

constructor enum status ext2_init(void) {
    log(Log_Debug, "ext2 init");
    ext2_show_superblock();
}
