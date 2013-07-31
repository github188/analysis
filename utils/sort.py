#! /usr/bin/env python
# -*- coding:utf-8 -*-


import sys


class sort(object) :
    @classmethod
    def __bubble_sort(cls, data = [], start = 0, end = -1) :
        if end < 0 :
            end += len(data)
        if end > len(data) :
            end = len(data)

        if end <= start :
            return None

        for i in range(start, end) :
            for j in range(i + 1, end + 1) :
                if data[i] > data[j] :
                    data[i], data[j] = data[j], data[i]
        return None

    @classmethod
    def __quick_sort(cls, data = [], start = 0, end = -1) :
        def __mid__(data, a, b, c) :
            if data[a] > data[b] :
                if data[a] < data[c] :
                    return a
                elif data[b] > data[c] :
                    return b
                else :
                    return c
            else :
                if data[b] < data[c] :
                    return b
                elif data[a] > data[c] :
                    return a
                else :
                    return c

        if (end - start + 1) < 3 :
            return cls.__bubble_sort(data, start, end)

        if end < 0 :
            end += len(data)
        if end > len(data) :
            end = len(data)

        if end <= start :
            return None

        pivot = __mid__(data, start, start + (end - start) / 2, end)
        data[pivot], data[end] = data[end], data[pivot]

        pivot = end
        i = start
        j = end - 1
        while True :
            if data[i] > data[pivot] :
                data[i], data[j] = data[j], data[i]
            else :
                i += 1
            if i > j :
                break;
            if data[j] < data[pivot] :
                data[j], data[i] = data[i], data[j]
            else :
                j -= 1
            if i > j :
                break;

        data[i], data[pivot] = data[pivot], data[i]
        pivot = i

        cls.__quick_sort(data, start, pivot - 1)
        cls.__quick_sort(data, pivot + 1, end)

    @classmethod
    def do_sort(cls, data, start = 0, end = -1) :
        cls.__quick_sort(data, start, end)


def main() :
    if "__main__" == __name__ :
        orig = (5, 2, 56, 33, 1, 9, 4, 12, 10, 7)
        data = list(orig)

        for i in range(0, len(data)) :
            sort.do_sort(data, 0, i)
            print data

        print orig
        sys.exit(0)

main()
