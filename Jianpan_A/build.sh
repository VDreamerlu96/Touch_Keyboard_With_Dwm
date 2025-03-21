#!/bin/sh

gcc main.c use_uinput.c error.c use_xcb.c keyboard.c -O2 -o stkeyboard -lxcb
#simple touch keyboard