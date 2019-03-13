#ifndef SYSOPYFIND_H
#define SYSOPYFIND_H

#include <stdlib.h>

#ifndef SYSOPYFIND_C

extern char **block_array;

extern size_t block_array_length, block_count;

extern const char *searched_dir;
extern const char *searched_file;

extern const char *last_tmpfile;

#endif

char **sysopyfind_create_array(size_t size);

void sysopyfind_free_array(void);

const char *sysopyfind_set_dir(const char *dir);

const char *sysopyfind_set_file(const char *file);

int sysopyfind_invoke(const char *tmpfile_name);

ssize_t sysopyfind_load_file(const char *file_name, ssize_t *filesize);

ssize_t sysopyfind_load_last_tmpfile(ssize_t *filesize);

void sysopyfind_delete_block(size_t index);

#endif // SYSOPYFIND_H
