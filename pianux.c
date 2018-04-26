#define FUSE_USE_VERSION 29 
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

/**
 * Pid to file descriptor bookkeeping
 */
typedef struct {
  pid_t owner;
  int comm;
} piano_proc;

#define TYPE piano_proc 
#include "data_structures/list/list.h"
#undef TYPE
struct list_sentinal_piano_proc p_mapping;

/**
 * Logging to file objects
 */
FILE *log_file = NULL;
#define LOGGING(...)  if(log_file) { \
  fprintf(log_file, __VA_ARGS__);    \
  fflush(log_file);                  \
  fsync(fileno(log_file));           \
}

/**
 * Janitor thread for cleaning up dead processes
 * This prevents the creation of zombies
 */
int janitor_pipe[2];
pthread_t janitor_tid;

/**
 * Janitor function to clean up dead processes
 *
 * Usage: To inform the janitor of a new process, write the new process' pid
 * and stdin file descriptor to the janitor.
 *
 * To stop janitor send a pid of -1.
 *
 * @param unused is not used
 */
static void *janitor(void *unused){
  LOGGING("Janitor Spawned!\n");
  pid_t p;
  int fd;
  while(1){
    read(janitor_pipe[0], (char*)&p, sizeof(p));
    if( p == -1 )
      break;
    read(janitor_pipe[1], (char*)&fd, sizeof(fd));
    LOGGING("Waiting for process %d\n", p);
    waitpid(p, NULL, 0); // Clean up child process
    close(fd);

    // Delete pid from list 
    LIST_FOR_EACH_SAFE(&p_mapping, elem, temp, {
      if(elem->entry.owner == p){
        LIST_DEL(&p_mapping, elem);
        break;
      }
    });
    LOGGING("Cleaned up %d\n", p);
  }
  return NULL;
}

/**
 * Hardcoded readdir implementation since the only file is 'piano'
 *
 * For detailed description of parameters see libfuse documentation
 */
static int pianux_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *fi){
  filler(buffer, ".", NULL, 0);
  filler(buffer, "..", NULL, 0);
  if(strlen(path) == 1 && path[0] == '/'){
    filler(buffer, "piano", NULL, 0);
  }
  return 0;
}

/**
 * Unimplemented read to refuse read requests.
 *
 * For detailed description of parameters see libfuse documentation
 */
static int pianux_read( const char *path, char *buffer, size_t size, 
    off_t offset, struct fuse_file_info *fi){
  return -1;
}

/**
 * Fuse equivalent of close.
 *
 * For detailed description of parameters see libfuse documentation
 */
static int pianux_release( const char *path, struct fuse_file_info *fi) {
  if(!strcmp(path, "/piano"))
    return -ENOENT;
  LOGGING("RELEASED %ld\n", fi->fh);
  close(fi->fh);
  
  LIST_FOR_EACH(&p_mapping, elem, {
      if(elem->entry.comm == fi->fh){
  wait(NULL);

      }
  });
  return 0;  
}

/**
 * Truncate implemented as no-op
 *
 * For detailed description of parameters see libfuse documentation
 */
static int pianux_truncate( const char *path, off_t offset){
  LOGGING("TRUNC %s\n", path);
  return 0;
}

/**
 * Send commands to piano process passed in via buffer.
 * Essentially this allows the file to act like a pipe.
 *
 * For detailed description of parameters see libfuse documentation
 */
static int pianux_write( const char *path, const char *buffer, size_t size, 
    off_t offset, struct fuse_file_info *fi){
  LOGGING("WRITE %s\n", path);
  if(strcmp(path, "/piano"))
    return -ENOENT;

  LOGGING("WRITE /piano %lu %s\n", fi->fh, buffer);
  return write(fi->fh, buffer, size);  
}

/**
 * Spawn new piano process and set up pipes to communicate.
 * If logging is enabled, the spawned process's stdout and stderr will be 
 * redirected to the logfile.
 *
 * For detailed description of parameters see libfuse documentation
 */
static int pianux_open(const char *path, struct fuse_file_info *fi){
  // Set up pipe to talk to piano process 
  int fds[2];
  pipe(fds);

  pid_t child = fork();
  if(!child){
    dup2(fds[0], 0); 
    close(fds[1]);

    // Redirect program output/error to log
    if(log_file){
      dup2(fileno(log_file), 1); 
      dup2(fileno(log_file), 2); 
    }
    // TODO move path into env
    execl("/home/aneesh/piano/piano", "./piano", NULL);
    LOGGING("EXEC FAILED!\n");
    perror("");
    exit(1);
  }
  LOGGING("Starting process %d\n", child);

  // Inform janitor of child process
  write(janitor_pipe[1], (char*)&child, sizeof(child));
  write(janitor_pipe[1], (char*)&(fds[1]), sizeof(fds[1])); 
  close(fds[0]);
  
  // Register write end of pipe with current writer
  fi->fh = fds[1];
  // Register write end of pipe with list of open fds 
  piano_proc new_proc = {child, fds[1]};
  LIST_APPEND(&p_mapping, new_proc);

  LOGGING("OPENED %ld (%lu processes)\n", fi->fh, p_mapping.length);
  return 0;
}

/**
 * Hardcoded getattr implementation since the only file is 'piano'
 *
 * For detailed description of parameters see libfuse documentation
 */
static int pianux_getattr(const char *path, struct stat *stbuf) {
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
    // Permissions for directory
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 1;
	} else if (strcmp(path+1, "piano") == 0) {
    // Allows anyone to read and write, no execute
		stbuf->st_mode = S_IFREG | 0555;
		stbuf->st_nlink = 1;
    // Size of 0 to illustrate that reads are not allowed
		stbuf->st_size = 0;
	} else
		res = -ENOENT;

	return res;
}

/**
 * Performs any setup needed prior to mount
 *
 * @param unused is not used
 */
void *pianux_init(struct fuse_conn_info *unused){
  // Setup janitor 
  pipe(janitor_pipe);
  pthread_create(&janitor_tid, NULL, janitor, NULL);
  // Set up pid mapping
  p_mapping = new_list_piano_proc(NULL, 0, NULL);
  return NULL;
}

/**
 * Cleans up any remaining processes and threads before unmounting
 *
 * @param unused is not used
 */
void pianux_destroy(void *unused){
  // Clean up mapping
  LIST_FOR_EACH(&p_mapping, elem, {
    LOGGING("KILLING %d\n", elem->entry.owner);
    kill(elem->entry.owner, SIGKILL);
  });
  LIST_DESTROY(&p_mapping);
  
  // Stop janitor
  pid_t cancel = (pid_t)-1;
  write(janitor_pipe[1], (char*)&cancel, sizeof(cancel));
  pthread_join(janitor_tid, NULL);
  close(janitor_pipe[1]);
  close(janitor_pipe[0]);

  LOGGING("DONE PIANUX CLEANUP\n");
}

// fops stores mapping of functions to purpose
static struct fuse_operations fops = {
	.readdir	= pianux_readdir,
	.getattr	= pianux_getattr,
	.open	  	= pianux_open,
	.read	  	= pianux_read,
	.write		= pianux_write,
	.truncate	= pianux_truncate,
	.release  = pianux_release,
  .init     = pianux_init,
  .destroy  = pianux_destroy,
};

int main(int argc, char *argv[]){
  // Set up logging from environment
  char *fname = getenv("LOGFILE");
  if(fname){
    log_file = fopen(fname, "w");
  }

  // Register callbacks in fops and mount filesystem
  return fuse_main(argc, argv, &fops, NULL);
}
