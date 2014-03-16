#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

#include <iostream>
#include <cstdlib>
#include <cstring>


#define addr "192.168.1.105"
#define port 9999
#define unblocking_fd(fd) ::fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

using namespace std;

int main(int argc, char *argv[])
{
    int s;
    int state;
    struct sockaddr_in svr;

    s = ::socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == s) {
        cout << "socket failed" << endl;

        return EXIT_FAILURE;
    }

    static_cast<void>(memset(&svr, 0, sizeof(svr)));
    svr.sin_family = AF_INET;
    svr.sin_addr.s_addr = inet_addr(addr);
    svr.sin_port = htons(9999);
    errno = 0;
    if (-1 == ::connect(s, reinterpret_cast<struct sockaddr *>(&svr),
                        sizeof(struct sockaddr)))
    {
        cout << "connect failed: " << errno << endl;
        ::close(s);

        return EXIT_FAILURE;
    }

    static_cast<void>(unblocking_fd(s));

    state = 0;
    setsockopt(s, IPPROTO_TCP, TCP_CORK, &state, sizeof(state));
    #define data "abcd"
    if (-1 == send(s, data, sizeof(data) - 1, 0)) {
        cout << "connect failed" << endl;
        ::close(s);

        return EXIT_FAILURE;
    }
    sleep(60);
    state = 1;
    setsockopt(s, IPPROTO_TCP, TCP_CORK, &state, sizeof(state));

    ::close(s);

    return EXIT_SUCCESS;
}
