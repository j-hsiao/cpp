#!/bin/bash

EXPECTDIR=expecteds;
RESULTDIR=results;

DUMMYKEY="000102030405060708090a0b0c0d0e0f00102030405060708090a0b0c0d0e0f0";

for BITS in 128 192 256
do
	keylen=$[ BITS / 8 ]
	key="${DUMMYKEY:0:$[keylen * 2 ]}"
	for MODE in ecb cbc
	do
		if [ "${MODE}" = "cbc" ]
		then
			flags="-iv 00000000000000000000000000000000"
		else
			flags=""
		fi

		mkdir -p "${EXPECTDIR}" "${RESULTDIR}"
		echo hello world > testin;
		openssl enc -aes-${BITS}-${MODE} \
			-K "${key}" \
			-in testin \
			${flags} \
			-out "${EXPECTDIR}/out_${BITS}_${MODE}"

		./aes --mode "${MODE}" --bits "${BITS}" -x "${key}" testin "${RESULTDIR}/out_${BITS}_${MODE}" -y

		numlines=$( diff "${EXPECTDIR}/out_${BITS}_${MODE}" "${RESULTDIR}/out_${BITS}_${MODE}" | wc -l )
		if [ "${numlines}" -gt 0 ]
		then
			echo "${BITS} ${MODE}: failed"
		else
			echo "${BITS} ${MODE}: pass"
		fi
	done
done
