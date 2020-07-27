#!/bin/bash
out=~/out_traverse.txt
stats=~/out_traverse_stats.txt

rm -f $out
rm -f $stats

echo_all () {
	echo $1
	echo $1 >> $out
	echo $1 >> $stats
}

for m in "SH" "SW" "AH" "AW"
do
	echo_all "====== ${m} ======"
	cd traverse_${m}/build
	
	for f in $(ls ~/inputs/tr/*.txt)
	do
		echo_all "Next: $f"
		./run $f >> $out
		../../util/sumstats.awk stats.txt >> $stats
		sleep 5s
	done
	
	cd ../..
done
