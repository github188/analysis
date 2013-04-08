#! /bin/sh


LAYOUT=dot
IMGS_DIR=imgs

if [ ! -d imgs ]; then
    mkdir -p ${IMGS_DIR}
fi

${LAYOUT} -T png nginx.dot -o ${IMGS_DIR}/nginx.png
${LAYOUT} -T png nginx2.dot -o ${IMGS_DIR}/nginx2.png
