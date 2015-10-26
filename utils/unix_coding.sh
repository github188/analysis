#! /bin/bash


function convert_file()
{
    if [[ -z ${1} ]] ; then
        echo "missing arguments"
        return;
    fi

    filename=${1}
    iconv -f gbk -t utf8 ${filename} > ${filename}.tmp
    mv ${filename}.tmp ${filename}
    dos2unix ${filename}
}


function convert_dir()
{
    if [[ -z ${1} ]] ; then
        echo "missing arguments"
        return;
    fi

    dirname=$1
    cd ${dirname}
    for i in $(ls) ; do
        if [[ -d ${i} ]] ; then
            convert_dir ${i}
        else
            convert_file ${i}
        fi
    done
    cd -
}


if [[ ${#} -lt 1 ]] ; then
    echo "usage: ${0} directory"
    exit 1
fi

basedir=${1}
convert_dir ${basedir}
exit 0
