# **KV TCP Client–Server Benchmarking Project**

This project implements a high performance multithreaded TCP server and a benchmarking client written in C++. The server supports simple commands such as `PING`, `ECHO`, `SET`, and `GET`. The client sends many parallel requests and measures throughput.

The project includes Dockerfiles for both the server and client, along with a `docker-compose.yml` file that allows both services to run in isolated containers with a single command.

---

## **Project Structure**

```
.
├── client/
│   ├── Dockerfile
│   └── kv_client.cpp
├── server/
│   ├── Dockerfile
│   └── kv_server.cpp
├── docker-compose.yml
```

---

# **Features**

### **Server**

* Multithreaded architecture using worker threads
* Simple text protocol:

  * `PING` → `PONG`
  * `ECHO <msg>` → `<msg>`
  * `SET <key> <value>`
  * `GET <key>`
* Line based message parsing
* Graceful shutdown logic
* Uses a work queue with condition variables

### **Client**

* Multi threaded TCP load generator
* Sends many `PING` requests and counts successful replies
* Prints total throughput and timing statistics
* Uses DNS based container networking when run under Docker

---

# **Run Using Docker Compose (recommended)**

Make sure you are in the project root (where `docker-compose.yml` is located).

### **Build and run both server and client**

```bash
docker compose up --build
```

Docker will:

1. Build the server and client images
2. Start the server container
3. Start the client container and automatically connect it to the server
4. Print throughput results from the client

### **Stop everything**

```bash
docker compose down
```

---

# **Manual Build (without Docker)**

### **Build server**

```bash
g++ -O2 -std=c++20 -pthread server/kv_server.cpp -o kv_server
```

### **Build client**

```bash
g++ -O2 -std=c++20 -pthread client/kv_client.cpp -o kv_client
```

### **Run server**

```bash
./kv_server
```

### **Run client (requires server running on localhost:5000)**

```bash
./kv_client
```

---

# **Testing Connectivity**

To test locally on Windows:

```powershell
Test-NetConnection -ComputerName 127.0.0.1 -Port 5000
```

This confirms whether the server is listening on port 5000.

---

# **Configuration Notes**

* Inside Docker, containers communicate by service name, not localhost.
  The client resolves `kv_server` through Docker's internal DNS.
* When running outside Docker, the client defaults to `127.0.0.1`.
* The client uses `getaddrinfo` to support both environments.

---

# **Future Improvements**

* Add a key value persistence layer
* Switch to epoll or io_uring for higher concurrency
* Add automated benchmarking graphs
* Extend Docker setup with a metrics dashboard

---

⡐⢂⠲⡐⢢⢀⠂⡔⠠⠒⡄⢃⡒⠔⡂⠖⡐⢆⠒⡔⢂⠲⡐⢂⠖⡐⢢⠒⡰⢂⡈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⠰⡉⢆⢡⠃⣄⠣⢀⠣⠄⢘⡡⠘⡄⢣⢘⠰⣈⠒⡌⡘⠤⡑⢊⠔⡉⢆⠱⢠⠃⡄⣹⣿⣿⣿⣿⣿⣿⣿⣿⣿⢿⣻⡽⣯⠟
⢂⠱⡈⠆⣸⢆⠡⢂⠒⣈⠐⡄⢋⠔⡡⢊⠔⢢⠑⡄⢃⡒⢡⠊⡔⢡⠊⡔⢡⠊⡔⢙⣿⣿⣿⣿⣿⣿⣿⣿⣟⣯⢿⡽⢧⢋
⡈⢆⡑⢲⢻⣌⡳⣌⢶⣠⢃⠘⠤⢊⠔⡡⢊⠤⢃⠜⡰⢈⠆⡱⢈⠆⡱⢈⠆⡱⢈⠜⣿⣿⣿⣿⣿⣿⣿⣿⢯⣟⣯⣟⡯⢆
⡐⢂⠀⡀⠁⢚⠿⣽⣻⢾⣭⠎⡐⢣⠘⡄⢣⠘⡄⢎⠰⡁⢎⠰⡁⢎⠰⡁⢎⠰⡁⠆⢿⡿⣿⣿⣿⣿⡿⣯⣿⣻⢾⣽⣛⡆
⡘⢄⣀⠀⠸⣿⣿⣶⣭⡛⢾⣓⠈⢆⠱⡈⢆⠱⡈⢆⠱⡈⢆⠱⡈⢆⠱⡈⢆⠱⣈⠱⡘⠿⡽⢻⡞⣷⢻⢯⠷⣟⠿⠾⠝⠂
⠌⣸⣿⢿⡄⢻⣿⣿⣿⣿⣷⣦⣉⠢⡑⢌⠢⡑⢌⠢⡑⢌⠢⡑⢌⠢⡑⢌⠢⡑⠤⢃⠄⠰⢀⠃⡐⠠⠂⣀⣢⣴⣶⣿⡇⠀
⢲⣿⢯⣟⣷⡘⣿⣿⣿⣿⣿⣿⣿⣷⣮⡀⢇⡘⢄⠣⡘⢄⠣⡘⢄⠣⡘⢢⢑⡘⡰⢉⠜⠀⡌⣤⣴⣶⣿⣿⣿⣿⣿⣿⠇⠀
⣿⢯⣟⡿⣞⠳⡘⢿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡘⠄⢣⣜⣀⣃⣘⣂⣡⣑⣂⣈⢐⣁⣎⣶⣷⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠀⡐
⣿⢯⡿⣽⢏⢣⠐⡈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⣀⠔⡠
⠌⢋⡙⠤⠋⢄⠃⠤⠁⠙⢿⣿⡿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⣡⣾⢯⣟⡔
⠀⠂⠀⠄⢁⠂⠌⠤⢁⠂⠀⢡⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣯⡻⢋⣥⣾⡿⣯⣟⡾⡐
⠀⠀⠀⠀⠀⡈⠰⢈⠆⡘⢠⣿⣿⣿⣿⡿⠛⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⢿⣿⣿⣿⣿⣿⣷⡜⢯⣷⣻⢷⡻⣜⠡
⠀⠀⠀⠀⠀⢀⠡⢊⠔⣉⣾⣿⣿⣿⣿⠘⠛⠀⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠘⠟⠀⣿⣿⣿⣿⣿⣿⣿⡨⢓⡹⢎⡱⢌⠡
⠀⠀⠀⠀⠀⠀⠂⠥⡚⣼⣿⣿⣿⣿⣿⣶⣤⣾⣿⣿⣿⡿⠿⣿⣿⣿⣿⣿⣿⣷⣤⣶⣿⣿⣿⣿⣿⣿⣿⣇⠡⠒⢄⠒⡨⠐
⠀⠀⠀⠀⠀⠠⡉⢦⢡⡿⢟⡻⠟⠿⣿⣿⣿⣿⣿⣿⣿⣷⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⡿⢟⢻⡛⢻⣿⣿⣿⡄⢡⠊⠤⡑⠌
⢀⡄⣄⢢⣌⡵⣜⣮⢹⣡⠚⣔⢋⢎⣹⣿⣿⣿⣿⣿⣿⡿⠿⠿⠿⢿⣿⣿⣿⣿⣿⣿⠔⣊⠦⣍⢣⢾⣿⣿⡧⢮⣝⣲⢡⢊
⣾⣼⣞⡷⣾⣽⣻⢾⣸⣷⣼⣬⣷⣴⣿⣿⣿⣿⣿⣿⡏⣾⣿⣿⣿⣷⢹⣿⣿⣿⣿⣿⣲⣩⣶⣬⣶⣾⣿⣿⣿⠾⣽⣳⣏⠆
⣿⢾⣽⣻⢷⣯⣟⣯⣧⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣙⣿⠿⡿⣏⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣿⣳⢿⡌
⣿⢯⣷⢿⣻⡾⣽⣳⣯⢧⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⣾⣽⡻⡔
⣟⡿⣞⣯⢷⣻⣽⣳⢯⡟⣇⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠓⣯⠳⠌
⠸⢹⠙⡎⢏⠳⢍⠫⡙⡜⢄⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣆⠣⡙⡐
