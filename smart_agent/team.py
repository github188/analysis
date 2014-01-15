#! /usr/bin/env python
# -*- coding:utf-8 -*-


import os
import sys
import time
import uuid
import socket
import select


master = str()
collaboration = True
groupname = "test"
grouplist = []
myname = str(uuid.uuid1())


def recv_data(active_fds):
    print "recv_data"
    for fd in active_fds:
        data, addr = [], None

        while True:
            try:
                tmp_data, addr = fd.recvfrom(1500)
                data.append(tmp_data)
            except Exception:
                break
        print data, addr


def do_collaboration(sock):
    collaboration = True

    ''' 请求成员列表 '''
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.sendto("collaboration|member" + "|" + groupname + "|" + myname,
                ('<broadcast>', 10001))
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 0)


def team(sock):
    epfd = select.epoll()

    try:
        epfd.register(sock.fileno(), select.EPOLLIN | select.EPOLLET)

        while True:
            active_fds = []

            events = epfd.poll(3)

            for fileno, event in events:
                if event & select.EPOLLIN:
                    active_fds.append(sock)
                if event & select.EPOLLHUP:
                    print 'epoll hup'
                if event & select.EPOLLOUT:
                    print 'epoll out'

            if active_fds:
                recv_data(active_fds)

            if master:
                continue

            ''' 发起分布式协同 '''
            #do_collaboration(sock)

    finally:
        epfd.unregister(sock.fileno())
        epfd.close()

    return 0


if "__main__" == __name__:
    jobs = []
    srv = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.setblocking(0)
    srv.bind(('0.0.0.0', 10001))

    team(srv)

    sys.exit(0)
