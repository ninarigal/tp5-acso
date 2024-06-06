
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
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    char *pathname_ = strdup(pathname); 
    if (pathname_ == NULL) {
        return -1;
    }
    char *directory = strtok(pathname_, "/"); // split the pathname by "/"
    int inumber = 1; // root inode is 1
    struct direntv6* dirent = malloc(sizeof(struct direntv6));
    if (dirent == NULL) {
        free(pathname_);
        return -1;
    }
    while (directory != NULL) { // while there are directories to read
        if (directory_findname(fs, directory, inumber, dirent) == -1) {
            free(pathname_);
            free(dirent);
            return -1;
        }
        inumber = dirent->d_inumber; // update the inumber
        directory = strtok(NULL, "/"); // read the next directory
    }
    free(pathname_);
    free(dirent);
    return inumber; // return the inumber of the last directory
}

