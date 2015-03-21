#!/bin/bash

# count=100
# if [ $count -eq 100 ]
# then
  # echo "Count is 100"
# fi

i=0
j=0
t_start=0
t_end=0
t_start_ref=0
t_end_ref=0
while read line; do
	if [ $i -eq 3 ]; then
		for word in $line; do
			if [ $j -eq 1 ]; then
				#echo "word1 = '$word' "
				t_start=$(echo $word | cut -d',' -f1) 
				echo "t_start = '$t_start' "
			fi
			#echo $j;
			((j++))
		done
		((j=0))
	fi
	
	if [ $i -eq 6 ]; then
		for word in $line; do
			if [ $j -eq 1 ]; then
				#echo "word = '$word' "
				t_end=$(echo $word | cut -d',' -f1) 
				echo "t_end = '$t_end' "
			fi
			((j++))	
		done
		((j=0))
	fi 
	
	if [ $i -eq 3 ]; then
		for word in $line; do
			if [ $j -eq 1 ]; then
				#echo "word = '$word' "
				t_start_ref=$(echo $word | cut -d',' -f1) 
				echo "t_start_ref = '$t_start_ref' "
			fi
			#echo $j;
			((j++))
		done
		((j=0))
	fi
	
	
	if [ $i -eq 10 ]; then
		for word in $line; do
			if [ $j -eq 1 ]; then
				#echo "word = '$word' "
				t_end_ref=$(echo $word | cut -d',' -f1) 
				echo "t_end_ref = '$t_end_ref' "
			fi
			((j++))
		done
		((j=0))
	fi 
	
	((i++));
	
	echo "$i $t_start"
	echo "$i $t_end"
	echo "$i $t_end_ref"
	echo "$i $t_start_ref"
done <log.log

echo "$t_start"
echo "$t_end"
echo "$t_end_ref"
echo "$t_start_ref"

		
echo "Execution time = $((t_end - t_start))"
echo "Reference time = $((t_end_ref - t_start_ref))"