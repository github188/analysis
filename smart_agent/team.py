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
    time.sleep(3)

    return 0


def do_fork(ps_list, *srvs):
    for i in xrange(multiprocessing.cpu_count()):
        rslt = 0
        try:
            rslt = os.fork()
        except OSError:
            sys.exit(1)

        if 0 == rslt:
            team(srvs)
            sys.exit(0)
        else:
            continue


if "__main__" == __name__:
    jobs = []
    servfd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    servfd.setblocking(0)
    servfd.bind(('0.0.0.0', 10001))

    print "pid:", os.getpid()
    do_fork(jobs, servfd)
    while True:
        try:
            exit_info = os.wait()
        except OSError:
            break;
        print exit_info

    sys.exit(0)
