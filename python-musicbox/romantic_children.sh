#!/bin/sh

loop=1
trap 'loop=0' INT TERM

python -m musicbox.main ../score/romantic_children.txt
while [ $loop -eq 1 ]; do
  python -m musicbox.main ../score/romantic_children_loop.txt
done
