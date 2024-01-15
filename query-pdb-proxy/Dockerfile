FROM golang:alpine

MAINTAINER szdyg "szdyg@outlook.com"

ARG GOPROXY=https://goproxy.cn,direct

COPY . /query-pdb-proxy/

RUN mkdir -p /pdb && \
    cd /query-pdb-proxy && \
    go build 

WORKDIR /query-pdb-proxy
ENTRYPOINT ["/query-pdb-proxy/query-pdb-proxy"]
