#! /usr/bin/env python
# -*- coding:utf-8 -*-


'''简单回显服务器'''


import sys


__author__ = "ryuuzaki"
__copyright__ = "copywrite 2014, ryuuzaki"
__credits__ = "ryuuzaki"
__license__ = "gpl"
__version__ = "1.0"
__maintainer__ = "ryuuzaki"
__email__ = "jackzxty@126.com"
__status__ = "development"


if "__main__" == __name__:
    import socket
    import time

    addr = '0.0.0.0'
    port = 9999
    print "usage: %s [port]" % sys.argv[0]

    if len(sys.argv) > 1:
        try:
            digit = int(sys.argv[1])
            if digit < 65535 and digit > 1024:
                port = digit
        except:
            print '[ERROR] invalid argument'
            sys.exit(1)


    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((addr, port))
    s.listen(1000)
    while True:
        conn, addr = s.accept();
        conn.setblocking(1)
        while True:
            try:
                buf = conn.recv(1024)
                if 0 == len(buf):
                    break;
                print buf
            except socket.error:
                break
        conn.close()

    sys.exit(0)
