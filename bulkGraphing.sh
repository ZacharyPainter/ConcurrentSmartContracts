#!/bin/bash

Program="./a.out"

for ((i=400 ; i <= 10000; i+=100 ));
do
	for ((k=1 ; k <= 1 ; k ++ ))
	do
		echo $Program LockFree 4 1000 10000 4 true
		$Program LockFree 4 1000 10000 4 true
		echo $Program Locking  4 1000000 10000 4 true
		$Program Locking  4 1000000 10000 4 true
	done
done
