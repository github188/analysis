#! /usr/bin/env python
# -*- coding:utf-8 -*-


def random_select(dataset, n):
    tmpset = list(dataset)

    datalen = len(tmpset)
    if n < 0:
        n = 1
    if n > datalen:
        n = datalen

    if 1 == datalen:
        return tmpset[n - 1]
    if 2 == datalen:
        if tmpset[0] > tmpset[1]:
            tmpset[0], tmpset[1] = tmpset[1], tmpset[0]
        return tmpset[n - 1]

    leftset = []
    rightset = []

    ''' 寻找枢纽元，并把枢纽元放到最后 '''
    mid = len(tmpset)
    pivot = None
    if tmpset[0] < tmpset[-1]:
        if tmpset[mid] < tmpset[0]:
            pivot = 0
            tmpset[0], tmpset[-1] = tmpset[-1], tmpset[0]
        elif tmpset[mid] > tmpset[-1]:
            pivot = -1
        else:
            pivot = mid
            tmpset[mid], tmpset[-1] = tmpset[-1], tmpset[mid]
    else:
        if tmpset[mid] < tmpset[-1]:
            pivot = -1
        elif tmpset[mid] > tmpset[0]:
            privot = 0
            tmpset[0], tmpset[-1] = tmpset[-1], tmpset[0]
        else:
            pivot = mid
            tmpset[mid], tmpset[-1] = tmpset[-1], tmpset[mid]

    ''' 分治 '''
    for i in xrange(datalen - 1):
        if tmpset[i] < tmpset[-1]:
            leftset.append(tmpset[i])
        else:
            rightset.append(tmpset[i])

    ''' 递归 '''


if "__main__" == __name__:
    x = 3
    y = 5
    x,y=y,x
    print x,y
