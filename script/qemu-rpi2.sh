#!/bin/sh

qemu-system-arm -M raspi2 -kernel build/kernel/tupai-kernel.elf -d guest_errors -m 256M #-serial stdio
