if lsmod | grep enable_arm_pmu; then
	rmmod enable_arm_pmu
fi
insmod enable_arm_pmu.ko && $*
