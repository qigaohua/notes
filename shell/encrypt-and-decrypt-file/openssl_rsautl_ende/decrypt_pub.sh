#!/bin/sh


# 这个是私钥解密脚本，对应的是公钥加密的脚本


# 需要解密的文件
ENCRYPT_PKG=$1

# 私钥文件
USER_KEY=$2


FILENAME=`echo $ENCRYPT_PKG | sed 's/^.*\///'`

PWD=`pwd`/

ENCRYPT_FILE_AES=${PWD}${FILENAME}".aes"
URANDOM_STRING_FILE=${PWD}"urandom.key"
URANDOM_STRING_FILE_ENCRYPT=${PWD}"urandom.rsa"

echo ${ENCRYPT_FILE_AES} ${URANDOM_STRING_FILE} ${URANDOM_STRING_FILE_ENCRYPT}

# 解密后的文件默认是 .tar.gz 类型压缩文件
DECRYPT_FILE=${PWD}`echo ${FILENAME} | sed 's/_encrypt.*$/\.tar.gz/'`

if [ -f $ENCRYPT_PKG ]; then
    echo "start encrypt file: $ENCRYPT_PKG ..."
else
    echo "$ENCRYPT_PKG is not exists ! exit."
    exit 1
fi


dd if=${ENCRYPT_PKG} of=$URANDOM_STRING_FILE_ENCRYPT bs=256 count=1
dd if=${ENCRYPT_PKG} of=${ENCRYPT_FILE_AES} bs=256 skip=1

# openssl enc -d -des3 -a -salt -in ${URANDOM_STRING_FILE_ENCRYPT} -kfile ${USER_KEY} -out ${URANDOM_STRING_FILE}
# 用私钥解密
openssl rsautl -decrypt -in ${URANDOM_STRING_FILE_ENCRYPT} -inkey ${USER_KEY} -out ${URANDOM_STRING_FILE}

openssl enc -d -aes-256-cbc -in ${ENCRYPT_FILE_AES} -kfile ${URANDOM_STRING_FILE} -out ${DECRYPT_FILE}

rm -f ${ENCRYPT_FILE_AES} ${URANDOM_STRING_FILE} ${URANDOM_STRING_FILE_ENCRYPT}

echo output: ${DECRYPT_FILE}
echo Done.

exit 0


