#!/bin/bash
out=~/out_traverse.txt
stats=~/out_traverse_stats.txt
#res=~/out_res

#rm -f $out
#rm -f $stats
#mkdir $res

echo_all () {
	echo $1
	echo $1 >> $out
	echo $1 >> $stats
}

for m in "SF" "AF" "SW" "AW"
do
	echo_all "====== ${m} ======"
	cd traverse_${m}/build

	for f in $(ls ~/inputs/tr/5/lim*.txt)
	do
		echo_all "Next: $f"
		./run $f >> $out
		../../util/sumstats.awk stats.txt >> $stats
		#fn=$(basename $f)
		#cp result.txt ${res}/${m}_${fn}
		sleep 5s
	done

	cd ../..
done
