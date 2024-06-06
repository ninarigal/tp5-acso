#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
  //Implement your code here
  // busca un archivo que se llame name en el directorio que esta representado por el inode dirinumber (numero de inode de un directorio)
  // vas al bloque (como es un directorio lo interptetas como dirent) -> buscas el nombre que te pasan
  // dirEnt es un output parameter -> guardas ahi el dirent que corresponde al nombre que buscas
  // si no lo encontras devolves -1

  // te dan un nombre e iteras por dirents hasta encontrar el nombre que buscas (no sabes cual es el inode)

  // struct inode *inp = malloc(sizeof(struct inode));
  // if (inp == NULL) {
  //   return -1;
  // }
  // if (inode_iget(fs, dirinumber, inp) == -1) {
  //   free(inp);
  //   return -1;
  // }
  // int is_dir = ((inp->i_mode & IFMT) == IFDIR);
  // if (!is_dir) {
  //   free(inp);
  //   return -1;
  // }
  // int total_size = inode_getsize(inp);
  // if (total_size <= 0) {
  //   free(inp);
  //   return -1;
  // }
  // int complete_blocks = total_size / DISKIMG_SECTOR_SIZE;
  // int blocknums_in_last_block = total_size % DISKIMG_SECTOR_SIZE;
  // if (blocknums_in_last_block != 0) {
  //   complete_blocks++;
  // }
  // int dirent_size = sizeof(struct direntv6);
  // void *buff = malloc(DISKIMG_SECTOR_SIZE);
  // if (buff == NULL) {
  //   free(inp);
  //   return -1;
  // }
  // for (int i = 0; i < complete_blocks; i++) {
  //   int valid_bytes = file_getblock(fs, dirinumber, i, buff);
  //   if (valid_bytes == -1) {
  //     free(inp);
  //     free(buff);
  //     return -1;
  //   }
  //   int dirents = valid_bytes / dirent_size;
  //   for (int j = 0; j < dirents; j++) {
  //     struct direntv6 *dirent = (struct direntv6 *)(buff + j * dirent_size);
  //     if (strcmp(dirent->d_name, name) == 0) {
  //       memcpy(dirEnt, dirent, dirent_size);
  //       free(inp);
  //       free(buff);
  //       return 0;
  //     }
  //   }
  // }
  // free(inp);
  // free(buff);
  // return -1;
  
  struct inode* inp = malloc(sizeof(struct inode));
  if (inp == NULL) {
    return -1;
  }
  if (inode_iget(fs, dirinumber, inp) == -1) { // Busco el inode que le corresponde a este directorio (al directorio con el numero dirinumber)
    free(inp);
    return -1;
  }
  // checkeo que efectivamente sea un directorio
  int is_dir = ((inp->i_mode & IFMT) == IFDIR);
  if (!is_dir) {
    free(inp);
    return -1;
  }
  int size = inode_getsize(inp);
  if(size <= 0) {
    free(inp);
    return -1;
  }

  int complete_blocks = size/DISKIMG_SECTOR_SIZE; // un bloque tiene 512 bytes, que representan 256 numeros de bloques. Esto me da la cantidad de bloques completos que tiene el archivo.
  int blocknums_in_last_block = size % DISKIMG_SECTOR_SIZE; // me fijo si hay un ultimo bloque que no esta completo

  if (blocknums_in_last_block != 0) {  // Si hay uno, aunque no este lleno lo tengo que agregar
    complete_blocks++;
  }
  int blocks = complete_blocks; // La cantidad de bloques que tiene el archivo

  int dirent_size = sizeof(struct direntv6);
  void* buff = malloc(DISKIMG_SECTOR_SIZE);
  if (buff == NULL) {
    free(inp);
    return -1;
  }
  int fd = fs->dfd;
  // vamos por cada bloque
  for (int i = 0; i < blocks; i++) {
    int valid_bytes = file_getblock(fs, dirinumber, i, buff); // guardamos en buff el contenido del directorio que me especificaron (con dirinumber)
    if (valid_bytes == -1) { // si no se pudo leer el bloque, devuelvo error
      free(inp);
      free(buff);
      return -1;
    }
    int dirents = valid_bytes/dirent_size; // la cantidad de dirents que tiene el bloque
    for (int j = 0; j < dirents ; j++) {
      struct direntv6 dirent_j = ((struct direntv6*) buff)[j]; // agarro el dirent j del bloque i
      if (strcmp(dirent_j.d_name, name) == 0) {
        memcpy(dirEnt, &dirent_j, dirent_size);
        free(inp);
        free(buff);
        return 0;
      }
    }
  }
  // Si no lo encontro en ningun dirent, devuelvo error
  free(buff);
  free(inp);
  return -1;
}
