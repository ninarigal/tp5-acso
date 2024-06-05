#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"


/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    //Implement Code Here
    // quiero leer el inode que me pasan 
    // inumber es el numero de inode 
    // inp es donde quiero escribir -> meter el inode (es alocado afuera no tenes que liberar)
    // inodes empiezan desde el 2 
    // cuenta para obtener el numero de sector, leerlo y guardarlo en imp -> obtener el contenido del inumber q me pasan
    // por sector entran 16 inodes

    // // 1. calcular el sector donde esta el inodo
    // int sector = (inumber / 16) + INODE_START_SECTOR;
    // // 2. leer el sector
    // if (diskimg_readsector(fs->dfd, sector, inp) != DISKIMG_SECTOR_SIZE) {
    //     perror("Error reading inode sector");
    //     return -1;
    // }
    // // 3. calcular el offset del inodo dentro del sector
    // int offset = (inumber % 16) * sizeof(struct inode);
    // // 4. mover el puntero al inodo correcto
    // inp += offset;
    // return 0; 

    void *buff= malloc(DISKIMG_SECTOR_SIZE);
    if (buff == NULL) {
        return -1;
    }
    if (diskimg_readsector(fs->dfd, INODE_START_SECTOR + (inumber - 1) / 16, buff) == -1) {
        return -1;
    }
    // memcpy(inp, buff + ((inumber - 1) % 16) * sizeof(struct inode), sizeof(struct inode));
    *inp = ((struct inode *) buff)[(inumber - 1) % 16];
    free(buff);
    return 0;
}


/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {  
    //Implement code here
    // archivo distribuido en distintos bloques 
    // blockNum es el indice del numero de bloque que quiero leer
    // inp es el inodo que quiero leer -> dentro de la estructura inode tiene un atrubuto imode que me dice si es large file
    // fs es el sistema de archivos
    // quiero leer el bloque blockNum del inodo inp
    // si es un archivo chico -> los primeros 7 bloques estan en el inodo
    // si es un archivo grande -> los primeros 7 bloques estan en el inodo y el octavo esta en un bloque que esta en el inodo

    int small_file = ((inp->i_mode & ILARG) == 0);
    if (small_file) {            
        if (blockNum < 7) {
            return inp->i_addr[blockNum];
        } else {
            return -1;
        }
    } else {
        // si esta en los primeros 7 bloques (solo un indireccionamiento)
        int blockIndex = blockNum / 256;
        if (blockIndex <= 7) {
            void *buff= malloc(DISKIMG_SECTOR_SIZE);
            if (buff == NULL) {
                return -1;
            }
            if (diskimg_readsector(fs->dfd, blockIndex, buff) == -1) {
                return -1;
            }
            // ahora iteramos sobre el buffer cada 2 bytes buscando blocknum
            int *block = (int *)buff;
            int i;
            int blockIndex2 = blockNum - (256 * (blockIndex - 1));
            for (i = 0; i < 256; i++) {
                int block = ((uint16_t *) buff)[i];
                if (block == blockIndex2) {
                    free(buff);
                    return block;
                }                   
            }
            free(buff);
            return -1;
        } else {
            // si esta en el octavo bloque (doble indireccionamiento)
            void *buff= malloc(DISKIMG_SECTOR_SIZE);
            if (buff == NULL) {
                return -1;
            }
            if (diskimg_readsector(fs->dfd, blockIndex, buff) == -1) {
                return -1;
            }
            int *block = (int *)buff;
            int i;
            int blockIndex2 = blockNum - (256 * (blockIndex - 1));
            for (i = 0; i < 256; i++) {
                int block = ((uint16_t *) buff)[i];
                if (block == blockIndex2) {
                    if (diskimg_readsector(fs->dfd, blockIndex, buff) == -1) {
                        return -1;
                    }
                    for (i = 0; i < 256; i++) {
                        int block = ((uint16_t *) buff)[i];
                        if (block == blockNum) {
                            free(buff);
                            return block;
                        }
                    }
                }                   
            }
            free(buff);
            return -1;              
        }
    }
}

// int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
// 	int fd = fs->dfd;
// 	int is_small_file = ((inp->i_mode & ILARG) == 0);

// 	// if it is a small file
// 	if(is_small_file) {
// 		return inp->i_addr[blockNum];
// 	}	

// 	// if it is a large file
// 	int addr_num = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);
// 	int indir_addr_num = addr_num * 7;
// 	if(blockNum < indir_addr_num) {		// if it only uses INDIR_ADDR
// 		int sector_offset = blockNum / addr_num;
// 		int addr_offset = blockNum % addr_num;
// 		uint16_t addrs[addr_num];
// 		int err = diskimg_readsector(fd, inp->i_addr[sector_offset], addrs);
// 		if(err < 0) return -1;	
// 		return addrs[addr_offset];
// 	} else {							// if it also uses the DOUBLE_INDIR_ADDR
// 		// the first layer
// 		int blockNum_in_double = blockNum - indir_addr_num;
// 		int sector_offset_1 = 7;
// 		int addr_offset_1 = blockNum_in_double / addr_num;
// 		uint16_t addrs_1[addr_num];
// 		int err_1 = diskimg_readsector(fd, inp->i_addr[sector_offset_1], addrs_1);
// 		if(err_1 < 0) return -1;

// 		// the second layer
// 		int sector_2 = addrs_1[addr_offset_1];
// 		int addr_offset_2 = blockNum_in_double % addr_num;
// 		uint16_t addrs_2[addr_num];
// 		int err_2 = diskimg_readsector(fd, sector_2, addrs_2);
// 		if(err_2 < 0) return -1;
// 		return addrs_2[addr_offset_2];
// 	}	
// }

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1);  // devuelve la cantidad de bytes que tiene el archivo 
}
