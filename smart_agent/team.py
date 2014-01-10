#! /usr/bin/env python

import os
import sys
import time
import socket
import select
import multiprocessing

def team(*args):
    epfd = select.epoll()
    print "pid:", os.getpid()

    try:
        servers = {}

        for sock in args:
            servers[sock.fileno()] = sock
            epfd.register(sock.fileno(), select.EPOLLIN | select.EPOLLET)

        while True:
            data, addr = None, None
            events = epfd.poll(0.1)
            for fileno, event in events:
                if event & select.EPOLLIN:
                    time.sleep(1)
                    try:
                        while True:
                            (data, addr) = servers[fileno].recvfrom(1500)
                            print addr, data
                        servers[fileno].sendto('aaa', addr)
                    except Exception:
                        pass
                if event & select.EPOLLHUP:
                    print 'epoll hup'
                if event & select.EPOLLOUT:
                    print 'epoll out'

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
