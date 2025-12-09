#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace
{
    constexpr const char *HOST = "kv_server";
    constexpr int PORT = 5000;
    constexpr int THREADS = 64;
    constexpr int REQS_PER_THREAD = 500;

    int connect_once()
    {
        // Resolve HOST and PORT using getaddrinfo (lots of stuff to learn)
        addrinfo hints{};
        hints.ai_family = AF_INET;       // IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP

        addrinfo *res = nullptr;
        std::string port_str = std::to_string(PORT);

        int err = getaddrinfo(HOST, port_str.c_str(), &hints, &res);
        if (err != 0)
        {
            std::cerr << "getaddrinfo: " << gai_strerror(err) << "\n";
            std::exit(1);
        }

        int fd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (fd < 0)
        {
            perror("socket");
            freeaddrinfo(res);
            std::exit(1);
        }

        if (connect(fd, res->ai_addr, res->ai_addrlen) < 0)
        {
            perror("connect");
            freeaddrinfo(res);
            std::exit(1);
        }

        freeaddrinfo(res);

        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
        return fd;
    }

    void write_all(int fd, const char *data, size_t len)
    {
        size_t off = 0;
        while (off < len)
        {
            ssize_t n = ::send(fd, data + off, len - off, 0);
            if (n < 0)
            {
                if (errno == EINTR)
                    continue;
                perror("send");
                std::exit(1);
            }
            off += (size_t)n;
        }
    }

    bool readline(int fd, std::string &out)
    {
        out.clear();
        char c;
        while (true)
        {
            ssize_t n = ::recv(fd, &c, 1, 0);
            if (n == 0)
                return false;
            if (n < 0)
            {
                if (errno == EINTR)
                    continue;
                perror("recv");
                std::exit(1);
            }
            if (c == '\n')
                break;
            out.push_back(c);
        }
        return true;
    }

} // namespace

int main()
{
    using clock = std::chrono::steady_clock;
    std::vector<std::thread> ts;
    ts.reserve(THREADS);
    std::atomic<int> ok{0};

    auto t0 = clock::now();
    for (int i = 0; i < THREADS; ++i)
    {
        ts.emplace_back([&ok]
                        {
            int fd = connect_once();
            std::string line;
            for (int j = 0; j < REQS_PER_THREAD; ++j) {
                write_all(fd, "PING\n", 5);
                if (!readline(fd, line)) break;
                if (line == "PONG") ++ok;
            }
            ::close(fd); });
    }
    for (auto &t : ts)
        t.join();
    auto t1 = clock::now();

    const int total = THREADS * REQS_PER_THREAD;
    double sec = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "OK replies: " << ok.load() << "/" << total
              << "  |  time: " << sec << " s"
              << "  |  approx throughput: " << (ok.load() / sec) << " req/s\n";
}
