#! /bin/bash

set -e

origin=$(dirname "$(readlink -f "$0")")
files=$(find "$(readlink -f ${origin}/../src)" -type f -regex "^.*\.\(hpp\|cpp\|h\|c\)$")
files+=$'\n'$(find "$(readlink -f ${origin}/../tests)" -type f -regex "^.*\.\(hpp\|cpp\|h\|c\)$")

echo "${files}" | parallel clang-format-21 -i
