import socket


def multi_connect(host, port, sockfds):
    for sock_fd in sockfds:
        sock_fd.connect((host, port))


def create_sockfds(num=8):
    sock_fds = []
    for i in range(num):
        fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock_fds.append(fd)
    return sock_fds


def close_sockfds(sockfds):
    for fd in sockfds:
        fd.close()


if __name__ == '__main__':
    host, port = socket.gethostname(), 9981
    sockfds = create_sockfds(500)
    multi_connect(host, port, sockfds)
    close_sockfds(sockfds)

# do not close it will cause busy loop
