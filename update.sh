#!/bin/bash

scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

function relink() {
	if [ -z "$3" ]; then
		if [ -d "${scriptdir:?}/$2" ]; then
			rm -Rf   "${scriptdir:?}/$2"
		fi
	fi


	src_dir="${scriptdir}/$1/"
	dst_dir="${scriptdir}/$2/"

	mkdir -p "${dst_dir}"

	relative_prefix="./`realpath --relative-to="${dst_dir}" "${src_dir}"`"

	for header in "${src_dir}/"*.hpp
	do
		if [ ! -d "${header}" ] # Ignore directories
		then
			name=`basename ${header%.*}`
			outfile="${dst_dir}/$3${name}"
			rm "${outfile}"
			ln -s "${relative_prefix}/${name}.hpp" "${outfile}"
		fi
	done
}

relink "include/stx/" "include/" "x"
