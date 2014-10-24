#!/bin/bash
# this file will be used to compile your recolor.c file.
echo "Compiling recolor.c"
# Make sure the command below is the same as the command you use to compile recolor.c from the terminal or geany, 
# if not, modify the command to match the one you use.
# BUT KEEP THE FILE NAMES
gcc recolor.c -o recolor.x -lm
