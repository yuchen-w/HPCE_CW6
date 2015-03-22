#!/bin/bash
testvar="1 2 3 4 8 16 32 64 128 256 512 1024 2048"
rm option_explicit.log
for i in $testvar; do
	./bin/run_puzzle option_explicit $i 2 2>> option_explicit.log
done
	
declare -a t_start=($(cat option_explicit.log | grep "Executing puzzle" | sed 's/.*\],//;s/,.*$//')) 	#replace everything before ], with nothing and everything after , with nothing
declare -a t_start_ref=($(cat option_explicit.log | grep "Executing reference" | sed 's/.*\],//;s/,.*$//'))
declare -a t_end_ref=($(cat option_explicit.log | grep "Checking output" | sed 's/.*\],//;s/,.*$//') )
echo $(cat option_explicit.log | grep "Output is not correct")
j=0
for i in $testvar; do
	echo "For puzzle size '$i'"
	echo "Execution time:"
	echo ${t_start_ref[j]}-${t_start[j]} | bc -l
	echo "Reference time: "  
	echo ${t_end_ref[j]}-${t_start_ref[j]} | bc -l
	((j++))
	#echo "Reference time = '${
done