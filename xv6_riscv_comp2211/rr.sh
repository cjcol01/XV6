#!/bin/bash

if [[ "$1" == "gdb" ]]; then
  make clean
  make
  make qemu-gdb

  qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel-m 3G -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 -S -gdb tcp::26000
else
  make clean
  make
  make qemu
fi
