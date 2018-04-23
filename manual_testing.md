# Manual Test Plan

To begin, first compile all executables with `make`.

# Piano tests

+ Launch program with `./piano`
+ Launch and run the command 'ax' to confirm that it plays an A note and exits
+ Launch and run the command '~2c>1e>2gx' to confirm that it plays a major C chord and exits
+ Launch and run the command '{a,3}x' to confirm that is plays 3 A notes and exits
+ Launch and run the command '[a]x' to confirm that is plays an A note in a loop and does not exit
+ Launch and run the command '[a]', wait 1 second and enter '~0x' to hear an A note that stops after the second command and exits
+ Run `./piano < hbd.in` and confirm that happy birthday is played

# Pianux tests

+ In a seperate terminal run `while [ true ]; do ps aux | grep $USER | grep pia | wc -l; sleep 1; clear; done` and record the initial number. During testing this number should rise and fall. 
+ Start pianux with `./pianux mount`
+ Run the command `cd mount; ls; cd ..` and ensure that the file piano is seen.
+ Run the command `echo "a" | cat > mount/piano`
+ Ensure that no output is printed and an A note is heard
+ Unload the filesystem with `make unload`
+ Run the command `cd mount; ls; cd ..` and ensure that no output is seen.
+ Run the command `make rundebug` and then `echo "a" | cat > mount/piano`
+ Ensure that logging information is printed and an A note is heard
+ In a separate terminal navigate to this directory and run the command `echo "abc" | cat > mount/piano`
+ In the original terminal run `echo "a" | cat > mount/piano`
+ Ensure that some interference between the sound is heard.
+ Run the command `make unload` and ensure that the log prints out all pids of spawned processes as they are cleaned up.
+ Ensure that the terminal running the while loop from step 1 has the same number it had initially.

