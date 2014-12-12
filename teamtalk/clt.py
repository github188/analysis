#! /usr/bin/env python
# -*- coding:utf8 -*-


import sys
import struct
import tornado.concurrent
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


class TTTCPClient(object):
    def __init__(self):
        self.__stm = None
        self.__msg_parser = TTMsgParser()
        self.__tcpclt = tornado.tcpclient.TCPClient()

    def get_msg_tuples(self, io_loop, **kwargs):
        self.__msg_tuples = tornado.concurrent.Future()
        ft = self.__tcpclt.connect(**kwargs)
        io_loop.add_future(ft, self.__on_connect)
        return self.__msg_tuples

    def __on_connect(self, ft):
        try:
            self.__stm = ft.result()
        except Exception as err:
            print "failed to connect server:", err
            self.__msg_tuples.set_exception(err)
            return

        self.__stm.read_bytes(num_bytes = 1024,
                              callback = self.__on_recv, partial = True)
        self.__stm.write(self.__msg_parser.pack_tt_msg(1, 1, ""))

    def __on_recv(self, data):
        self.__stm.read_bytes(num_bytes = 1024,
                              callback = self.__on_recv, partial = True)
        msg_tuples = self.__msg_parser.unpack_tt_msg(data)
        self.__msg_tuples.set_result(msg_tuples)


class TTClient(object):
    def __init__(self):
        self.__login_clt = TTTCPClient()
        self.__msg_clt = TTTCPClient()
        self.__iolp = tornado.ioloop.IOLoop.instance()
        self.__rsps = {
            2: self.__on_login,
        }

    def __on_login(self, ft):
        msg_tuples = None
        try:
            msg_tuples = ft.result()
        except Exception as err:
            print "failed to get msg server info:", err
            return

        for tp in msg_tuples:
            (pdu_length, module_id, command_id, version, reserved, body) = tp
            (result, ) = struct.unpack("!I", body[0 : 4])
            (ip1_length, ) = struct.unpack("!I", body[4 : 8])
            ip1 = body[8 : 8 + ip1_length]
            (ip2_length, ) = struct.unpack(
                "!I", body[8 + ip1_length : 12 + ip1_length]
            )
            ip2 = body[12 + ip1_length : 12 + ip1_length + ip2_length]
            (port, ) = struct.unpack("!H", body[-2:])

            print result, ip1_length, ip1, ip2_length, ip2, port

    def run(self):
        msg_tuples = self.__login_clt.get_msg_tuples(
            self.__iolp, host = "120.24.217.4", port = 8008
        )
        self.__iolp.add_future(msg_tuples, self.__on_login)
        self.__iolp.start()


if "__main__" == __name__:
    clt = TTClient()

    clt.run()

    sys.exit(0)
