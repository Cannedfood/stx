#!/bin/bash

scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

rm -r "${scriptdir}/include/x"* 2> /dev/null

for header in "${scriptdir}/include/stx/"*
do
	if [ ! -d "${header}" ]
	then
		name=`basename ${header%.*}`
		printf '#include "%s"' "stx/${name}.hpp" > "${scriptdir}/include/x${name}"
	fi
done

mkdir "${scriptdir}/include/xwip/"
for header in "${scriptdir}/include/stx/wip/"*.hpp
do
	if [ ! -d "${header}" ]
	then
		name=`basename ${header%.*}`
		printf '#include "%s"' "../stx/wip/${name}.hpp" > "${scriptdir}/include/xwip/${name}"
	fi
done
