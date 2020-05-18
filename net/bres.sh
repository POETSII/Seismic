g=4
for f in 0 1 2 3 4 5
do
	./net -i f${f}.bin -g $g -r 23 -a 1 -o f${f}_G0.txt
	for r in 2 3 5 7
	do
		./net -i f${f}.bin -g $g -r $r -a 0 -o f${f}_fwds${r}.txt
		./net -i f${f}.bin -g $g -r $r -a 1 -o f${f}_full${r}.txt
		./net -i f${f}.bin -g $g -r $r -a 2 -o f${f}_bres${r}.txt
		echo "Done f$f r=$r"
	done
done
