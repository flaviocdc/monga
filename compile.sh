#!/bin/bash

./compile $1.m
sh a.out.defs
as -o $1.o a.out
cc -o $1 $1.o runtime.o -lc

