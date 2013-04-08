#! /bin/bash


LAYOUT=dot
IMGS_DIR=imgs

if [ ! -d ${IMGS_DIR} ]; then
    mkdir -p ${IMGS_DIR}
fi

if [ nginx.dot -nt ${IMGS_DIR}/nginx.png ]; then
    ${LAYOUT} -T png nginx.dot -o ${IMGS_DIR}/nginx.png
fi
if [ nginx1.dot -nt ${IMGS_DIR}/nginx1.png ]; then
    ${LAYOUT} -T png nginx1.dot -o ${IMGS_DIR}/nginx1.png
fi
