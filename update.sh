#!/bin/bash

scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

function relink() {
	if [ -z "$3" ]; then
		if [ -d "${scriptdir:?}/$2" ]; then
			rm -Rf   "${scriptdir:?}/$2"
		fi
	fi

	mkdir -p "${scriptdir}/$2"

	for header in "${scriptdir}/$1/"*.hpp
	do
		if [ ! -d "${header}" ]
		then
			name=`basename ${header%.*}`
			outfile="${scriptdir}/$2/$3${name}"
			printf '#include "%s"' "$(realpath --relative-to="$(dirname "${outfile}")" "${header}")" > "${outfile}"
		fi
	done
}

relink "include/stx/" "include/" "x"
relink "include/stx/wip" "include/xwip"
