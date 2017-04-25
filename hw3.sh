#!/bin/sh
# this is for automating the assignment
# tasks becausse i'm I don't want
# to type them all repeatedly...

ls
lsmod | grep acme
ls /dev/acme
make
sudo insmod acme.ko
lsmod | grep acme
ls /dev/acme
sudo cat /dev/acme
gcc -o acme_rwr acme_rwr.c
sudo ./acme_rwr
sudo ./acme_rwr 42
sudo ./acme_rwr
sudo cat /dev/acme
sudo rmmod acme
dmesg | tail

lsmod | grep acme
ls /dev/acme
sudo insmod acme.ko syscall_val=41
lsmod | grep acme
ls /dev/acme
sudo cat /dev/acme
sudo ./acme_rwr
sudo ./acme_rwr 43
sudo ./acme_rwr
sudo cat /dev/acme
sudo cat /proc/devices | grep acme
sudo rmmod acme
dmesg | tail

