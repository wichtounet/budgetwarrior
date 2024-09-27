#!/root/.local/share/pipx/venvs/yfinance/bin/python3

import yfinance as yf
import sys

import json as js

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Invalid arguments")
        exit(1)

    ticker_str = sys.argv[1]
    ticker_start_date = sys.argv[2]
    ticker_end_date = sys.argv[3]

    ticker = yf.Ticker(ticker_str)
    data = ticker.history(start = ticker_start_date, end = ticker_end_date)

    for v in data["Close"].keys():
        print("{:%Y-%m-%d}:{:.2f}".format(v, data["Close"][v]))
