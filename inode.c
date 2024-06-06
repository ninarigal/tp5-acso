#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"


/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    void *buff= malloc(DISKIMG_SECTOR_SIZE);
    if (buff == NULL) {
        return -1;
    }
    int blocks_per_sector = DISKIMG_SECTOR_SIZE / sizeof(struct inode); // 256
    int real_inumber = inumber - 1; // real inumber (index starts from 0)
    int sectorNum = INODE_START_SECTOR + real_inumber / blocks_per_sector; 
    if (diskimg_readsector(fs->dfd, sectorNum, buff) == -1) { // read the sector where the inode is
        free(buff);
        return -1;
    }
    int inode_size = sizeof(struct inode);
    memcpy(inp, buff + (real_inumber % blocks_per_sector) * inode_size, inode_size); // copy the inode to inp
    free(buff);
    return 0;
}


/**
 * TODO
 */
// int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
//     int is_small_file = ((inp->i_mode & ILARG) == 0); // check if it is a small file
//     if (is_small_file) {
//         if (blockNum < 7) { // if it is a small file and the blockNum is less than 7, it is a direct block
//             return inp->i_addr[blockNum];
//         } else {
//             return -1;
//         }
//     }
//     // if not a small file, it is a large file
//     int blocks_per_sector = DISKIMG_SECTOR_SIZE / sizeof(uint16_t); // 256
//     int first_indir_limit = blocks_per_sector * 7; // 1792
//     void *buffer_1 = malloc(DISKIMG_SECTOR_SIZE);
//     if (buffer_1 == NULL) {
//         return -1;
//     }
//     if (blockNum < first_indir_limit) { // if the blockNum is less than 1792, it is a first level of indirection
//         int indir_index = blockNum / blocks_per_sector; // index of the indirect block
//         int offset = blockNum % blocks_per_sector; // offset in the indirect block
//         if (diskimg_readsector(fs->dfd, inp->i_addr[indir_index], buffer_1) == -1) {
//             free(buffer_1);
//             return -1; 
//         }
//         int block_number = ((uint16_t *)buffer_1)[offset];
//         free(buffer_1);
//         return block_number;
//     } else { // if the blockNum is greater than 1792, it is a second level of indirection
//         if (diskimg_readsector(fs->dfd, inp->i_addr[7], buffer_1) == -1) { // read the 8th block (first level indirect block)
//             free(buffer_1);
//             return -1; 
//         }
//         void *buffer_2 = malloc(DISKIMG_SECTOR_SIZE);
//         if (buffer_2 == NULL) {
//             free(buffer_1);
//             return -1; 
//         }
//         int second_indir_index = (blockNum - first_indir_limit) / blocks_per_sector; // index of the second level indirect block
//         if (diskimg_readsector(fs->dfd, ((uint16_t *)buffer_1)[second_indir_index], buffer_2) == -1) { // read the second level indirect block
//             free(buffer_1);
//             free(buffer_2);
//             return -1; 
//         }
//         int offset = (blockNum - first_indir_limit) % blocks_per_sector; // offset in the second level indirect block
//         int block_number = ((uint16_t *)buffer_2)[offset]; // get the block number
//         free(buffer_1);
//         free(buffer_2);
//         return block_number;
//     }
// }
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    // Determine if the file is small or large
    int is_small_file = ((inp->i_mode & ILARG) == 0);
    
    if (is_small_file) {
        // Small file: Direct addressing for the first 7 blocks
        if (blockNum < 7) {
            return inp->i_addr[blockNum];
        } else {
            return -1;  // Block number out of range for small file
        }
    }

    // Large file handling
    int blocks_per_sector = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);
    int first_indirect_limit = blocks_per_sector * 7;

    if (blockNum < first_indirect_limit) {
        // First level of indirection
        int indir_index = blockNum / blocks_per_sector;
        int offset = blockNum % blocks_per_sector;

        // Allocate buffer for sector read
        void *buffer = malloc(DISKIMG_SECTOR_SIZE);
        if (buffer == NULL) {
            return -1;  // Memory allocation failed
        }

        // Read the indirect block
        if (diskimg_readsector(fs->dfd, inp->i_addr[indir_index], buffer) == -1) {
            free(buffer);
            return -1;  // Disk read failed
        }

        // Retrieve the block number from the indirect block
        int block_number = ((uint16_t *)buffer)[offset];
        free(buffer);
        return block_number;

    } else {
        // Second level of indirection
        int second_level_start = first_indirect_limit;
        int indir_index = 7;
        
        // Allocate buffer for first level indirect block
        void *buffer_1 = malloc(DISKIMG_SECTOR_SIZE);
        if (buffer_1 == NULL) {
            return -1;  // Memory allocation failed
        }

        // Read the first level indirect block
        if (diskimg_readsector(fs->dfd, inp->i_addr[indir_index], buffer_1) == -1) {
            free(buffer_1);
            return -1;  // Disk read failed
        }

        // Calculate indices for second level
        int second_indir_index = (blockNum - second_level_start) / blocks_per_sector;
        int offset = (blockNum - second_level_start) % blocks_per_sector;

        // Allocate buffer for second level indirect block
        void *buffer_2 = malloc(DISKIMG_SECTOR_SIZE);
        if (buffer_2 == NULL) {
            free(buffer_1);
            return -1;  // Memory allocation failed
        }

        // Read the second level indirect block
        if (diskimg_readsector(fs->dfd, ((uint16_t *)buffer_1)[second_indir_index], buffer_2) == -1) {
            free(buffer_1);
            free(buffer_2);
            return -1;  // Disk read failed
        }

        // Retrieve the block number from the second level indirect block
        int block_number = ((uint16_t *)buffer_2)[offset];
        free(buffer_1);
        free(buffer_2);
        return block_number;
    }
}


int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1);  // devuelve la cantidad de bytes que tiene el archivo 
}
