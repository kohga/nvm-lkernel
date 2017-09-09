#!/bin/sh

TARGET_DIR=linux-3.12.49
NVM_DIR=/root/nvmlog
DATE_DIR=/root/nvmlog/`date '+%Y%m%d'`
LOG_TIME=`date '+%Y%m%d_%H:%M:%S'`
SYSTEM=/boot/System.map-3.12.49
CONFIG=/boot/config-3.12.49
INITRD=/boot/initrd.img-3.12.49
VMLINUZ=/boot/vmlinuz-3.12.49

makedir () {
	if [ -e ${1} ]; then
		echo "${1} found."
	else
		echo "${1} NOT found."
		echo "mkdir ${1}"
		mkdir ${1}
	fi
}

mvfile () {
	if [ -e ${1} ]; then
		echo "mv ${1} ${DATE_DIR}/${LOG_TIME}_${2}"
		mv ${1} ${DATE_DIR}/${LOG_TIME}_${2}
	fi
}


start_time=`date +%s`

if [ "${USER}" = "root" ]; then
	makedir ${NVM_DIR}
	makedir ${DATE_DIR}

	mvfile $SYSTEM "System.map-3.12.49"
	mvfile $CONFIG "config-3.12.49"
	mvfile $INITRD "initrd-3.12.49"
	mvfile $VMLINUZ "vmlinuz-3.12.49"

	cd ${TARGET_DIR}
	echo "===== make -j5 ====="
	make -j5
	echo "===== make modules ====="
	make modules
	echo "===== make modules_install ====="
	make modules_install
	echo "===== make install ====="
	make install
	cd ..

	echo "===== setting boot (grub2) ====="
	echo "./set_grub.sh nvm"
	./set_grub.sh nvm

else
	echo 'ERROR: not root.'

fi

end_time=`date +%s`
time=$((end_time - start_time))
echo "(do.sh) total time: ${time}"
