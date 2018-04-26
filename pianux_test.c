#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define load_pianux() system("make run >/dev/null");
#define unload_pianux() ({ system("make unload >/dev/null"); sleep(1); })
#define TEST_CLEAN(fn)                                                         \
  if (!fn()) {                                                                 \
    puts(#fn " passed!");                                                      \
  } else {                                                                     \
    puts(#fn " failed.");                                                      \
  }

#define TEST(fn)                                                               \
  load_pianux();                                                               \
  TEST_CLEAN(fn);                                                              \
  unload_pianux();

#define pianux_path "mount/piano"

int piano_exists_test(){
  if (access(pianux_path, F_OK))
    return 1;
  if (access(pianux_path, W_OK))
    return 1;
  return 0;
}

int piano_size_test(){
  struct stat buf;
  if(stat(pianux_path, &buf))
    return 1;
  if(buf.st_size)
    return 1;
  return 0;
}

int piano_read_test(){
  int fd = open(pianux_path, O_RDONLY); 
  char buf[1];
  if(read(fd, buf, 1) > 0)
    return 1;
  close(fd);
  return 0;
}

int piano_write_test(){
  int fd = open(pianux_path, O_WRONLY); 
  write(fd, "ax", 2);
  close(fd);
  return 0;
}

int piano_process_test(){
  int fds[2];
  pipe(fds);
  int orig_stdout = dup(1);
  dup2(fds[1], 1);
  system("ps aux | grep pia | wc -l");
  load_pianux();
  int fd = open(pianux_path, O_WRONLY); 
  write(fd, "ax", 2);
  close(fd);
  unload_pianux();
  system("ps aux | grep pia | wc -l");
  dup2(orig_stdout, 1);
  
  FILE *output = fdopen(fds[0], "r");
  size_t len = 0;
  char *buffer = NULL;
  
  getline(&buffer, &len, output);
  int start = atoi(buffer);

  memset(buffer, 0, len);
  getline(&buffer, &len, output);
  int end = atoi(buffer);

  return end-start;
}

int piano_logging_test(){
  return 0;
}

int main(){
  TEST(piano_exists_test);
  TEST(piano_size_test);
  TEST(piano_read_test);
  TEST(piano_write_test);
  TEST_CLEAN(piano_process_test);
  TEST(piano_logging_test);
  
}
