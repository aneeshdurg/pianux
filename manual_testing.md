# Manual Test Plan

+ Launch program with `./piano`
+ Launch and run the command 'ax' to confirm that it plays an A note and exits
+ Launch and run the command '~2c>1e>2gx' to confirm that it plays a major C chord and exits
+ Launch and run the command '{a,3}x' to confirm that is plays 3 A notes and exits
+ Launch and run the command '[a]x' to confirm that is plays an A note in a loop and does not exit
+ Launch and run the command '[a]', wait 1 second and enter '~0x' to hear an A note that stops after the second command and exits
+ Run `./piano < hbd.in` and confirm that happy birthday is played
