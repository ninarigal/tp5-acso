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
    // te dan el numero de inode y el numero de bloque -> cantidad de bytes validos en el blocqie
    // buf es un output parameter -> devolves guardando ahi (el contendiso se retorna en buf) -> esta alocado afuera 
    // lo que importa es cuantos bytes son validos 
    // el unico bloque que no tiene todos los bytes usados es el ultimo 
    // hay que retornar la cantidad de bytes
    //busca adentro de un file un bloque
    // usa inumber porque un archivo esta lleno de dirents (tienen el nombre del archivo en 14 bytes y el numero del inode) --> asi se accede al inode (nos dice donde en disco esta cada parte del archivo)

    // struct inode *inp = malloc(sizeof(struct inode));
    // if (inp == NULL) {
    //     return -1;
    // }
    // if (inode_iget(fs, inumber, &inp) == -1) {
    //     free(inp);
    //     return -1;
    // }
    // int block_index = inode_indexlookup(fs, inp, blockNum);
    // if (block_index == -1) {
    //     free(inp);
    //     return -1;
    // }
    // if (diskimg_readsector(fs->dfd, block_index, buf) == -1) {
    //     free(inp);
    //     return -1;
    // }
    // int total_size = inode_getsize(inp);
    // free(inp);
    // int blocks = total_size / DISKIMG_SECTOR_SIZE;
    // if (blockNum < blocks - 1) {
    //     return DISKIMG_SECTOR_SIZE;
    // } else {
    //     return total_size % DISKIMG_SECTOR_SIZE;
    // }
    // get inode content
	// struct inode my_inode;
	// int err = inode_iget(fs, inumber, &my_inode);
	// if(err < 0) return -1;

	// // get true block num
	// int sector = inode_indexlookup(fs, &my_inode, blockNum);
	// if(sector < 0) return -1;

	// // get block content
	// int read_err = diskimg_readsector(fs->dfd, sector, buf);
	// if(read_err < 0) return -1;

	// // get bytes and blocks
	// int total_bytes = inode_getsize(&my_inode);
	// if(total_bytes < 0) return -1;
	// int total_blocks = total_bytes / DISKIMG_SECTOR_SIZE;
	// if(blockNum == total_blocks) {
	// 	return total_bytes % DISKIMG_SECTOR_SIZE;
	// } else {
	// 	return DISKIMG_SECTOR_SIZE;
	// }

    struct inode *inp = malloc(sizeof(struct inode));
    if (inp == NULL) {
        return -1;
    }
    if (inode_iget(fs, inumber, inp) == -1) {
        free(inp);
        return -1;
    }
    int sector = inode_indexlookup(fs, inp, blockNum); 
    if (sector == -1) {
        free(inp);
        return -1;
    }
    if (diskimg_readsector(fs->dfd, sector, buf) == -1) {
        free(inp);
        return -1;
    }
    int total_size = inode_getsize(inp);
    if (total_size <= 0) {
        free(inp);
        return -1;
    }
    int blocks = total_size / DISKIMG_SECTOR_SIZE;
    // if (blockNum == blocks) {
    //     return total_size % DISKIMG_SECTOR_SIZE;
    // } else {
    //     return DISKIMG_SECTOR_SIZE;
    // }
    if (blockNum < blocks) { // Si es uno de los bloques del medio, usa todos los bytes
        return DISKIMG_SECTOR_SIZE;
    } else{
        return total_size % DISKIMG_SECTOR_SIZE; // Si es el ultimo bloque, no usa todos los bytes.
    }
}

