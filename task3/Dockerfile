FROM debian:bullseye-slim

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    libcurl4-openssl-dev \
    libtinyxml2-dev \
    libiconv-hook-dev \
    g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY main.cpp .

RUN g++ main.cpp -o currency_rates -lcurl -ltinyxml2 -liconv -std=c++17

CMD ["./currency_rates"]
