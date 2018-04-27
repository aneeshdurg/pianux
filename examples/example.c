#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int audio_fd;
time_t start;
int words_completed = 0;
int characters_completed = 0;
int characters_typed = 0;
int errors_made = 0;
// total time
void sigint_handler(int sig) {
  (void)sig;
  time_t elapsed = time(NULL) - start;
  double wpm = (double)words_completed / ((double)elapsed / 60);
  // Restore stty attributes before exit
  system("stty sane");
  close(audio_fd);

  printf("Time elapsed: %lds\n", elapsed);
  printf("Words completed: %d\n", words_completed);
  printf("Characters typed: %d\n", characters_typed);
  printf("Errors: %d\n", errors_made);
  printf("\n");
  printf("Words per minute: %0.2f\n", wpm);
  exit(0);
}

char *dictionary_path = "/usr/share/dict/american-english";

void swap_stdout(int *swap_buffer) {
  swap_buffer[0] = dup(1);
  int pipes[2];
  pipe(pipes);
  swap_buffer[1] = pipes[0];
  swap_buffer[2] = pipes[1];

  dup2(pipes[1], 1);
}

char *restore_stdout(int *swap_buffer, size_t len) {
  int *pipes = swap_buffer + 1;

  dup2(swap_buffer[0], 1);
  close(pipes[1]);
  char *buffer = calloc(1, len);
  read(pipes[0], buffer, len);
  if (buffer[strlen(buffer) - 1] == '\n')
    buffer[strlen(buffer) - 1] = 0;
  buffer = realloc(buffer, strlen(buffer) + 1);
  close(pipes[0]);
  return buffer;
}
char *randomword() {
  char *command = NULL;
  asprintf(&command, "shuf %s | head -1", dictionary_path);
  int swap_stdouts[3];

  swap_stdout(swap_stdouts);
  system(command);
  char *output = restore_stdout(swap_stdouts, 1024);
  free(command);

  return output;
}

#define erase() printf("\b \b")
#define up() printf("\033[A")
#define to_start(counter) printf("\033[%luD", counter)
#define clear_line() printf("\033[K")

#define green() printf("\033[1;32m")
#define red() printf("\033[0;31m")
#define color_normal() printf("\033[0m")

void display_word(char *word, size_t counter, int tries) {
  green();
  for (size_t i = 0; i < counter; i++)
    printf("%c", *(word + i));

  color_normal();
  if (tries)
    red();
  // char at counter
  printf("%c", *(word + counter));
  color_normal();
  // rest of string
  // Needs carrige return to move cursor to start
  printf("%s\n\r", word + counter + 1);
  for (size_t i = 0; i < counter; i++)
    printf(" ");
}

#define audio_failed(fd)                                                       \
  { write(fd, "6a", 2); }

#define audio_success(fd)                                                      \
  { write(fd, "6cg", 3); }

int main(int argc, char *argv[]) {

  if (argc > 1) {
    fprintf(stderr, "Pianux typing game demo. To use, run ./example\n");
    fprintf(stderr, "To play, simply enter the text on screen.\n");
    fprintf(stderr, "Pressing CTRL+C will quit and display statistics\n");
    fprintf(stderr, "To change the dictionary of words, set DICTIONARY_PATH\
 to be whatever file you want in you environment.\n");
    exit(0);
  }
  audio_fd = open(getenv("PIANUX_PATH"), O_WRONLY);
  if (audio_fd < 0) {
    fprintf(stderr, "Error: could not open piano file, is pianux mounted?\n");
    exit(1);
  }

  if (getenv("DICTIONARY_PATH"))
    dictionary_path = getenv("DICTIONARY_PATH");

  // Get input character by character
  system("stty raw");
  start = time(NULL);
  // Cleanup on signal
  signal(SIGINT, sigint_handler);

  // Game loop
  while (1) {
    char *word = randomword();
    size_t len = strlen(word);
    size_t counter = 0;
    int tries = 0;
    while (counter < len) {
      display_word(word, counter, tries);
      char c = getc(stdin);
      characters_typed++;
      erase();

      if (c == EOF || c == 3) {
        // Cause signal
        raise(SIGINT);
      } else if (c == word[counter]) {
        tries = 0;
        counter++;
        characters_completed++;
      } else {
        audio_failed(audio_fd);
        tries++;
        errors_made++;
      }

      up();
      to_start(counter);
      clear_line();
      // raise(SIGINT);
    }
    audio_success(audio_fd);
    words_completed++;
  }

  // Unreachable
  return 0;
}
