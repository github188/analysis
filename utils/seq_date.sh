#! /bin/bash


if [[ $# -lt 2 ]] ; then
    echo "usage:seq_date date1 date2"
    exit 1
fi


start_sec=$(date -d "${1}" "+%s")
if [[ 0 -ne $? ]] ; then
    echo "invalid arguments"
    exit 1
fi
end_sec=$(date -d "${2}" "+%s")
if [[ 0 -ne $? ]] ; then
    echo "invalid arguments"
    exit 1
fi


# get to work
day_sec=86400
while [[ ${start_sec} -le ${end_sec} ]] ; do
    date -d "@${start_sec}" "+%Y%m%d"
    let start_sec=${start_sec}+86400
done
