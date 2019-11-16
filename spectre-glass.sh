#!/bin/sh

if lsmod | grep enable_arm_pmu; then
	rmmod enable_arm_pmu
fi

insmod /data/spectre/enable_arm_pmu.ko && /data/spectre/spectre-glass.out
