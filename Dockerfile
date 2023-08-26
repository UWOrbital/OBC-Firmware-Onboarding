FROM ubuntu:latest

WORKDIR /app

RUN apt-get update && apt-get install -y gcc make

COPY . /app
