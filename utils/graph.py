#! /usr/bin/env python
# -*- coding:utf-8 -*-


import sys


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

        def del_edge(self, to):
            del self.__edges[to]

        def printme(self):
            for (k, v) in self.__edges.items():
                sys.stdout.write("%s(%d)|" % (k, v.get_weight()))
            sys.stdout.write("\n")

    ''' graph '''
    def __init__(self):
        self.__vertexes = {}

    def add_vertex(self, name):
        self.__vertexes[name] = graph.vertex()

    def add_edge(self, frm, to, weight = 0):
        if frm not in self.__vertexes.keys():
            self.add_vertex(frm)

        if to not in self.__vertexes.keys():
            self.add_vertex(to)

        self.__vertexes[frm].add_edge(to, weight)

    def printme(self):
        for (k, v) in self.__vertexes.items():
            sys.stdout.write("%s: " % k)
            v.printme()


if "__main__" == __name__:
    g = graph()
    g.add_edge("home", "school")
    g.add_edge("home", "company")
    g.printme()
