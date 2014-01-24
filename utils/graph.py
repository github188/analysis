#! /usr/bin/env python
# -*- coding:utf-8 -*-


import sys
import copy


class graph:
    ''' vertex '''
    class vertex:
        ''' edge '''
        class edge:
            def __init__(self, weight):
                self.__weight = weight;

            def get_weight(self):
                return self.__weight

        def __init__(self):
            self.__edges = {}

        def add_edge(self, to, weight):
            if not self.exist_edge(to):
                self.__edges[to] =  graph.vertex.edge(weight)

        def get_edges(self):
            return self.__edges

        def exist_edge(self, to):
            return to in self.__edges.keys()

        def del_edge(self, to):
            if self.exist_edge(to):
                del self.__edges[to]

        def printme(self):
            for (k, v) in self.__edges.items():
                sys.stdout.write("%s(%d)|" % (k, v.get_weight()))
            sys.stdout.write("\n")

    ''' graph '''
    def __init__(self):
        self.__vertexes = {}
        self.__vertexes_indegree = {}

    def __add_vertex(self, name):
        self.__vertexes[name] = graph.vertex()
        self.__vertexes_indegree[name] = 0

    def add_vertex(self, name):
        if name not in self.__vertexes.keys():
            self.__add_vertex(name)

    def del_vertex(self, name):
        if name not in self.__vertexes.keys():
            return -1

        ''' 删除所有到该顶点的边 '''
        for (k, v) in self.__vertexes.items():
            v.del_edge(name)

        ''' 降低所有后继顶点的入度 '''
        for (k, v) in self.__vertexes[name].get_edges().items():
            self.__vertexes_indegree[k] -= 1

        ''' 删除顶点极其出边 '''
        del self.__vertexes[name]
        del self.__vertexes_indegree[name]

        return 0

    def add_edge(self, frm, to, weight = 0):
        if frm not in self.__vertexes.keys():
            self.__add_vertex(frm)
        if to not in self.__vertexes.keys():
            self.__add_vertex(to)

        if not self.__vertexes[frm].exist_edge(to):
            self.__vertexes[frm].add_edge(to, weight)
            self.__vertexes_indegree[to] += 1

    def del_edge(self, frm, to):
        if frm not in self.__vertexes.keys():
            return -1

        if to not in self.__vertexes.keys():
            return -1

        if not self.__vertexes[frm].exist_edge(to):
            return -1

        self.__vertexes[frm].del_edge(to)
        self.__vertexes_indegree[to] -= 1

        return 0

    ''' 拓扑排序 '''
    def topsort(self):
        rslt = []
        g = copy.deepcopy(self)

        while g.__vertexes_indegree:
            size_before = len(g.__vertexes_indegree)
            for k in g.__vertexes_indegree.keys():
                if 0 == g.__vertexes_indegree[k]:
                    rslt.append(k)
                    g.del_vertex(k)
            size_after = len(g.__vertexes_indegree)

            if size_before == size_after:
                return []

        return rslt

    def printme(self):
        print self.__vertexes
        print self.__vertexes_indegree
        for (k, v) in self.__vertexes.items():
            sys.stdout.write("%s(%d): " % (k, self.__vertexes_indegree[k]))
            v.printme()


if "__main__" == __name__:
    g = graph()
    '''
    g.add_edge("v1", "v2")
    g.add_edge("v1", "v3")
    g.add_edge("v1", "v4")
    g.add_edge("v2", "v4")
    g.add_edge("v2", "v5")
    g.add_edge("v3", "v6")
    g.add_edge("v4", "v3")
    g.add_edge("v4", "v6")
    g.add_edge("v4", "v7")
    g.add_edge("v5", "v4")
    g.add_edge("v5", "v7")
    g.add_edge("v7", "v6")
    '''
    g.add_edge("v1", "v2")
    g.add_edge("v2", "v3")
    g.add_edge("v3", "v1")
    print g.topsort()
    g.printme()
