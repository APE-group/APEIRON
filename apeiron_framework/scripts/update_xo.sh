#!/bin/bash

# SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
# SPDX-License-Identifier: EUPL-1.2

if [ $# -ne 2 ]
then
	echo "$0: invalid arguments"
	exit 1
fi

XO=$1
SHELL=$2


if [ ! -d "ip_repo/${SHELL}" ]
then
	echo "Nothing to do for platform ${SHELL}"
	exit 0
fi


for IP in ip_repo/${SHELL}/*.xci
do
	IP_NAME=$(basename ${IP})
	IP_PATH="$(unzip -Z1 ${XO} | grep -m1 ${IP_NAME})"
	[ -z "${IP_PATH}" ] && continue

	echo "Replacing ${XO}:${IP_PATH} with ${IP}"
	zip -d ${XO} ${IP_PATH}
	python3 -c "import zipfile as zf, sys; zf.ZipFile(sys.argv[1], 'a').write(sys.argv[2], sys.argv[3])" ${XO} ${IP} ${IP_PATH}
done
