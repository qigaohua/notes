#! /bin/bash


# EXT=".txt"
# MYCOPYDIR="copydir"
# COPYDIR=


# function process_file() {
#     # filename=$(basename  $1 ".c")
#     file=$1
#     extension="${file##*.}"
#     filename="${file%.*}"

#     if [ x"$extension" == x"c" ] || [ x"$extension" == x"h" ];then
#         echo "run cmd: cat $file > $COPYDIR/$file$EXT"
#         cat $file > $COPYDIR/$file$EXT
#     fi
# }


# function read_dir() {
#     for file in `ls $1`
#     do
#         if [ -d "./$1/$file" ]; then
#             echo "file >>> $file"
#             if [ "$file" != "." ] && [ "$file" != ".." ] && [ "$file" != "$MYCOPYDIR" ]; then
#                 echo "run cmd: mkdir -p $COPYDIR/$1/$file"
#                 mkdir -p "$COPYDIR/$1/$file"
#                 read_dir "$1/$file"
#             fi
#         else
#             process_file "$1/$file"
#         fi
#     done
# }


function txt_to_c() {
    for file in `ls $1`
    do
        if [ -d "$1/$file" ]; then
            txt_to_c "$1/$file"
        else
            file1="$1/$file"
            extension="${file1##*.}"
            filename="${file1%.*}"

            if [ x"$extension" == x"txt" ];then
                echo "run cmd: mv $file1 > $filename"
                mv "$file1" "$filename"
            fi
        fi
    done

}


# path=`pwd`
# COPYDIR="$path/$MYCOPYDIR"
# mkdir -p $COPYDIR

# read_dir .
txt_to_c .





