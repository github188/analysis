#! /usr/bin/env python
# -*- coding:utf-8 -*-


class Solution:
    # @param num, a list of integer
    # @return an integer
    def longestConsecutive(self, num):
        max_length = 0
        dc = {}

        for x in num:
            dc[x] = 1
        for key in dc:
            keym1 = key - 1
            keya1 = key + 1
            tmp_length = 1

            dc[key] = 0
            while dc.has_key(keym1) and 1 == dc[keym1]:
                dc[keym1] = 0
                keym1 -= 1
                tmp_length += 1
            while dc.has_key(keya1) and 1 == dc[keya1]:
                dc[keya1] = 0
                keya1 += 1
                tmp_length += 1

            if tmp_length > max_length:
                max_length = tmp_length

        return max_length


if "__main__" == __name__:
    s = Solution()

    pending = [10, 4, 20, 1, 3, 2]
    print pending
    print s.longestConsecutive(pending)
