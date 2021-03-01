#ifndef __SFS_H
#define __SFS_H

#include <time.h>
#include <sys/stat.h>

#define DOCROOT "public_html"

struct sfile {
    int fd;
    int size;
    time_t last_mod;
//    char *type;
    char *body;
    char *path;
};

int sfs_set_file(char *uri, struct sfile *file, int *code);
int sfs_get_code(char *uri, struct stat *st, struct sfile *file); 
int sfs_find_file(char *uri, struct stat *st, struct sfile *file);

char *sfs_get_path(char *uri);
char *sfs_map_body(int fd, int size);






#endif
