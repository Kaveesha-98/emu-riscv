#! /bin/sh

for file in ../../cpu-test/target_texts/*.text; do
  #riscv64-unknown-elf-as "$file" -o objectfiles/`basename "$file" ".s"`.o
  #echo "running test `basename $file .out`"
  echo $file
  ./emu $file
done