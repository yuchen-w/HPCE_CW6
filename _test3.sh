#!/bin/bash

declare -a t_start=($(cat log.log | grep "Executing puzzle" | sed 's/.*\],//;s/,.*$//')) 	#replace everything before ], with nothing and everything after , with nothing
declare -a t_start_ref=($(cat log.log | grep "Executing reference" | sed 's/.*\],//;s/,.*$//'))
declare -a t_end_ref=($(cat log.log | grep "Checking output" | sed 's/.*\],//;s/,.*$//') )

echo ${t_start[0]}



# for i in $t_start_ref
# do
	# echo $i
# done
	