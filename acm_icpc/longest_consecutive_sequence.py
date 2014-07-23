#! /usr/bin/env python
# -*- coding:utf-8 -*-


class Solution:
    def getMinMaxOfList(self, num):
        min = num[0]
        max = num[0]

        for x in num:
            if x < min:
                min = x
            if x > max:
                max = x

        return (min, max)

    # @param num, a list of integer
    # @return an integer
    def longestConsecutive(self, num):
        tmp_length = 0
        max_length = 0

        (min, max) = self.getMinMaxOfList(num)
        tmp_list = [0 for x in xrange(max - min + 1)]
        for x in num:
            tmp_list[x - min] = 1
        for n in tmp_list:
            if 0 == n:
                if tmp_length > max_length:
                    max_length = tmp_length
                tmp_length = 0
            else:
                tmp_length += 1

        return max_length

if "__main__" == __name__:
    s = Solution()

    pending = [10, 4, 20, 1, 3, 2]
    print pending
    print s.longestConsecutive(pending)
