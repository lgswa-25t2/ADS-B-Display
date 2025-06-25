#!/bin/sh

for file in *.cpp *.c *.h *.hpp; do
  if [ -f "$file" ]; then
    output_file="lint/${file}.txt"
    cpplint "$file" > "$output_file" 2>&1
  fi
done
