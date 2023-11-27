#!/bin/bash

Program="./a.out"
transactionSize=(4 8 16)

for ((i=1 ; i <= 8; i++ ));
do
	for value in "${transactionSize[@]}"
	do
		for ((k=1 ; k <= 1 ; k ++ ))
		do
			echo $Program LockFree $value 1000000 10000 $i false
			$Program LockFree $value 1000000 10000 $i false
			echo $Program Locking  $value 1000000 10000 $i false
			$Program Locking  $value 1000000 10000 $i false
		done
	done
done
