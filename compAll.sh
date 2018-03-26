#!/bin/bash
echo "Compiling..."

gcc -pthread -lrt -o csmc csmc.c
