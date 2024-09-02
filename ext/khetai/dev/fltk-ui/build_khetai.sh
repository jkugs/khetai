#!/bin/bash
set -e

cd "$(dirname "$0")"

make -f Makefile.khetai clean
make -f Makefile.khetai

echo "KhetAI lib rebuilt successfully"