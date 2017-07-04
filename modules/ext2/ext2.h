#pragma once

#include <truth/types.h>

// http://www.nongnu.org/ext2-doc/ext2.html

#define EXT2_Block_Size (1 * KB)
#define EXT2_UUID_Size 16
#define EXT2_Volume_Name_Size 16
#define EXT2_Last_Mounted_Size 64
#define EXT2_Hash_Seed_Size (4 * 4)

#define EXT2_OS_Linux   0
#define EXT2_OS_Hurd    1
#define EXT2_OS_Masix   2
#define EXT2_OS_FreeBSD 3
#define EXT2_OS_Lites   4

#define EXT2_Good_Old_Rev 0
#define EXT2_Dynamic_Rev 1

#define EXT2_Valid_FS 1
#define EXT2_Error_FS 2
#define EXT2_Super_Magic 0xef53

#define EXT2_Errors_Continue 1
#define EXT2_Errors_RO       2
#define EXT2_Errors_Panic    3

#define EXT2_Feature_Incompat_Compression 0x0001
#define EXT2_Feature_Incompat_Filetype 0x0002
#define EXT3_Feature_Incompat_Recover 0x0004
#define EXT3_Feature_Incompat_Journal_Dev 0x0008
#define EXT2_Feature_Incompat_Meta_BG 0x0010

#define EXT2_Feature_Compat_Dir_Prealloc  0x0001
#define EXT2_Feature_Compat_IMagic_INodes 0x0002
#define EXT3_Feature_Compat_Has_Journal   0x0004
#define EXT2_Feature_Compat_Ext_Attr      0x0008
#define EXT2_Feature_Compat_Resize_Ino    0x0010
#define EXT2_Feature_Compat_Dir_Index     0x0020

#define EXT2_Feature_RO_Compat_Sparse_Super 0x0001
#define EXT2_Feature_RO_Compat_Large_File 0x0002
#define EXT2_Feature_RO_Compat_BTree_Dir 0x0004

#define EXT2_LZV1_Alg   0x00000001
#define EXT2_LZRW3A_Alg 0x00000002
#define EXT2_GZIP_Alg   0x00000004
#define EXT2_BZIP2_Alg  0x00000008
#define EXT2_LZO_Alg    0x00000010




struct ext2_superblock {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint32_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_reguid;
    // EXT2_DYNAMIC_REV Specific
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t s_uuid[EXT2_UUID_Size];
    uint8_t s_volume_name[EXT2_Volume_Name_Size];
    uint8_t s_last_mounted[EXT2_Last_Mounted_Size];
    uint32_t s_algo_bitmap;
    // Performance hints
    uint8_t s_prealloc_blocks;
    uint8_t s_prealloc_dir_blocks;
    uint16_t __alignment0;
    // Journaling support
    uint16_t s_journal_uuid;
    uint32_t s_journal_inum;
    uint32_t s_journal_dev;
    uint32_t s_last_orphan;
    // Directory indexing sypport
    uint8_t s_hash_seed[EXT2_Hash_Seed_Size];
    uint8_t s_def_hash_version;
    uint8_t __alignment1;
    uint8_t __alignment2;
    uint8_t __alignment3;
    // Other options
    uint32_t s_default_mount_points;
    uint32_t s_first_meta_bg;
    // All others reserved.
};

#define EXT2_BG_Reserved_Size 12
struct block_group_descriptor_table {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint8_t bg_reserved[EXT2_BG_Reserved_Size];
};

#define EXT2_INode_Table_Blocks 15
#define EXT2_INode_Table_OS_Data_Size 12
struct inode_table {
    uint16_t i_mode;
    uint16_t i_uid;
    uint16_t i_size;
    uint16_t i_atime;
    uint16_t i_ctime;
    uint16_t i_mtime;
    uint16_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_ods1;
    uint32_t i_block[EXT2_INode_Table_Blocks];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint8_t i_osd2[EXT2_INode_Table_OS_Data_Size]
};

#define EXT2_Bad_INO         1
#define EXT2_Root_INO        2
#define EXT2_ACL_IDX_INO     3
#define EXT2_ACL_Data_INO    4
#define EXT2_Boot_Loader_INO 5
#define EXT2_Undel_Dir_INO   6

// -- file format --
#define EXT2_S_IFSOCK   0xC000
#define EXT2_S_IFLNK    0xA000
#define EXT2_S_IFREG    0x8000
#define EXT2_S_IFBLK    0x6000
#define EXT2_S_IFDIR    0x4000
#define EXT2_S_IFCHR    0x2000
#define EXT2_S_IFIFO    0x1000
#define // -- process execution user/group override --
#define EXT2_S_ISUID    0x0800
#define EXT2_S_ISGID    0x0400
#define EXT2_S_ISVTX    0x0200
#define // -- access rights --
#define EXT2_S_IRUSR    0x0100
#define EXT2_S_IWUSR    0x0080
#define EXT2_S_IXUSR    0x0040
#define EXT2_S_IRGRP    0x0020
#define EXT2_S_IWGRP    0x0010
#define EXT2_S_IXGRP    0x0008
#define EXT2_S_IROTH    0x0004
#define EXT2_S_IWOTH    0x0002
#define EXT2_S_IXOTH    0x0001

#define EXT2_SecRM_FL     0x00000001
#define EXT2_UNRM_FL      0x00000002
#define EXT2_Compr_FL     0x00000004
#define EXT2_Sync_FL      0x00000008
#define EXT2_Immutable_FL 0x00000010
#define EXT2_Append_FL    0x00000020
#define EXT2_NoDump_FL    0x00000040
#define EXT2_NoAtime_FL   0x00000080
// -- Reserved for compression usage --
#define EXT2_Dirty_FL    0x00000100
#define EXT2_ComprBlk_FL 0x00000200
#define EXT2_NoCompr_FL  0x00000400
#define EXT2_ECompr_FL   0x00000800
// -- End of compression flags --
#define EXT2_BTree_FL        0x00001000
#define EXT2_Index_FL        0x00001000
#define EXT2_IMagic_FL       0x00002000
#define EXT3_Journal_Data_FL 0x00004000
#define EXT2_Reserved_FL     0x80000000

#define EXT2_FT_Unknown  0
#define EXT2_FT_Reg_File 1
#define EXT2_FT_Dir      2
#define EXT2_FT_ChrDev   3
#define EXT2_FT_BlkDev   4
#define EXT2_FT_FIFO     5
#define EXT2_FT_Sock     6
#define EXT2_FT_Symlink  7

