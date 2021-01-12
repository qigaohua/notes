#! /bin/sh


# 这个是用公钥加密脚本，对应的是私钥解密


# 需要加密的文件
ENCRYPT_FILE=$1

# 公钥文件
USER_KEY=$2

PWD=`pwd`/

ENCRYPT_FILE_AES=${PWD}${ENCRYPT_FILE}".aes"
URANDOM_STRING_FILE=${PWD}"urandom.key"
URANDOM_STRING_FILE_ENCRYPT=${PWD}"urandom.rsa"


# 加密后生成的文件, 默认pkg结尾
# ENCRYPT_FILE_PKG=${ENCRYPT_FILE}".pkg"
ENCRYPT_FILE_PKG=${PWD}`echo $ENCRYPT_FILE | sed 's/^.*\///' | sed 's/\..*$/_encrypt\.pkg/'`

if [ -f $ENCRYPT_FILE ]; then
    echo "start encrypt file: $ENCRYPT_FILE ..."
else
    echo "$ENCRYPT_FILE is not exists ! exit."
    exit 1
fi

# 随机产生一个key值，并保存到urandom.key
echo "Randomly generate a key value and save it to urandom.key ..."
dd if=/dev/urandom bs=32 count=1 2>/dev/null | hexdump -v -e '/1 "%02X"' > ${URANDOM_STRING_FILE}

echo "openssl -e urandom.key >> urandom.des3 ..."
# 用公钥加密
openssl rsautl -encrypt -in ${URANDOM_STRING_FILE}  -inkey ${USER_KEY} -pubin  -out ${URANDOM_STRING_FILE_ENCRYPT}

# 加密需要加密的文件，kfile为上面产生的urandom.key
openssl enc -e -aes-256-cbc -in ${ENCRYPT_FILE} -kfile ${URANDOM_STRING_FILE} -out ${ENCRYPT_FILE_AES}

# 将 opensslurandom.key而产生的urandom.des3 和
# openssl 加密文件而产生的ENCRYPT_FILE_AES文件
# 一起放入的一个文件中
# 这里urandom.des3的字节长度为256，解密时要记得
cat ${URANDOM_STRING_FILE_ENCRYPT} ${ENCRYPT_FILE_AES} > ${ENCRYPT_FILE_PKG}

echo "output: ${ENCRYPT_FILE_PKG}"

# 删除不需要的文件
rm -f ${ENCRYPT_FILE_AES} ${URANDOM_STRING_FILE} ${URANDOM_STRING_FILE_ENCRYPT}

echo Done.

exit 0


