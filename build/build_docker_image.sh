#!/bin/bash

set -e

docker build -f Dockerfile -t budgetwarrior:build .
docker tag budgetwarrior:build wichtounet/budgetwarrior:build
docker push wichtounet/budgetwarrior:build
