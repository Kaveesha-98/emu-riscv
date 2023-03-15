#! /bin/sh

for file in ../../cpu-test/target_binaries/*.out; do
  #riscv64-unknown-elf-as "$file" -o objectfiles/`basename "$file" ".s"`.o
  echo "running test `basename $file .out`"
  ./emu $file
done