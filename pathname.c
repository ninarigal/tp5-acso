
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/**
 * TODO
 */
// int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
//     //Implement code here
//     // me dan un absolut path (desde root) 
//     // no hay que manejar el caso de // -> linux lo maneja como una /
//     // se que el inode del ROOT es el 1 --> adentro del uno busco usr -> adentro de usr busco bin -> adentro de bin busco bash -> buscas los dirents usando la funcion de los dirents (para el ejemplo /usr/bin/bash)
//     // devolves el inumber del dirent de bash
//     char* pathname_copy = strdup(pathname);
//     char* dir = strtok(pathname_copy, "/");
//     int inumber = 1; // el root inode es 1
//     struct direntv6 * dirent = malloc(sizeof(struct direntv6));
//     if (dirent == NULL) {
//         return -1;
//     }
//     while (dir != NULL) {
//         int found = directory_findname(fs, dir, inumber, dirent);
//         if (found == -1) {
//             free(dirent);
//             return -1;
//         }
//         inumber = dirent->d_inumber;
//         dir = strtok(NULL, "/");
//     }
//     free(dirent);
//     return inumber;
// }
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    char *pathname_copy = strdup(pathname);
    if (pathname_copy == NULL) {
        return -1;
    }

    char *dir = strtok(pathname_copy, "/");
    int inumber = 1; // El inode del root es 1
    struct direntv6 dirent;

    while (dir != NULL) {
        int found = directory_findname(fs, dir, inumber, &dirent);
        if (found == -1) {
            free(pathname_copy);
            return -1;
        }
        inumber = dirent.d_inumber;
        dir = strtok(NULL, "/");
    }

    free(pathname_copy);
    return inumber;
}

