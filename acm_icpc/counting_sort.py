#! /usr/bin/env python
# -*- coding:utf-8 -*-


class CountingSort(object):
    def __getMinMax(self, datalist):
        nmin = datalist[0]
        nmax = datalist[0]
        for x in datalist:
            if x > nmax:
                nmax = x
            if x < nmin:
                nmin = x
        return (nmin, nmax)

    def sort(self, datalist):
        return self.__countingSort(datalist, *self.__getMinMax(datalist))

    def __countingSort(self, datalist, nmin, nmax):
        span_sz = nmax - nmin + 1
        span_list = []
        rslt_list = list(datalist)
        for i in xrange(span_sz):
            span_list.append(0)
        for x in datalist:
            span_list[x - nmin] += 1
        for i in xrange(1, span_sz):
            span_list[i] += span_list[i - 1]
        for x in datalist[: : -1]:
            rslt_list[span_list[x - nmin] - 1] = x
            span_list[x - nmin] -= 1
        return tuple(rslt_list)


if "__main__" == __name__:
    cs = CountingSort()
    datalist = (9, 2,7,3,4, 0,5)
    print cs.sort(datalist)
    datalist = (2,5,3,0,2,3,0,3)
    print cs.sort(datalist)
