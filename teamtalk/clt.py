#! /usr/bin/env python
# -*- coding:utf8 -*-


import sys
import tornado.tcpclient
import tornado.ioloop


class TTClient(object):
    def __init__(self):
        self.__tcpclt = tornado.tcpclient.TCPClient()
        self.__iolp = tornado.ioloop.IOLoop.instance()
        self.__stm = None

    def connect(self, **kwargs):
        ft = self.__tcpclt.connect(**kwargs)
        self.__iolp.add_future(ft, self.__on_connect)

    def __on_connect(self, ft):
        try:
            self.__stm = ft.result()
        except Exception as err:
            print "failed to connect server:", err
            return

    def run(self):
        self.__iolp.start()


if "__main__" == __name__:
    clt = TTClient()

    clt.connect(host = "120.24.217.4", port = 8008)
    clt.run()

    sys.exit(0)
