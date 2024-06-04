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

    //Disco --> memoria
    // Inode: 32 bytes. 16 inodes por bloque (512 bytes). 16 inodes por bloque.
    int sector = (INODE_START_SECTOR  + (int)((inumber - 1) / 16));
    int fd = fs->dfd;
    void *buff= malloc(DISKIMG_SECTOR_SIZE);
    if (buff == NULL) {
        return -1;
    }
    if (diskimg_readsector(fd, sector,  buff) == -1) {
        return -1;
    }
    memcpy(inp, buff + (int)((inumber - 1) % 16) * sizeof(struct inode), sizeof(struct inode));
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

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1);  // devuelve la cantidad de bytes que tiene el archivo 
}
