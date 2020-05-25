FROM ubuntu:latest

RUN apt-get update -qq
RUN apt-get install -y -qq gcc make git cmake libgmp-dev libmpfr-dev dh-autoreconf socat

RUN useradd -d /home/compileman -m -p compileman -s /bin/bash compileman
RUN echo "compileman:compileman" | chpasswd

WORKDIR /home/compileman

RUN git clone https://github.com/malb/dgs.git && cd dgs && autoreconf -i && ./configure && make && make install

COPY . . 

RUN mkdir build && cd build && cmake .. && make && mv <executable-name> ..
