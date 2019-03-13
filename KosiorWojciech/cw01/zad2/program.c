#include <string.h>
#include <err.h>
#include <time.h>
#include <sys/times.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


#ifndef DYNAMIC_LOAD

#include "sysopyfind.h"

#else

#include <dlfcn.h>

char ***block_array;

size_t *block_array_length, *block_count;

const char **searched_dir;
const char **searched_file;

const char **last_tmpfile;


char** (*sysopyfind_create_array)(size_t);

void (*sysopyfind_free_array)(void);

const char* (*sysopyfind_set_dir)(const char*);

const char* (*sysopyfind_set_file)(const char*);

int (*sysopyfind_invoke)(const char*);

ssize_t (*sysopyfind_load_file)(const char*, ssize_t*);

ssize_t (*sysopyfind_load_last_tmpfile)(ssize_t*);

void (*sysopyfind_delete_block)(size_t);

void load_libsysopyfind(void)
{
  // we do not load any more shared objects, so it doesn't
  // matter whether we use RTLD_LOCAL or RTLD_GLOBAL
  void *dl_handle = dlopen("libsysopyfind.so", RTLD_NOW);

#define LOAD(symname) !(symname = dlsym(dl_handle, #symname))
  
  if (!dl_handle
      || LOAD(block_array)
      || LOAD(block_array_length)
      || LOAD(block_count)
      || LOAD(searched_dir)
      || LOAD(searched_file)
      || LOAD(last_tmpfile)
      || LOAD(sysopyfind_create_array)
      || LOAD(sysopyfind_free_array)
      || LOAD(sysopyfind_set_dir)
      || LOAD(sysopyfind_set_file)
      || LOAD(sysopyfind_invoke)
      || LOAD(sysopyfind_load_file)
      || LOAD(sysopyfind_load_last_tmpfile)
      || LOAD(sysopyfind_delete_block))
    errx(-1, dlerror());
}

// these will allow the code using them to remain the same
#define block_array (*block_array)

#define block_array_length (*block_array_length)
#define block_count (*block_count)

#define searched_dir (*searched_dir)
#define searched_file (*searched_file)

#define last_tmpfile (*last_tmpfile)

#endif // DYNAMIC_LOAD

int dup2_wrap(int fd1, int fd2)
{
  int errsv = errno, dup2_res;
  while ((dup2_res = dup2(fd1, fd2)) == -1
         && errno == EINTR);

  if (dup2_res != -1) errno = errsv;

  return dup2_res;
}

struct tms start;
clock_t realtime_clocks;

void start_time(void)
{
  realtime_clocks = times(&start);
}

int outputpipe[2];  
pid_t child;

void terminate_child_connection(void)
{
  while (close(outputpipe[1]) == -1)
    if (errno != EINTR)
      return;

  while (close(1) == -1)
    if (errno != EINTR)
      return;

  while (close(2) == -1)
    if (errno != EINTR)
      return;

  // once the outputs are closed child should read EOF and terminate
  // hence waitpid can be called without danger of deadlock
  int wstatus;
  while (waitpid(child, &wstatus, 0) == -1
         && errno == EINTR);
}

int main(int argc, char **argv)
{
#ifdef DYNAMIC_LOAD
  load_libsysopyfind();
#endif

  // We want the output to go both to file and console. We can
  // do this without modifying code throughout the program by
  // creating another process, that gets our output as input
  // and does all the work
  if (pipe(outputpipe)) err(-1, NULL);

  int fork_result = fork();
  if (fork_result == -1)
    err(-1, NULL);

  if (!fork_result)
    {
      // child
      // substitute stdin
      if (dup2_wrap(outputpipe[0], 0) == -1)
        err(-1, NULL);

      while (close(outputpipe[1]) == -1)
        if (errno != EINTR)
          err(-1, NULL);
      
      // create process that splits output
      system("if [ -e logfile ]; then rm logfile; fi;"
             "while read line;"
             "do echo $line; echo $line >> logfile;"
             "done");
      
      exit(0);
    }
  
  // parent
  child = fork_result;

  while (close(outputpipe[0]) == -1)
    if (errno != EINTR)
      err(-1, NULL);

  if (atexit(terminate_child_connection))
    errx(-1, "Couldn't register atexit function.");
  
  printf("\r");
  // substitute stdout and stderr
  if (dup2_wrap(outputpipe[1], 2) == -1
      || dup2_wrap(outputpipe[1], 1) == -1)
    err(-1, NULL);

  int argidx, next_argidx = 1;

  long ticks_per_second = sysconf(_SC_CLK_TCK);
  
  while ((argidx = next_argidx) < argc)
    {
#define args_check_supplied(number)                                     \
      if (argc <= argidx + (number))                                    \
        {                                                               \
          warnx("Missing arguments to last command.");                  \
          break;                                                        \
        }                                                               \
      next_argidx = argidx + 1 + (number);                              \
      printf("command:   ");                                            \
      for (int i = argidx; i < next_argidx - 1; i++)                    \
        printf("%s ", argv[i]);                                         \
      printf("%s\n", argv[next_argidx - 1])                             \
      
#define invalid(predicate, message)             \
      if (predicate)                            \
        {                                       \
          warnx(message);                       \
          continue;                             \
        }

#define assert_warn(predicate)                                  \
      if (predicate)                                            \
        {                                                       \
          warn(NULL);                                           \
          continue;                                             \
        }
      
      if (!strcmp(argv[argidx], "create_array"))
        {
          args_check_supplied(1);
          
          invalid(block_array, "Array already exists.");
          
          ssize_t array_size = atoll(argv[++argidx]);
          invalid(array_size < 1, "Invalid array size.");

          start_time();
          assert_warn(!sysopyfind_create_array(array_size));
        }
      else if (!strcmp(argv[argidx], "search_directory"))
        {
          args_check_supplied(3);

          invalid(!strlen(argv[argidx+1]), "Empty search pathname.");
          invalid(!strlen(argv[argidx+2]), "Empty filename to search.");
          invalid(!strlen(argv[argidx+3]), "Empty tmpfile name.");

          start_time();
          sysopyfind_set_dir(argv[++argidx]);
          sysopyfind_set_file(argv[++argidx]);
          int invoke_result = sysopyfind_invoke(argv[++argidx]);
          assert_warn(invoke_result == -1);
          invalid(invoke_result == -2, "Couldn't execute find.");
        }
      else if (!strcmp(argv[argidx], "load_tmpfile"))
        {
          args_check_supplied(1);

          invalid(!strlen(argv[argidx+1]), "Empty tmpfile name.");
          invalid(!block_array, "No block array to load tmpfile to.");
          invalid(block_count >= block_array_length, "No free slots in array.");

          start_time();
          ssize_t filesize;
          assert_warn(sysopyfind_load_file(argv[++argidx], &filesize) < 0);
        }
      else if (!strcmp(argv[argidx], "search_and_load"))
        {
          args_check_supplied(3);

          invalid(!strlen(argv[argidx+1]), "Empty search pathname.");
          invalid(!strlen(argv[argidx+2]), "Empty filename to search.");
          invalid(!strlen(argv[argidx+3]), "Empty tmpfile name.");

          start_time();
          sysopyfind_set_dir(argv[++argidx]);
          sysopyfind_set_file(argv[++argidx]);
          
          ssize_t filesize;
          int invoke_result = sysopyfind_invoke(argv[++argidx]);
          assert_warn(invoke_result == -1);
          invalid(invoke_result == -2, "Couldn't execute find.");
          assert_warn(sysopyfind_load_last_tmpfile(&filesize) < 0);
        }
      else if (!strcmp(argv[argidx], "remove_block"))
        {
          args_check_supplied(1);
          
          invalid(!block_array, "No array to remove from.");
          
          ssize_t block_index = atoll(argv[++argidx]);
          invalid(block_index < 0, "Invalid block index.");
          invalid(block_index >= block_array_length,
                  "Index outside of bounds.");

          start_time();
          sysopyfind_delete_block(block_index);
        }
      else if (!strcmp(argv[argidx], "free_array"))
        {
          args_check_supplied(0);

          start_time();
          sysopyfind_free_array();
        }
      else
        {
          // No args needed but thic macro call also sets next_argidx
          next_argidx = argidx + 1;

          warnx("Unrecognized command: %s", argv[argidx]);
          continue;
        }

      double realtime, kerneltime, usertime, child_kerneltime, child_usertime;
      struct tms end;

      realtime = (times(&end) - realtime_clocks) / (double) ticks_per_second;
      kerneltime = (end.tms_stime - start.tms_stime) / (double) ticks_per_second;
      usertime = (end.tms_utime - start.tms_utime) / (double) ticks_per_second;
      child_kerneltime = (end.tms_cstime - start.tms_cstime) / (double) ticks_per_second;
      child_usertime = (end.tms_cutime - start.tms_cutime) / (double) ticks_per_second;
      
      printf("real:%g     kernel:%g     user:%g     child_kernel:%g     child_user:%g\n",
             realtime, kerneltime, usertime, child_kerneltime, child_usertime);
    }
  
  return 0;
}
