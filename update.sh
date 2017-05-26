#!/bin/bash

scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

rm "${scriptdir}/include/"* 2> /dev/null

for header in "${scriptdir}/include/stx/"*.hpp
do
	if [ ! -d "${header}" ]
	then
		name=`basename ${header%.*}`
		printf '#include "%s"' "stx/${name}.hpp" > "${scriptdir}/include/x${name}"
	fi
done
