#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>

/**
 * Prompt user for sudo access if not already granted
 */
#define get_sudo_access() system("sudo echo");

/**
 * Wrappers around system to mount and unmount pianux
 */
#define load_pianux() ({ sleep(1); system("make run >/dev/null"); })
#define load_pianux_bg() system("make run >/dev/null &");
#define unload_pianux() ({ sleep(1); system("make unload >/dev/null"); })

/**
 * Macros to test functions of type int(*)(void)
 */
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

// Default mount path
#define pianux_path "mount/piano"

/**
 * Test existance of piano file
 */
int piano_exists_test(){
  if (access(pianux_path, F_OK))
    return 1;
  if (access(pianux_path, W_OK))
    return 1;
  return 0;
}

/**
 * Test 0 size of piano file
 */
int piano_size_test(){
  struct stat buf;
  if(stat(pianux_path, &buf))
    return 1;
  if(buf.st_size)
    return 1;
  return 0;
}

/**
 * Test unsupported read
 */
int piano_read_test(){
  int fd = open(pianux_path, O_RDONLY); 
  char buf[1];
  if(read(fd, buf, 1) > 0)
    return 1;
  close(fd);
  return 0;
}

/**
 * Test working write (doesn't test audio output)
 */
int piano_write_test(){
  int fd = open(pianux_path, O_WRONLY); 
  write(fd, "ax", 2);
  close(fd);
  return 0;
}

/**
 * Test that processes are correctly cleaned up
 */
int piano_process_test(){
  int fds[2];
  pipe(fds);
  // Save original standard out
  int orig_stdout = dup(1);

  // Redirect stdout to pipe
  dup2(fds[1], 1);
  system("ps aux | grep pia | wc -l");
  load_pianux();
  int fd = open(pianux_path, O_WRONLY); 
  write(fd, "ax", 2);
  close(fd);
  unload_pianux();
  system("ps aux | grep pia | wc -l");
  // Restore stdout - pipe has contents of above commmands
  dup2(orig_stdout, 1);
  close(fds[1]);

  // Open read end of pipe for examining output of ps
  FILE *output = fdopen(fds[0], "r");
  size_t len = 0;
  char *buffer = NULL;
  
  getline(&buffer, &len, output);
  int start = atoi(buffer); // Initial number of processes

  memset(buffer, 0, len);
  getline(&buffer, &len, output);
  int end = atoi(buffer); // Final number of processes

  fclose(output);
  return end-start; // Test passes when end == start
}

// Necessary code for 'try/catch' block
sigjmp_buf _env;
void alarm_handle(int sig){
  (void)sig;
  siglongjmp(_env, 1);
}

int piano_logging_test(){
  int retval = 0;
  mkfifo("newfile", 0664);
  // Turn on logging 
  setenv("LOGFILE", "newfile", 1);

  // Need to mount in background so we can simultaneously open in read mode
  // as a result, fopen will block until the mount is done
  load_pianux_bg();
  FILE *f = fopen("newfile", "r");

  /**
   * 'try/catch' block
   * Attempts to read, but if the read takes longer than 1s, sigalrm will be 
   * generated and siglongjmp will jump to sigsetjmp and return 1.
   */
  signal(SIGALRM, alarm_handle);
  int retry = sigsetjmp(_env, 1);
  if(!retry){
    alarm(1); // Set alarm, read has 1s to complete
    char c;
    retval = !(fread(&c, 1, 1, f) == 1);
    alarm(0); // Clear pending alarms
  } else {
    retval = 1; // Test failed, got here because siglongjmp jumped to sigsetjmp
  }

  fclose(f);
  unlink("newfile");
  unload_pianux();
  return retval;
}

int main(){
  printf("Tests require sudo access, please enter authenticate to continue\n");
  get_sudo_access();

  if(!access(pianux_path, F_OK)){
    printf("Detected mounted pianux!\nUnmount and proceed with tests (y/n)? ");
    char choice = tolower(getc(stdin));
    if(choice != 'y')
      return 0;
    unload_pianux();
  }

  TEST(piano_exists_test);
  TEST(piano_size_test);
  TEST(piano_read_test);
  TEST(piano_write_test);
  TEST_CLEAN(piano_process_test);
  TEST_CLEAN(piano_logging_test);
  
  return 0;
}
