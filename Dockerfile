FROM ubuntu:latest
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        make \
        wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
RUN cd opt \
    && wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 \
    && tar xf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 \
    && rm gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2
ENV PATH=$PATH:/opt/gcc-arm-none-eabi-9-2020-q2-update/bin