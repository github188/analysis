#! /usr/bin/env python
# -*- coding:utf-8 -*-


import socket


if __name__ == "__main__":
    import socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("localhost", 8888))
    sock.listen(5)
    while True:
        print "[info] wait client"
        connection,address = sock.accept()
        while True:
            buf = connection.recv(1024)
            if len(buf) == 0:
                print "closing"
                connection.close()
                break
            else:
                print buf

