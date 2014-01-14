#! /usr/bin/env python
# -*- coding:utf-8 -*-

import os
import sys
import time
import socket
import select
import multiprocessing

def recv_data(*active_fds):
    print active_fds
    for fd in active_fds:
        data, addr = "", None
        while True:
            try:
                tmp_data, addr = fd.recvfrom(1500)
                data += tmp_data
            except Exception:
                print "recv failed"
                break
        print addr, data


def team(*args):
    time.sleep(5)
    print "pid:", os.getpid()

    servfd.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    for sock in args:
        sock.sendto("vote|test",
                    ('<broadcast>', 10001))
    servfd.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 0)

    epfd = select.epoll()

    try:
        servers = {}

        for sock in args:
            servers[sock.fileno()] = sock
            epfd.register(sock.fileno(), select.EPOLLIN | select.EPOLLET)

        while True:
            active_fds = []

            events = epfd.poll(1)

            for fileno, event in events:
                if event & select.EPOLLIN:
                    active_fds.append(servers[fileno])
                if event & select.EPOLLHUP:
                    print 'epoll hup'
                if event & select.EPOLLOUT:
                    print 'epoll out'

            if active_fds:
                recv_data(*active_fds)

    finally:
        for sock in args:
            epfd.unregister(sock.fileno())
            epfd.close()

    return 0


def do_fork(ps_list, *srvs):
    for i in xrange(multiprocessing.cpu_count()):
        p = multiprocessing.Process(target = team, args = srvs)
        p.start()
        ps_list.append(p)


if "__main__" == __name__:
    jobs = []
    test = True
    servfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    servfd.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    servfd.setblocking(0)
    servfd.bind(('0.0.0.0', 10001))

    print "pid:", os.getpid()
    if test:
        team(servfd)
    else:
        do_fork(jobs, servfd)
    while True:
        try:
            exit_info = os.wait()
        except OSError:
            break;
        print exit_info

    sys.exit(0)
