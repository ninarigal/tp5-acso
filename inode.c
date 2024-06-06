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
    //Implement Code Here
    // quiero leer el inode que me pasan 
    // inumber es el numero de inode 
    // inp es donde quiero escribir -> meter el inode (es alocado afuera no tenes que liberar)
    // inodes empiezan desde el 2 
    // cuenta para obtener el numero de sector, leerlo y guardarlo en imp -> obtener el contenido del inumber q me pasan
    // por sector entran 16 inodes

    void *buff= malloc(DISKIMG_SECTOR_SIZE);
    if (buff == NULL) {
        return -1;
    }
    if (diskimg_readsector(fs->dfd, INODE_START_SECTOR + (inumber - 1) / ( DISKIMG_SECTOR_SIZE / sizeof(struct inode)), buff) == -1) {
        free(buff);
        return -1;
    }
    memcpy(inp, buff + ((inumber - 1) % (DISKIMG_SECTOR_SIZE / sizeof(struct inode))) * sizeof(struct inode), sizeof(struct inode));
    // memcpy(inp, buff + ((inumber - 1) % 16) * sizeof(struct inode), sizeof(struct inode));
    // *inp = ((struct inode *) buff)[(inumber - 1) % 16];
    free(buff);
    return 0;
}



/**
 * TODO
 */
// int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {  
//     //Implement code here
//     // archivo distribuido en distintos bloques 
//     // blockNum es el indice del numero de bloque que quiero leer
//     // inp es el inodo que quiero leer -> dentro de la estructura inode tiene un atrubuto imode que me dice si es large file
//     // fs es el sistema de archivos
//     // quiero leer el bloque blockNum del inodo inp
//     // si es un archivo chico -> los primeros 7 bloques estan en el inodo
//     // si es un archivo grande -> los primeros 7 bloques estan en el inodo y el octavo esta en un bloque que esta en el inodo

//     int small_file = ((inp->i_mode & ILARG) == 0);
//     if (small_file) {
//         if (blockNum < 7) { // si es un archivo chico y el bloque que quiero leer esta en los primeros 7 bloques
//             return inp->i_addr[blockNum];
//         } else {
//             return -1;
//         }
//     }

//     // large file
    
//     int first_indir_bytes = (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) * 7;
//     if (blockNum < first_indir_bytes) { // 512 / 2 * 7 (entran 256 bloques por sector) -> esta en los primeros 7 (1er nivel de indireccionamiento)
//         int first_block_num = blockNum / (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));     
//         void *buff= malloc(DISKIMG_SECTOR_SIZE);
//         if (buff == NULL) {
//             return -1;
//         }
//         if (diskimg_readsector(fs->dfd, inp->i_addr[first_block_num], buff) == -1) {
//             free(buff);
//             return -1;
//         }
//         int second_block_num = blockNum % (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));
//         // uint16_t *table = (uint16_t *)buff;
//         int block_number = ((uint16_t *)buff)[second_block_num];
//         free(buff);
//         return block_number;
   
//     } else { // sino hay dos niveles de indireccionamiento
//         void *buff_1= malloc(DISKIMG_SECTOR_SIZE);
//         if (buff_1 == NULL) {
//             return -1;
//         }
//         int first_block_num = 7;
//         if (diskimg_readsector(fs->dfd, inp->i_addr[first_block_num], buff_1) == -1) { // en el último inode
//             free(buff_1);
//             return -1;
//         }
//         // uint16_t *table_1 = (uint16_t *)buff_1; 
//         void *buff_2= malloc(DISKIMG_SECTOR_SIZE);
//         if (buff_2 == NULL) {
//             free(buff_1);
//             return -1;
//         }
//         int sec_block_num = (blockNum - (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) * 7) / (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));
//         if (diskimg_readsector(fs->dfd, ((uint16_t *)buff_1)[sec_block_num], buff_2) == -1) {
//             free(buff_1);
//             free(buff_2);
//             return -1;
//         }
//         // uint16_t *table_2 = (uint16_t *)buff_2;
//         int third_block_num = (blockNum - (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) * 7) % (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));
//         int block_number = ((uint16_t *)buff_2)[third_block_num];
//         free(buff_1);
//         free(buff_2);
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
