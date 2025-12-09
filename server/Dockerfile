FROM debian:12-slim

RUN apt-get update && \
    apt-get install -y g++ make && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy only the server source
COPY kv_server.cpp .

# Build the server
RUN g++ -O2 -std=c++20 -pthread kv_server.cpp -o kv_server

# Expose the TCP port
EXPOSE 5000

# Default command: run the server
CMD ["./kv_server"]
