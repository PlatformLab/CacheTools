#!/bin/bash

# Usage: ./TakeCores.sh <MyCores> <OtherCores> <my_pid>

if [[ "$#" -lt 3 ]]; then
    echo "Usage: ./TakeCores.sh <MyCores> <OtherCores> <my_pid>"
    exit
fi

cd /sys/fs/cgroup/cpuset
mkdir -p AllOthers
echo "$2" > AllOthers/cpuset.cpus

# This line may only be nec. on RC cluster machines.
cat cpuset.mems > AllOthers/cpuset.mems

# Move all other processes away
for i in $(cat cgroup.procs ); do
    echo $i > AllOthers/cgroup.procs 2> /dev/null;
done

# Create a private group for my process
mkdir -p CurrentProcess
echo "$1" > CurrentProcess/cpuset.cpus
cat cpuset.mems > CurrentProcess/cpuset.mems

# Put my process into it.
echo "$3" > CurrentProcess/cgroup.procs
