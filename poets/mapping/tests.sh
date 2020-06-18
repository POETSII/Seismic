#!/bin/bash
# ./tests.sh |& tee -a ~/out.txt
for f in $(ls ~/inputs/vol/j*r1.txt)
do
  echo "Next: $f"
  ./run $f
  sleep 5s
done
