#!/bin/bash

rm ./include/*

for header in ./include/stx/*.hpp
do
	if [ ! -d "${header}" ]
	then
		name=`basename ${header%.*}`
		printf '#include "%s"' "stx/${name}.hpp" > "./include/x${name}"
	fi
done
