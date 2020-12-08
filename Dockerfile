FROM ubuntu:latest
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        wget \
        make \
        gcc-arm-none-eabi \
        libnewlib-arm-none-eabi \
        gdb-multiarch \
        qemu-system-arm \
        tightvncserver \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
EXPOSE 5901 1234