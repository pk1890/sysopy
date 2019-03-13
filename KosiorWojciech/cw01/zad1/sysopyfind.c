#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <sys/wait.h>

#define SYSOPYFIND_C

#include "sysopyfind.h"

// It is not specified whether the library is supposed to support
// creation of arbitrary number of block arrays or only work on
// one, global array.
// The first option would require making each array accompanied
// by a data structure. As only array and not structures
// are ever mentioned, the second option has been chosen.

// Programs are supposed not to modify the variables directly
// although they can read them

char **block_array = NULL;

// the following two have no meaning when block_array is NULL
size_t block_array_length, block_count;

const char *searched_dir = NULL;
const char *searched_file = NULL;

const char *last_tmpfile = NULL;

// For all functions constituting this library's api: on all
// errors reported by standard library functions errno set
// by them is preserved (when possible) and special value
// is returned (NULL and -1 for pointer and int-returning
// functions respectively unless specified otherwise)

// More than one function called by given routine may use
// the same error. The routine just preserves errno and returns
// without mechanisms to provide the caller with information
// about which function actually set the error

// Possible errors: errors from calloc() (ENOMEM)
char **sysopyfind_create_array(size_t size)
{
  // Forbid creation of a new array if old one exists
  if (block_array)
    errx(-1,"Attempt to allocate a new block array"
         " while old one exists.");
  
  // Forbid allocation of zero-sized array
  if (!size)
    errx(-1,"Attempt to allocate zero-sized array.");

  block_array_length = size;
  block_count = 0;
  
  // In case of lack of memory preserve errno and return NULL
  return block_array = calloc(size, sizeof(char*));
}

void sysopyfind_free_array(void)
{
  // Allow calling this function with no array
  if (!block_array) return;
  
  // Disallow freeing nonempty array
  if (block_count)
    errx(-1,"Attempt to free a nonempty array.");
  
  free(block_array);
  // Set pointer to indicate it's unallocated now
  block_array = NULL;
}

// Calling program is responsible for memory management of
// searched dir and searched file strings
// The strings have to be null-terminated
// Old value of appropriate variable is returned
const char *sysopyfind_set_dir(const char *dir)
{
  const char *oldval = searched_dir;
  searched_dir = dir;
  return oldval;
}

const char *sysopyfind_set_file(const char *file)
{
  const char *oldval = searched_file;
  searched_file = file;
  return oldval;
}

// returns 213 in case of some error, doesn't return otherwise
// 213 is just a magic number here - parent process will know
// that process exited before successful execlp
static int sysopyfind_child(const char *tmpfile_name)
{
  // child process code
  int fd;
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  do
    fd = open(tmpfile_name, O_WRONLY|O_CREAT|O_TRUNC|O_DSYNC, mode);
  while (fd == -1 && errno == EINTR);

  // error opening file
  if (fd == -1) return 213;
  
  // replace standard output with our tmpfile
  while (dup2(fd, 1) == -1)
    if (errno != EINTR) return 213;
  while (dup2(fd, 2) == -1)
    if (errno != EINTR) return 213;

  // User can break the program by setting searched_dir to something
  // like "-name". Preventing this is beyond the scope of this library
  execlp("find", "find", searched_dir, "-name", searched_file, (char*) NULL);

  // not reached if execlp succeeded
  return 213;
}

// In case of some error in parent process returns -1.
// In case of error in child before execlp returns -2.
// 0 otherwise
int sysopyfind_invoke(const char *tmpfile_name)
{
  if (!searched_dir || !strlen(searched_dir))
    errx(-1,"No valid directory set.");

  if (!searched_file || !strlen(searched_file))
    errx(-1,"No valid file set.");

  if (!tmpfile_name || !strlen(tmpfile_name))
    errx(-1,"Invalid tmpfile name provided.");

  pid_t pid = fork();
  if (pid == -1) return -1;
  if (pid == 0) exit(sysopyfind_child(tmpfile_name));

  // parent process code
  int wstatus;

  while (waitpid(pid, &wstatus, 0) == -1
         && errno == EINTR);

  // in case of success - save tmpfile_name
  // otherwise - try to remove broken tmpfile we might have created
  // all child's exit codes but 213 are considered success
  if (WEXITSTATUS(wstatus) == 213)
    {
      remove(tmpfile_name);
      return -2;
    }
  
  last_tmpfile = tmpfile_name;
  
  return 0;
}

// Possible errors: errors from open(), read(),
// lseek(), close() and calloc()
// also sets *filesize appropriately
ssize_t sysopyfind_load_file(const char *file_name, ssize_t *filesize)
{
  // Is set to 1 if anything failed, means that return
  // value of the function is -1
  // If no cleanups are done return -1 may be used instead
  _Bool failed = 0;

  // For saving errno
  int errsv;
  // Assume one is not interested in errno = EINTR set by some
  // functions. Restore previous errno value in such cases
  
  if (!block_array)
    errx(-1,"Attempt to load a file with no block array created.");

  if (block_count == block_array_length)
    errx(-1,"Attempt to load a file to a full array.");
  
  int fd;
  errsv = errno;
  while ((fd = open(file_name, O_RDONLY)) == -1
         && errno == EINTR);

  if (fd == -1) return -1;
  errno = errsv;

  // set filesize and then seek back to the beginning
  if ((*filesize = lseek(fd, 0, SEEK_END)) == -1
      || lseek(fd, 0, SEEK_SET) == -1)
    {
      failed = 1;
      goto close_fildes;
    }

  size_t i = block_array_length;
  while (i--)
    if (!block_array[i]) goto found_empty_slot;

  // if i reached 0 it means no empty slot was found
  errx(-1,"Invalid block array state detected.");

 found_empty_slot:
  if (!(block_array[i] = calloc(*filesize, 1)))
    {
      failed = 1;
      goto close_fildes;
    }

  size_t bytes_read = 0;
  ssize_t reading_result;

  while (bytes_read < *filesize)
    {      
      reading_result = read(fd, block_array[i] + bytes_read,
                            *filesize - bytes_read);
      
      if ((reading_result == -1 && errno != EINTR)
          || !reading_result)
        goto delete_block;
      
      bytes_read += reading_result;
    }

  // reached if no errors occured
  block_count++;
  goto close_fildes;
  
  // cleanup labels
 delete_block:
  free(block_array[i]);
  block_array[i] = NULL;
  
 close_fildes:
  errsv = errno;
  while (close(fd) && errno == EINTR);
  
  if (errno == EINTR) errno = errsv;
  // Give up if close() failed with something other than EINTR
  
  return failed ? -1 : i;
}

ssize_t sysopyfind_load_last_tmpfile(ssize_t *filesize)
{
  if (!last_tmpfile)
    errx(-1,"Attempt to load last tmpfile while none exists.");
  
  return sysopyfind_load_file(last_tmpfile, filesize);
}

void sysopyfind_delete_block(size_t index)
{
  if (!block_array)
    errx(-1,"Attempt to delete block from nonexistent array.");
  
  if (index >= block_array_length)
    errx(-1,"Block index outside block array range.");
  
  // Allow 'deletion' of nonexistent block
  if (block_array[index])
    {
      free(block_array[index]);
      block_array[index] = NULL;
      block_count--;
    }
}
