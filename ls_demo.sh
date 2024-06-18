#!/bin/bash
rm -f ./Dockerfile
wget https://raw.githubusercontent.com/yapresien/licensespring-poc/main/Dockerfile

sudo docker build --progress=plain . -t lsdemo:1.0

# sudo docker run -e MAC1=`ip link show eth0 | grep link/ether | awk '{print $2}'` \
#   -e TARGETHOSTNAME=$HOSTNAME \
#   -e CUSTOMER_SSN="HEXAGONSSN1234" \
#   --entrypoint /bin/sh \
#   -it lsdemo:latest


#sudo docker run -v /home/presien/tmp:/PresienLic -it lsdemo:latest
#sudo docker run --entrypoint /bin/bash -it lsdemo:latest