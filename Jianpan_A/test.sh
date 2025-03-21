#!/bin/bash

rm test
gcc main.c -o test -lxcb && ./test