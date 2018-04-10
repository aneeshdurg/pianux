#include <stdio.h>
#include <string.h>
#include <ao/ao.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <assert.h>
#include "algebraic-c/algebraic.h"
#include "parser/parser.h"

// Note to frequence mapping
int freqs[] = {220.0, 246.942, 261.626, 293.665, 329.628, 349.228, 391.995,  0};
// This macro emulates a dictionary like interface
#define GET_FREQ(c) ((c==' ')?0:freqs[c-'a']);

// Global vars (unique to each instance)
ao_device *device;
ao_sample_format format;
int restart = 0;

/**
 * Setup libao sound card
 */
void setup_sound(){
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
 * Close and cleanup sound card
 */
void shutdown_sound(){
	ao_close(device);
	ao_shutdown();
}

/** 
 * Sets the restart flag to 1
 * @param sig unused signal passed in from signal handler
 */
void handler(int sig){
  (void)sig;
  restart = 1;
}

/**
 * Clears pipe by reading until delimiter
 *
 * @param f file to be cleared
 * @param delim delimeter to clear until
 */
static void dump_buffer(FILE *f, char delim){
  char c;
  do {
    c = getc(f);
  } while (c != delim);
}

typedef float (*interp_fn)(float,float,int,int);

int piano_proc(){
	char *buffer = NULL;
	int buf_size;
	int sample;
	float freq = 440.0;
	float start_freq = 440.0;
	float end_freq = 440.0;
	int i;
 
  setup_sound();

  signal(SIGUSR1, handler);
	fprintf(stderr, "libao child program\n");
	
  int speed = 1;
  int volume = 0;
  float octave = 1.0;
  while(1){
    // Get ready for next input 
    free(buffer);
    // From the libao example:
    buf_size = format.bits/8 * format.channels * format.rate/speed;

    // Buffer for storing audio data
	  buffer = calloc(buf_size,
			sizeof(char));
    interp_fn interpolator = NULL;

    // Clear pipe and reset audio input
    if(restart){
      restart = 0;
      fprintf(stderr, "doing restart!\n");
      shutdown_sound();
      setup_sound();
      dump_buffer(stdin, '\0');
    }

    printf("Waiting... ");
    CmdT c = getinput();
    printCmdT(c);
    puts("");

    // Callbacks for input
    $(End, c, break);
    $(Speed, c, {
        if(c.Speed.relative)
          speed += c.Speed.relative * c.Speed.s;
        else
          speed = c.Speed.s;
        continue;
      });
    $(Note, c, {
        freq = GET_FREQ(c.Note.c);
      });

    // Adjust octave of note
    freq *= octave;

    // Fill buffer for a given quantum of time
	  for (i = 0; i < format.rate/speed; i++) { 
      // Adujust frequency/amplitude
      float amp = 32768.0+100*volume;
      float final_freq = freq;

      // Generate sound as sin wave
	  	sample = (int)(amp * 
	  	  	sin(2 * M_PI * final_freq * ((float) i/format.rate))
          );

	    // Duplicate left channel input into right channel input	
	  	buffer[4*i] = buffer[4*i+2] = sample & 0xff;
	  	buffer[4*i+1] = buffer[4*i+3] = (sample >> 8) & 0xff;
	  }
	  ao_play(device, buffer, buf_size);
  }
  shutdown_sound();
  return (0);
}

/**
 * Creates a new child process to run piano_proc
 *
 * @param proc_com pipe to replace child's stdin
 */
pid_t setup(int proc_com[2]){
  pid_t p = fork();
  if(p < 0){
    fprintf(stderr, "Unable to spawn p\n");
  } else if(!p){
    close(0);
    dup2(proc_com[0], 0); 
    close(proc_com[1]);
    piano_proc();
    exit(0);
  }
  return p;
}

/**
 * Entry point to the piano
 */
int main(int argc, char **argv){
  // Setup default number of channels
  int channels = 2;
  if(argc > 1)
    channels = atoi(argv[1]);
  int proc_com[10][2]; 
  pid_t *pids = calloc(10, sizeof(int));
  for(int i = 0; i<channels; i++){
    pipe(proc_com[i]);
    pids[i] = setup(proc_com[i]);
  }
  
  int channel = 0;
  while(1){
    char c = getc(stdin);
    if(c == '!'){
      // Assertions 
      char *line = NULL;
      size_t n = 0;
      getline(&line, &n, stdin);
      switch(line[0]){
        case 'c':
          assert(channels >= line[1] - '0');
      }
      continue;
    }
    if(c == 'x' || c == EOF){
      // Command to exit 
      break;
    }
    if( c == '~' ) {
      // Command to create new channels or reset existing ones
      c = getc(stdin);
      if(c < '0' || c > '9'){
        write(proc_com[channel][1], "~", 1);
        write(proc_com[channel][1], &c, 1);
        continue;
      }
      int chan = c - '0';
      if(pids[chan]){
        fprintf(stderr, "Resetting channel %d\n", chan);
        kill(pids[chan], SIGUSR1);
        // Write newline to dump 
        write(proc_com[chan][1], "\0", 1); 
      } else{
        pipe(proc_com[chan]);
        pids[chan] = setup(proc_com[chan]);
      }
    } else if(c == '>'){
      // Command to switch channels
      fprintf(stderr, "Got switch cmd\n");
      c = getc(stdin);
      if(c < '0' || c > '9'){
        write(proc_com[channel][1], ">", 1);
        write(proc_com[channel][1], &c, 1);
        continue;
      }
      channel = c - '0';
      fprintf(stderr, "Switching to channel %c\n", c);
    } else {
      write(proc_com[channel][1], &c, 1);
    }
  }

  // Clean up and exit
  fprintf(stderr, "Parent got x\n");
  for(int i = 0; i<channels; i++){
    close(proc_com[i][0]);
    write(proc_com[i][1], "x", 1);
    wait(NULL);
  }
}
