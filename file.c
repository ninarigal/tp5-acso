#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode *inp = malloc(sizeof(struct inode));
    if (inp == NULL) {
        return -1;
    }
    if (inode_iget(fs, inumber, inp) == -1) { // get the inode of the file
        free(inp);
        return -1;
    }
    int sector = inode_indexlookup(fs, inp, blockNum); // get the sector of the block
    if (sector == -1) {
        free(inp);
        return -1;
    }
    if (diskimg_readsector(fs->dfd, sector, buf) == -1) { // read the sector and store it in buf
        free(inp);
        return -1;
    }
    int total_size = inode_getsize(inp);
    if (total_size <= 0) {
        free(inp);
        return -1;
    }
    int blocks = total_size / DISKIMG_SECTOR_SIZE; // number of blocks of the file
    if (blockNum < blocks) { 
        return DISKIMG_SECTOR_SIZE; // if not last block, return full size
    } else{
        return total_size % DISKIMG_SECTOR_SIZE; // if last block, return remaining size
    }
}

