FROM wichtounet/cpp:latest

# Install dependencies
RUN apt-get update && apt-get -y upgrade && apt-get -y install pipx && rm -rf /var/lib/apt/lists/*
RUN pipx install yfinance

# Set the locale
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8
ENV LC_COLLATE C
