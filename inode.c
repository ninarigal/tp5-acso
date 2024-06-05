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
        if (blockNum < 7) { // si es un archivo chico y el bloque que quiero leer esta en los primeros 7 bloques
            return inp->i_addr[blockNum];
        } else {
            return -1;
        }
    }

    // large file
    
    if (blockNum < (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) * 7) { // 512 / 2 * 7 (entran 256 bloques por sector) -> esta en los primeros 7 (1er nivel de indireccionamiento)
        int first_block_num = blockNum / (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));     
        void *buff= malloc(DISKIMG_SECTOR_SIZE);
        if (buff == NULL) {
            return -1;
        }
        if (diskimg_readsector(fs->dfd, inp->i_addr[first_block_num], buff) == -1) {
            free(buff);
            return -1;
        }
        uint16_t *table = (uint16_t *)buff;
        int block_number = table[blockNum % (DISKIMG_SECTOR_SIZE / sizeof(uint16_t))];
        free(buff);
        return block_number;
   
    } else { // sino hay dos niveles de indireccionamiento
        void *buff_1= malloc(DISKIMG_SECTOR_SIZE);
        if (buff_1 == NULL) {
            return -1;
        }
        int first_block_num = 7;
        if (diskimg_readsector(fs->dfd, inp->i_addr[first_block_num], buff_1) == -1) { // en el último inode
            free(buff_1);
            return -1;
        }
        uint16_t *table_1 = (uint16_t *)buff_1; 
        void *buff_2= malloc(DISKIMG_SECTOR_SIZE);
        if (buff_2 == NULL) {
            free(buff_1);
            return -1;
        }
        int sec_block_num = (blockNum - (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) * 7) / (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));
        if (diskimg_readsector(fs->dfd, table_1[sec_block_num], buff_2) == -1) {
            free(buff_1);
            free(buff_2);
            return -1;
        }
        uint16_t *table_2 = (uint16_t *)buff_2;
        int third_block_num = (blockNum - (DISKIMG_SECTOR_SIZE / sizeof(uint16_t)) * 7) % (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));
        int block_number = table_2[third_block_num];
        free(buff_1);
        free(buff_2);
        return block_number;
    }
    
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1);  // devuelve la cantidad de bytes que tiene el archivo 
}
