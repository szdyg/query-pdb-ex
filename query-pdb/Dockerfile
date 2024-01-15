FROM ubuntu:22.04

MAINTAINER szdyg

RUN apt-get update && \
    mkdir -p /pdb

RUN apt-get install -y \
    build-essential \
    cmake \
    supervisor

COPY . /query-pdb/

RUN cd /query-pdb && \
    mkdir -p build && \
    cd build && \
    cmake .. && \
    cmake --build . --target query_pdb_server

WORKDIR /query-pdb
ENTRYPOINT ["/query-pdb/build/server/query_pdb_server"]
