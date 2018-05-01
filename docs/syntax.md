# Pianux syntax

In this document we distinguish between two kinds of input. Playback input, and meta input. Playback input is input which generates audio or directly influences the state of operations on the audio card. Meta input is input which controls the behavior of the parent process managing audio generating children.

# Playback Input 

## a,b,c,d,e,f,g,' '

Play a note corresponding to the character input. If the character is ' ', play some silence.

## (+/-)s

Sets the speed to s. If it is preceded by an optional + or -, respectively increase or decrease the current speed.

## #(+/-)n
  
Same as speed but for octave.

## v(+/-)n

Same as speed but for volume.

## *

Discard input until newline (comments)

## s
  
Save current octave, volume and speed

## r 
  
Restore last saved octave, volume and speed. Note that saves and restores can be nested. 

Example:
```
4#3v1*  speed = 4, octave = 3, volume = 1
s
5#4v2*  speed = 5, octave = 4, volume = 2
s
6#5v3*  speed = 5, octave = 4, volume = 2
r*      speed = 5, octave = 4, volume = 2
r*      speed = 4, octave = 3, volume = 1
```

## {commands..., n}

Repeat commands n times. Before and after each iteration the currect octave, volume and speed are saved and restored respectively.

## [commands...]
  
Repeat commands infinitely.

## "x~>y

x and y must be notes. Plays an x smoothly transitioning to y.

## .

Repeat last command

## Meta Input

Channels refer to children processes that have access to the audio card.

## ~c

Reset or create channel c. If channel c is currently playing a note or is in a loop it will be stopped. Alternatively, use `~!` to reset all channels.

## >c

Redirect playback input to channel c

## !

Assert some condition about the state of the parent process (e.g. number of channels)

