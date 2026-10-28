# Multi-stage Dockerfile for Garuda BEAM & RICHAT Production Server
# Stage 1: Build stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    ninja-build \
    gcc \
    g++ \
    libssl-dev \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN chmod +x scripts/build_linux.sh scripts/run_tests.sh
RUN ./scripts/build_linux.sh

# Stage 2: Runtime stage
FROM ubuntu:22.04 AS runner

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    ca-certificates \
    libssl3 \
    libsqlite3-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/garuda_richat_server /app/garuda_richat_server

EXPOSE 9001

CMD ["/app/garuda_richat_server", "9001", "/app/data"]
