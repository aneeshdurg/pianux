#include "data_structures/algebraic-c/algebraic.h"
#include "parser/parser.h"
#include <ao/ao.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Note to frequence mapping
int freqs[] = {220.0, 246.942, 261.626, 293.665, 329.628, 349.228, 391.995, 0};
// This macro emulates a dictionary like interface
#define GET_FREQ(c) ((c == ' ') ? 0 : freqs[c - 'a']);

// Encapsulation for audio settings
typedef struct {
  int speed;
  int volume;
  float octave;
} settings_t;

// Implements settings_t linked list
#define TYPE settings_t
#include "data_structures/list/list.h"
#undef TYPE

// Global vars (unique to each instance)
ao_device *device;
ao_sample_format format;
int restart = 0;
struct list_sentinal_settings_t settings_stack;
settings_t settings;

/**
 * Setup libao sound card
 */
void setup_sound() {
  ao_initialize();

  int default_driver = ao_default_driver_id();

  memset(&format, 0, sizeof(format));
  format.bits = 16;
  format.channels = 2;
  format.rate = 44100;
  format.byte_format = AO_FMT_LITTLE;

  device = ao_open_live(default_driver, &format, NULL /* no options */);
  if (device == NULL) {
    fprintf(stderr, "Error opening device.\n");
    exit(1);
  }
}

/**
 * Default sound settings
 */
settings_t default_settings() {
  settings_t settings;
  settings.speed = 1;
  settings.volume = 0;
  settings.octave = 1.0;
  return settings;
}

/**
 * Close and cleanup sound card
 */
void shutdown_sound() {
  ao_close(device);
  ao_shutdown();
}

/**
 * Perform all steps to shutdown piano
 */
static void shutdown_piano() {
  shutdown_sound();
  reset_parser();
  LIST_DESTROY(&settings_stack);
}

/**
 * Perform all setups to setup piano
 */
static void setup_piano() {
  setup_sound();
  reset_parser();
  settings = default_settings();
}

/**
 * Sets the restart flag to 1
 * @param sig unused signal passed in from signal handler
 */
void reset_handler(int sig) {
  (void)sig;
  restart = 1;
}

/**
 * Catch SIGINT, cleanup and exit
 * @param sig unused signal passed in from signal handler
 */
void interrupt_handler(int sig){
  (void)sig;
  shutdown_piano();
  exit(0);
}

/**
 * Clears pipe by reading until delimiter
 *
 * @param f file to be cleared
 * @param delim delimeter to clear until
 */
static void dump_buffer(FILE *f, char delim) {
  char c;
  do {
    c = getc(f);
  } while (c != delim);
}

// Interpolation type
typedef float (*interp_fn)(float, float, int, int);

/**
 * Interpolator that smoothly transitions from one frequency to another
 *
 * @return float current frequency
 *
 * @param start starting frequency
 * @param end ending frequency
 * @param speed current speed
 * @param position current timestamp relative to speed
 */
float smooth(float start, float end, int speed, int position) {
  float slope = (end - start) / (format.rate / speed);
  return start + slope * position;
}

int piano_proc() {
  char *buffer = NULL;
  int buf_size;
  int sample;
  float freq = 440.0;
  float start_freq = 440.0;
  float end_freq = 440.0;
  int i;
  settings_stack = new_list(settings_t, NULL);

  signal(SIGUSR1, reset_handler);

  setup_piano();
  fprintf(stderr, "libao child program\n");

  while (1) {
    // Get ready for next input
    free(buffer);
    // From the libao example:
    buf_size = format.bits / 8 * format.channels * format.rate / settings.speed;

    // Buffer for storing audio data
    buffer = calloc(buf_size, sizeof(char));
    interp_fn interpolator = NULL;

    // Clear pipe and reset audio input
    if (restart) {
      restart = 0;
      fprintf(stderr, "doing restart!\n");
      shutdown_piano();
      setup_piano();
      dump_buffer(stdin, '\0');
    }

    CmdT c = getinput(NONE);

    // Callbacks for input
    $(End, c, break);
    $(Save, c, LIST_PREPEND(&settings_stack, settings); continue;);

    $(Restore, c, {
      if (settings_stack.length) {
        settings = LIST_POPF(&settings_stack);
      }
      continue;
    });

    $(Speed, c, {
      if (c.Speed.relative)
        settings.speed += c.Speed.relative * c.Speed.s;
      else
        settings.speed = c.Speed.s;
      continue;
    });

    $(Volume, c, {
      if (c.Volume.relative)
        settings.volume += c.Volume.relative * c.Volume.v;
      else
        settings.volume = c.Volume.v;
      continue;
    });

    $(Octave, c, {
      if (c.Octave.relative)
        settings.octave += c.Octave.relative * c.Octave.o;
      else
        settings.octave = c.Octave.o;
      continue;
    });

    $(Interp, c, {
      start_freq = GET_FREQ(c.Interp.start);
      end_freq = GET_FREQ(c.Interp.end);
      interpolator = smooth;
    });

    $(Note, c, { freq = GET_FREQ(c.Note.c); });

    // Adjust octave of note
    freq *= settings.octave;

    // Fill buffer for a given quantum of time
    for (i = 0; i < format.rate / settings.speed; i++) {
      // Adujust frequency/amplitude
      float amp = 32768.0 + 100 * settings.volume;
      float final_freq = freq;
      if (interpolator)
        final_freq = interpolator(start_freq, end_freq, settings.speed, i);

      // Generate sound as sin wave
      sample =
          (int)(amp * sin(2 * M_PI * final_freq * ((float)i / format.rate)));

      // Duplicate left channel input into right channel input
      buffer[4 * i] = buffer[4 * i + 2] = sample & 0xff;
      buffer[4 * i + 1] = buffer[4 * i + 3] = (sample >> 8) & 0xff;
    }
    ao_play(device, buffer, buf_size);
  }
  shutdown_piano();
  return 0;
}

/**
 * Creates a new child process to run piano_proc
 *
 * @param proc_com pipe to replace child's stdin
 */
pid_t setup(int proc_com[2]) {
  pid_t p = fork();
  if (p < 0) {
    fprintf(stderr, "Unable to spawn p\n");
  } else if (!p) {
    close(0);
    dup2(proc_com[0], 0);
    close(proc_com[1]);
    piano_proc();
    exit(0);
  }
  return p;
}

/**
 * Create a channel
 *
 * @return pid_t pid of created channel process
 *
 * @param chan channel to reset
 */
#define create_channel(chan) ({                                               \
  fprintf(stderr, "Creating channel %d\n", chan);                             \
  pipe(proc_com[chan]);                                                       \
  pids[chan] = setup(proc_com[chan]);                                         \
})
/**
 * Reset a channel
 *
 * @return ssize_t return 1 on success and -1 on error
 *
 * @param chan channel to reset
 */
#define reset_channel(chan) ({                                                \
  fprintf(stderr, "Resetting channel %d\n", chan);                            \
  kill(pids[chan], SIGUSR1);                                                  \
  /* Write null byte to dump buffer*/                                         \
  write(proc_com[chan][1], "\0", 1);                                          \
})

/**
 * Entry point to the piano
 */
int main(int argc, char **argv) {
  // Setup default number of channels
  int channels = 2;
  if (argc > 1)
    channels = atoi(argv[1]);

  int proc_com[10][2];
  pid_t pids[10];
  for (int i = 0; i < channels; i++)
    create_channel(i);

  int channel = 0;
  while (1) {
    char c = getc(stdin);
    if (c == 'x' || c == EOF) {
      // Command to exit
      break;
    } else if (c == '!') {
      // Assertions
      char *line = NULL;
      size_t n = 0;
      getline(&line, &n, stdin);
      switch (line[0]) {
        case 'c':
          assert(channels >= line[1] - '0');
      }
      continue;
    } else if (c == '~' || c == '>') {
      // Command to create new channels or reset existing ones
      char arg = getc(stdin);
      if (arg < '0' || arg > '9') {
        if ( c == '~' && arg == '!' ){
          for(int i = 0; i < 10; i++){
            if(pids[i])
              reset_channel(i);
          }
        }
        write(proc_com[channel][1], &c, 1);
        write(proc_com[channel][1], &arg, 1);
        continue;
      }
      int chan = arg - '0';
      if (pids[chan]) {
        switch (c) {
          case '~': 
            reset_channel(chan);
            break;
          case '>':
            fprintf(stderr, "Switching to channel %d\n", chan);
            channel = chan;
            break;
        }
      } else{
        switch(c) {
          case '~': {
            create_channel(chan);
            break;
          }
          case '>': {
            fprintf(stderr, "Can't switch to channel %d\n", chan);
          }
        }
      }
    } else {
      write(proc_com[channel][1], &c, 1);
    }
  }

  // Clean up and exit
  fprintf(stderr, "Parent got x\n");
  for (int i = 0; i < channels; i++) {
    close(proc_com[i][0]);
    write(proc_com[i][1], "x", 1);
    close(proc_com[i][1]);
    wait(NULL);
  }
}
