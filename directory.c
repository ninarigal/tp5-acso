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
  struct inode* inp = malloc(sizeof(struct inode));
  if (inp == NULL) {
    return -1;
  }
  if (inode_iget(fs, dirinumber, inp) == -1) { // look for the inode corresponding to the dirinumber
    free(inp);
    return -1;
  }
  int is_directory = ((inp->i_mode & IFMT) == IFDIR); // check if it is a directory
  if (!is_directory) {
    free(inp);
    return -1;
  }
  int total_size = inode_getsize(inp);
  if(total_size <= 0) {
    free(inp);
    return -1;
  }
  int blocks = total_size / DISKIMG_SECTOR_SIZE; // number of blocks
  int last_block = total_size % DISKIMG_SECTOR_SIZE; // number of bytes in the last block
  if (last_block != 0) {
    blocks++;
  }
  int dirent_size = sizeof(struct direntv6);
  void* buff = malloc(DISKIMG_SECTOR_SIZE);
  if (buff == NULL) {
    free(inp);
    return -1;
  }
  for (int i = 0; i < blocks; i++) {
        int valid_bytes = file_getblock(fs, dirinumber, i, buff); // save in buff the content of the directory specified
        if (valid_bytes == -1) { 
            free(inp);
            free(buff);
            return -1;
        }
        int dirents = valid_bytes / dirent_size; 
        for (int j = 0; j < dirents; j++) {
            struct direntv6 *dirent_j = (struct direntv6 *)(buff + j * dirent_size);
            if (strcmp(dirent_j->d_name, name) == 0) {
                memcpy(dirEnt, dirent_j, dirent_size);
                free(inp);
                free(buff);
                return 0;
            }
        }
    }
    free(buff);
    free(inp);
    return -1;
}
//   for (int i = 0; i < blocks; i++) {
//     int valid_bytes = file_getblock(fs, dirinumber, i, buff); // save in buff the content of the directory specified
//     if (valid_bytes == -1) { 
//       free(inp);
//       free(buff);
//       return -1;
//     }
//     int dirents = valid_bytes / dirent_size; 
//     struct direntv6* dirent = malloc(sizeof(struct direntv6));
//     if (dirent == NULL) {
//       free(inp);
//       free(buff);
//       return -1;
//     }
//     for (int j = 0; j < dirents ; j++) {
//       memcpy(dirent, buff + j * dirent_size, dirent_size);
//       if (strcmp(dirent->d_name, name) == 0) {
//         memcpy(dirEnt, dirent, dirent_size);
//         free(inp);
//         free(buff);
//         free(dirent);
//         return 0;
//       }
      
//     }
//     free(dirent);
//   }
//   free(buff);
//   free(inp);
  
//   return -1;
// }
  


//       struct direntv6 dirent_j = ((struct direntv6*) buff)[j]; // agarro el dirent j del bloque i
//       if (strcmp(dirent_j.d_name, name) == 0) {
//         memcpy(dirEnt, &dirent_j, dirent_size);
//         free(inp);
//         free(buff);
//         return 0;
//       }
//     }
//   }
//   free(buff);
//   free(inp);
//   return -1;
// }
