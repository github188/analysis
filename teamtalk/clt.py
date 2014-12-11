#! /usr/bin/env python
# -*- coding:utf8 -*-


import sys
import struct
import tornado.tcpclient
import tornado.ioloop


class TTMsgParser(object):
    def __init__(self):
        self.__data = ""

    def pack_tt_msg(self, module_id, command_id, body):
        package = ""
        package += struct.pack("!I", 12 + len(body))
        package += struct.pack("!H", module_id)
        package += struct.pack("!H", command_id)
        package += struct.pack("!H", 1)
        package += struct.pack("!H", 0)
        package += body

        return package

    def unpack_tt_msg(self, data):
        rslt = []
        self.__data += data

        while len(self.__data) >= 12:
            head = struct.unpack("!I4H", self.__data[0 : 12])
            (pdu_length, module_id, command_id, version, reserved) = head
            body = self.__data[12 : pdu_length]
            self.__data = self.__data[pdu_length:]
            item = list(head)
            item.append(body)
            rslt.append(tuple(item))

        return rslt


class TTClient(object):
    def __init__(self):
        self.__tcpclt = tornado.tcpclient.TCPClient()
        self.__iolp = tornado.ioloop.IOLoop.instance()
        self.__stm = None
        self.__msg_parser = TTMsgParser()
        self.__rsps = {
        }

    def connect(self, **kwargs):
        ft = self.__tcpclt.connect(**kwargs)
        self.__iolp.add_future(ft, self.__on_connect)

    def __on_recv(self, data):
        self.__stm.read_bytes(num_bytes = 1024,
                              callback = self.__on_recv, partial = True)
        self.__msg_parser.unpack_tt_msg(data)

    def __on_connect(self, ft):
        try:
            self.__stm = ft.result()
        except Exception as err:
            print "failed to connect server:", err
            return

        self.__stm.read_bytes(num_bytes = 1024,
                              callback = self.__on_recv, partial = True)
        self.__stm.write(self.__msg_parser.pack_tt_msg(1, 1, ""))

    def run(self):
        self.__iolp.start()


if "__main__" == __name__:
    clt = TTClient()

    clt.connect(host = "120.24.217.4", port = 8008)
    clt.run()

    sys.exit(0)
