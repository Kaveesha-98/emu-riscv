#! /bin/sh

for file in ../riscv-tests-images/*.bin; do
  #riscv64-unknown-elf-as "$file" -o objectfiles/`basename "$file" ".s"`.o
  #echo "running test `basename $file .out`"
  echo $file
  ./emu $file
done