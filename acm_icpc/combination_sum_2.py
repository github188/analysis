#! /usr/bin/env python
# -*- coding:utf-8 -*-


'''
Given a collection of candidate numbers (C) and a target number (T),
find all unique combinations in C where the candidate numbers sums to T.

Each number in C may only be used once in the combination.

Note:
All numbers (including target) will be positive integers.
Elements in a combination (a1, a2, … , ak) must be in
non-descending order. (ie, a1 ≤ a2 ≤ … ≤ ak).
The solution set must not contain duplicate combinations.
For example, given candidate set 10,1,2,7,6,1,5 and target 8,
A solution set is:
[1, 7]
[1, 2, 5]
[2, 6]
[1, 1, 6]
'''


class Solution:
    def searchOnce(self, candidates, target):
        for i in xrange(len(candidates)):
            if i > 0 and candidates[i] == candidates[i-1]:
                continue

            if candidates[i] > target:
                continue
            elif candidates[i] < target:
                self.tmp.append(candidates[i])
                self.searchOnce(candidates[i+1:], target - candidates[i])
                self.tmp.pop()
            else:
                lst = self.tmp[:]
                lst.append(candidates[i])
                lst.sort()
                self.rslt.append(lst)


    # @param candidates, a list of integers
    # @param target, integer
    # @return a list of lists of integers
    def combinationSum2(self, candidates, target):
        self.rslt = []
        self.tmp = []
        sorted_cd = sorted(candidates, reverse = True)
        self.searchOnce(sorted_cd, target)
        return self.rslt


if "__main__" == __name__:
    s = Solution()
    print s.combinationSum2([10,1,2,7,6,1,5], 8)
    print s.combinationSum2([4,1,1,4,4,4,4,2,3,5], 10)
