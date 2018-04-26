#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define load_pianux() system("make run >/dev/null");
#define unload_pianux() system("make unload >/dev/null");
#define TEST(fn)                                                               \
  load_pianux();                                                               \
  if (!fn()) {                                                                 \
    puts(#fn " passed!");                                                      \
  } else {                                                                     \
    puts(#fn " failed.");                                                      \
  }                                                                            \
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
  fprintf(
  return 0;
}

int piano_logging_test(){
  return 0;
}

int main(){
  TEST(piano_exists_test);
  TEST(piano_size_test);
  TEST(piano_read_test);
  TEST(piano_write_test);
  TEST(piano_process_test);
  TEST(piano_logging_test);
  
}
