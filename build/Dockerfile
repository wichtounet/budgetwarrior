FROM wichtounet/cpp:alpine-latest

# Install dependencies
RUN apk add pipx
RUN pipx install yfinance

# Set the locale
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8
ENV LC_COLLATE C
