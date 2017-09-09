#!/bin/sh

OPERATION=$1
NVM_OPERATION=nvm
DEFAULT_OPERATION=default
NVM_DIR=/root/nvmlog
DATE_DIR=/root/nvmlog/`date '+%Y%m%d'`
LOG_TIME=`date '+%Y%m%d_%H:%M:%S'`
GRUB_NVM=grub/grub_nvm
GRUB_DEFAULT=grub/grub_def
GRUB=/etc/default/grub

usage () {
	echo "usage: ${0} operation ( ${NVM_OPERATION} or ${DEFAULT_OPERATION} )"
	echo "Please input as follows."
	echo "./set_grub.sh ${NVM_OPERATION}\n\tor\n./set_grub.sh ${DEFAULT_OPERATION}"
	exit 1
}

makedir () {
	if [ -e ${1} ]; then
		echo "${1} found."
	else
		echo "${1} NOT found."
		echo "mkdir ${1}"
		mkdir ${1}
	fi
}

cpfile () {
	if [ -e ${1} ]; then
		echo "cp ${1} ${DATE_DIR}/${LOG_TIME}_${2}"
		cp ${1} ${DATE_DIR}/${LOG_TIME}_${2}
	fi
}


if [ -z ${OPERATION} ]
then
	usage;
fi


if [ "${USER}" = "root" ]; then
	makedir ${NVM_DIR}
	makedir ${DATE_DIR}
	cpfile ${GRUB} "GRUB"

	if [ ${OPERATION} = ${NVM_OPERATION} ]; then
		echo "cp ${GRUB_NVM} ${GRUB}"
		cp ${GRUB_NVM} ${GRUB}
		echo "grub-mkconfig --output=/boot/grub/grub.cfg"
		grub-mkconfig --output=/boot/grub/grub.cfg

	elif [ ${OPERATION} = ${DEFAULT_OPERATION} ]; then
		echo "cp ${GRUB_DEFAULT} ${GRUB}"
		cp ${GRUB_DEFAULT} ${GRUB}
		echo "grub-mkconfig --output=/boot/grub/grub.cfg"
		grub-mkconfig --output=/boot/grub/grub.cfg

	else
		echo "${1} is no operation."
		usage
	fi

else
	echo "ERROR: not root."
fi
