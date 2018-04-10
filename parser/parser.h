#pragma once
#include "../algebraic-c/algebraic.h"
#include <stdio.h>

/**
 * Create new printable algebraic data type
 */
printableType2Header( Cmd,
    Note, char c,
    Speed,  int relative;
            int s,
    End,,
);

/**
 * Get next command
 */
CmdT getinput();
