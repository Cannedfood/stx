#!/bin/bash

# Licensed under the CC0 1.0 License: https://creativecommons.org/publicdomain/zero/1.0/

alias errecho='>&2 echo'

errecho "Unimplemented"
exit 1

# Error diagnosis:
if [ -z ${1+x} ]; then
	errecho "Usage: $(basename "$0") <C++ Header>"
	exit 1
fi

# Get compiler into variable CXX
if [ -z ${CXX+x} ]; then
	if which clang++; then
		CXX=clang++
	elif which g++; then
		CXX=g++
	fi
fi

# For each header given...
for header in "$@"; do
	makerule="`$CC -MM "$header"`"

	makerule_pfx="${header%$hpp}.o:"
	headers:=${makerule#$makerule_pfx}

	# TODO
done

exit 0
