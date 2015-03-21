#!/bin/bash
testvar="1 2 3 4 8 16 32 64 128 1024 2048 8192 16000"
#testvar="1 2 3 4 8 16"
echo "Running median_bits"
for i in $testvar; do
	./bin/run_puzzle median_bits $i 2 2>> median_bits.log
done
	
declare -a t_start=($(cat median_bits.log | grep "Executing puzzle" | sed 's/.*\],//;s/,.*$//')) 	#replace everything before ], with nothing and everything after , with nothing
declare -a t_start_ref=($(cat median_bits.log | grep "Executing reference" | sed 's/.*\],//;s/,.*$//'))
declare -a t_end_ref=($(cat median_bits.log | grep "Checking output" | sed 's/.*\],//;s/,.*$//') )
echo $(cat median_bits.log | grep "Output is not correct")
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
##############
echo "Running circuit_sim"
for i in $testvar; do
	./bin/run_puzzle circuit_sim $i 2 2>> circuit_sim.log
done
	
declare -a t_start=($(cat circuit_sim.log | grep "Executing puzzle" | sed 's/.*\],//;s/,.*$//')) 	#replace everything before ], with nothing and everything after , with nothing
declare -a t_start_ref=($(cat circuit_sim.log | grep "Executing reference" | sed 's/.*\],//;s/,.*$//'))
declare -a t_end_ref=($(cat circuit_sim.log | grep "Checking output" | sed 's/.*\],//;s/,.*$//') )
echo $(cat circuit_sim.log | grep "Output is not correct")
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
###############
testvarlife="1 2 3 4 8 16 32 64 128"
echo "Running life"
for i in $testvarlife; do
	./bin/run_puzzle life $i 2 2>> life.log
done
	
declare -a t_start=($(cat life.log | grep "Executing puzzle" | sed 's/.*\],//;s/,.*$//')) 	#replace everything before ], with nothing and everything after , with nothing
declare -a t_start_ref=($(cat life.log | grep "Executing reference" | sed 's/.*\],//;s/,.*$//'))
declare -a t_end_ref=($(cat life.log | grep "Checking output" | sed 's/.*\],//;s/,.*$//') )
echo $(cat life.log | grep "Output is not correct")
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

###############
echo "Running matrix_exponent"
for i in $testvar; do
	./bin/run_puzzle matrix_exponent $i 2 2>> matrix_exponent.log
done
	
declare -a t_start=($(cat matrix_exponent.log | grep "Executing puzzle" | sed 's/.*\],//;s/,.*$//')) 	#replace everything before ], with nothing and everything after , with nothing
declare -a t_start_ref=($(cat matrix_exponent.log | grep "Executing reference" | sed 's/.*\],//;s/,.*$//'))
declare -a t_end_ref=($(cat matrix_exponent.log | grep "Checking output" | sed 's/.*\],//;s/,.*$//') )
echo $(cat matrix_exponent.log | grep "Output is not correct")
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


###############
echo "Running option_explicit"
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


###############
echo "Running string_search"
for i in $testvar; do
	./bin/run_puzzle string_search $i 2 2>> string_search.log
done
	
declare -a t_start=($(cat string_search.log | grep "Executing puzzle" | sed 's/.*\],//;s/,.*$//')) 	#replace everything before ], with nothing and everything after , with nothing
declare -a t_start_ref=($(cat string_search.log | grep "Executing reference" | sed 's/.*\],//;s/,.*$//'))
declare -a t_end_ref=($(cat string_search.log | grep "Checking output" | sed 's/.*\],//;s/,.*$//') )
echo $(cat string_search.log | grep "Output is not correct")

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