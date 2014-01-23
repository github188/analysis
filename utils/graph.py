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
            self.__edges[to] =  graph.vertex.edge(weight)

        def exist_edge(self, to):
            return to in self.__edges.keys()

        def del_edge(self, to):
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
        pass

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

    def topsort(self):
        g = copy.deepcopy(self)
        g.printme()

    def printme(self):
        for (k, v) in self.__vertexes.items():
            sys.stdout.write("%s(%d): " % (k, self.__vertexes_indegree[k]))
            v.printme()


if "__main__" == __name__:
    g = graph()
    g.add_edge("home", "school")
    g.add_edge("home", "company")
    g.del_edge("home", "company")
    g.add_edge("home", "company")
    g.add_edge("home", "company")
    g.add_edge("home", "company")

    g.add_vertex("1")
    g.add_vertex("2")
    g.del_edge("1", "2")
    g.add_edge("1", "2")
    g.topsort()
