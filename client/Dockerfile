FROM debian:12-slim

RUN apt-get update && \
    apt-get install -y g++ make && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy only the client source
COPY kv_client.cpp .

# Build the client
RUN g++ -O2 -std=c++20 -pthread kv_client.cpp -o kv_client

# Default command: run the client
CMD ["./kv_client"]
