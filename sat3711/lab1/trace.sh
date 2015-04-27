#!/bin/bash
if [ $# -ne 1 ]; then
  echo "Usage:"
  echo "    trace.sh <host>"
  exit 1
fi

echo "Route to "$1
traceroute $1 | grep -e 'ms\|\*' | awk -F ' ' '{ print $1" "$2 }'

