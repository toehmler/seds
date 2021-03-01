/* 
 * sfs.c
 *
 * Interface for performing file i/o for http requests 
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include "inc/sfs.h"


/* sfs_set_file: attempts to initialize file struct from a given uri
 * @param: uri where the file is located in the docroot
 * @param: file a ptr to a sfile struct that will be initialized if file found 
 * @returns: 0 on success (whether found or not), -1 on err */
int sfs_set_file(char *uri, struct sfile *file, int *code) {
    struct stat st;
    if ((*code = sfs_get_code(uri, &st, file)) < 0) {
        return -1;
    }
    if (*code == 200) {
        if (sfs_find_file(uri, &st, file) < 0) {
            return -1;
        }
    } else { file = NULL; }
    return 0;
}

/* bfs_map_body: maps the contents of a file to memory
 * @note: path is assumed to be correct
 * @param: path a string for the full path of a file
 * @param: size the size of the file in bytes 
 * @returns: pointer to mapped memory, NULL on err */
char *sfs_map_body(int fd, int size) {
    char *body, *buf;
    if ((body = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        close(fd);
        return NULL;
    }
    if ((buf = malloc(size)) == NULL) {
        close(fd);
        return NULL;
    }
    memcpy(buf, body, size);
    munmap(body,size);
    close(fd);
    return buf;
}

int sfs_find_file(char *uri, struct stat *st, struct sfile *file) {
    file->size = st->st_size;
    file->last_mod = st->st_mtimespec.tv_sec;
    if ((file->fd = open(file->path, O_RDONLY)) < 0) {
        return -1;
    }
    if ((file->body = sfs_map_body(file->fd, st->st_size)) == NULL) {
        return -1;
    }
    return 0;
}

/* sfs_get_code: gets the code corresponding to a given path using stat
 * @param: uri a string of the uri from which a path will be constructed
 * @param: path a string of the full path to stat
 * @param: st a pointer to a stat struct 
 * @returns: the code corresponding a path, (200 = found, permissions ok; etc) */
int sfs_get_code(char *uri, struct stat *st, struct sfile *file) {
    int retv;
    char *path;
    if ((path = sfs_get_path(uri)) == NULL) {
        return -1;
    }
    if (stat(path, st) < 0) {
        switch(errno) {
            case EACCES:
                retv = 403;
                break;
            case ENOENT:
            case ENOTDIR:
                retv = 404;
                break;
            default: 
                retv = 500;
        }
    } else {
        if ((S_ISREG(st->st_mode)) && (st->st_mode & S_IROTH))
            retv = 200;
        else if (S_ISREG(st->st_mode))
            retv = 403;
        else
            retv = 404;
    }
    file->path = path;
    return retv;
}

/* sfs_get_path: constructs the full path for a file from a given uri
 * @param: uri a string representing a uri
 * @returns: pointer to an allocated string representing the path */
char *sfs_get_path(char *uri) {
    char *path;
    int path_len = snprintf(NULL, 0, "./%s%s", DOCROOT, uri);
    if ((path = malloc(path_len + 1)) == NULL)
        return NULL;
    if (snprintf(path, path_len + 1, "./%s%s", DOCROOT, uri) < 0) {
        free(path);
        return NULL;
    }
    return path;
}


