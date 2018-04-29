#!/bin/bash
# Usage: ./size.sh app.elf

objdump=${OBJDUMP:-arm-none-eabi-objdump}
elf_file=$1

function get_symbol {
	name=$1
	objdump_cmd="$objdump -t $elf_file"
	size=$($objdump_cmd | grep " $name" | cut -d " " -f1 | tr 'a-f' 'A-F')
	printf "ibase=16\n$size\n" | bc
}

function get_section_length {
	name=$1
	start=$(get_symbol "__${name}_start__")
	end=$(get_symbol "__${name}_end__")
	echo $(( $end - $start ))
}

function print_usage {
	symbol=$1
	length_symbol=$2
	usage=$(get_section_length $symbol)
	length=$(get_symbol $length_symbol)
	free=$(( $length - $usage ))
	echo -e "$symbol:\t$usage / $length\t($free bytes free)"
}

print_usage flash __FLASH_LENGTH__
print_usage ram __RAM_LENGTH__
