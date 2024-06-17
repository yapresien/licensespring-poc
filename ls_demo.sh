#!/bin/bash
rm -f ./Dockerfile
wget https://raw.githubusercontent.com/yapresien/licensespring-poc/main/Dockerfile

docker build --progress=plain . -t lsdemo:1.0
