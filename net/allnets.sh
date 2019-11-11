#!/bin/bash
for g in 32 16 8 4
do
	for r in 3 5 7
	do
		./net -i ~/inputs/in3.bin -o ~/inputs/L${g}r${r}.txt -g ${g} -r ${r}
	done
done
