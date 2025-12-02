#!/usr/bin/env bash

name="$1"
directory="${2:-.}"

if [ -z "$name" ]; then
    printf "Usage: %s <name> [directory]\n" "$0" >&2
    exit 1
fi

last=$(
    printf "%s\n" "$directory"/* 2>/dev/null \
    | sed -n 's|^.*/\([0-9]\{6\}\)_[a-zA-Z_.-]*$|\1|p' \
    | sort -n \
    | tail -n1
)
if [ -z "$last" ]; then
    next=1
else
    next=$((last + 1))
fi
prefix=$(printf "%06d" "$next")

file="$directory/${prefix}_${name}"
touch "$file"
echo "$file"
