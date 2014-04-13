#! /usr/bin/env python
# -*- coding:utf-8 -*-


import socket


if __name__ == '__main__':
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 8888))
    #sock.send('1')
    sock.shutdown(socket.SHUT_WR)
    buf = sock.recv(1024)
    if len(buf) == 0:
        sock.close()
        print "closed"
