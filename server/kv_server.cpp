#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <condition_variable>
#include <csignal>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr int PORT = 5000;
    constexpr int BACKLOG = 256;
    constexpr int WORKERS = 32;
    constexpr size_t BUFSZ = 4096;

    std::unordered_map<std::string, std::string> store;
    std::mutex store_m;

    // ---- simple fixed-size work queue of accepted client fds ----
    std::queue<int> q;
    std::mutex q_m;
    std::condition_variable q_cv;
    bool shutting_down = false;

    void enqueue(int fd)
    {
        std::lock_guard<std::mutex> lk(q_m);
        q.push(fd);
        q_cv.notify_one();
    }

    std::optional<int> dequeue()
    {
        std::unique_lock<std::mutex> lk(q_m);
        q_cv.wait(lk, []
                  { return shutting_down || !q.empty(); });
        if (shutting_down && q.empty())
            return std::nullopt;
        int fd = q.front();
        q.pop();
        return fd;
    }

    // ---- protocol ----
    std::string process_line(std::string line)
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            return "ERR empty";

        auto first_space = line.find(' ');
        std::string cmd = (first_space == std::string::npos) ? line : line.substr(0, first_space);
        for (auto &c : cmd)
            c = toupper(c);

        if (cmd == "PING")
            return "PONG";
        if (cmd == "ECHO")
        {
            if (first_space == std::string::npos)
                return "ERR usage";
            return line.substr(first_space + 1);
        }
        if (cmd == "SET")
        {
            // SET <k> <v>
            if (first_space == std::string::npos)
                return "ERR usage";
            auto second_space = line.find(' ', first_space + 1);
            if (second_space == std::string::npos)
                return "ERR usage";
            std::string k = line.substr(first_space + 1, second_space - first_space - 1);
            std::string v = line.substr(second_space + 1);
            {
                std::lock_guard<std::mutex> g(store_m);
                store[k] = v;
            }
            return "OK";
        }
        if (cmd == "GET")
        {
            if (first_space == std::string::npos)
                return "ERR usage";
            std::string k = line.substr(first_space + 1);
            std::lock_guard<std::mutex> g(store_m);
            auto it = store.find(k);
            return it == store.end() ? "NULL" : it->second;
        }
        return "ERR unknown";
    }

    void handle_connection(int fd)
    {
        // latency for small writes: disable Nagle
        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

        std::string acc;
        acc.reserve(2 * BUFSZ);
        char buf[BUFSZ];

        while (true)
        {
            ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
            if (n == 0)
                break; // client closed
            if (n < 0)
            { // error
                if (errno == EINTR)
                    continue;
                break;
            }
            acc.append(buf, buf + n);
            for (;;)
            {
                auto pos = acc.find('\n');
                if (pos == std::string::npos)
                    break;
                std::string line = acc.substr(0, pos);
                acc.erase(0, pos + 1);
                std::string out = process_line(line);
                out.push_back('\n');
                ssize_t sent = ::send(fd, out.data(), out.size(), 0);
                if (sent < 0)
                {
                    if (errno == EINTR)
                        continue;
                    break;
                }
            }
        }
        ::close(fd);
    }

    void worker_loop(int id)
    {
        (void)id;
        while (true)
        {
            auto maybe_fd = dequeue();
            if (!maybe_fd.has_value())
                return;
            handle_connection(*maybe_fd);
        }
    }

    int make_listen_socket(int port)
    {
        int srv = ::socket(AF_INET, SOCK_STREAM, 0);
        if (srv < 0)
        {
            perror("socket");
            std::exit(1);
        }
        int yes = 1;
        setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(srv, (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            std::exit(1);
        }
        if (listen(srv, BACKLOG) < 0)
        {
            perror("listen");
            std::exit(1);
        }
        return srv;
    }

    void sigint_handler(int)
    {
        {
            std::lock_guard<std::mutex> lk(q_m);
            shutting_down = true;
        }
        q_cv.notify_all();
    }

} // namespace

int main()
{
    std::signal(SIGINT, sigint_handler);
    int srv = make_listen_socket(PORT);
    std::cerr << "Server listening on :" << PORT << "\n";

    // spawn worker threads
    std::vector<std::thread> workers;
    workers.reserve(WORKERS);
    for (int i = 0; i < WORKERS; ++i)
        workers.emplace_back(worker_loop, i);

    // accept loop
    while (!shutting_down)
    {
        int cfd = ::accept(srv, nullptr, nullptr);
        if (cfd < 0)
        {
            if (errno == EINTR)
                continue;
            perror("accept");
            break;
        }
        enqueue(cfd);
    }

    // shutdown
    {
        std::lock_guard<std::mutex> lk(q_m);
        shutting_down = true;
    }
    q_cv.notify_all();
    for (auto &t : workers)
        t.join();
    ::close(srv);
    std::cerr << "Bye\n";
}
