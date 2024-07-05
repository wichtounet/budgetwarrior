FROM ubuntu:latest

# Install dependencies
RUN apt-get update && apt-get -y upgrade && apt-get -y install make gcc-14 g++-14 libssl-dev uuid-dev libcurl4-openssl-dev gdb openssl locales pipx && rm -rf /var/lib/apt/lists/*
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 60 --slave /usr/bin/g++ g++ /usr/bin/g++-14
RUN pipx install yfinance

# Set the locale
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8
ENV LC_COLLATE C
