#!/bin/bash

# singularity pull docker://callaghanmt/xv6-tools:buildx-latest
cd XV6/xv6_riscv_comp2211/

singularity shell xv6-tools_buildx-latest.sif

rr
# make clean

# make

# make qemu