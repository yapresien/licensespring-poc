FROM ubuntu:22.04
LABEL maintainer="anshul.yadav@presien.com"

# Install dependencies.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       apt-utils \
       build-essential \
       cmake \
       git \
       libcpuinfo-dev \
       software-properties-common \
       rsyslog systemd systemd-cron sudo \
    && apt-get clean \
    && rm -Rf /var/lib/apt/lists/* \
    && rm -Rf /usr/share/doc && rm -Rf /usr/share/man
RUN sed -i 's/^\($ModLoad imklog\)/#\1/' /etc/rsyslog.conf


RUN git clone --depth 1 https://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@github.com/yapresien/licensespring-poc.git


WORKDIR /licensespring-poc/sources/
RUN git reset --hard
ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache
RUN git pull

RUN chmod +x build.sh
RUN bash ./build.sh

#deactivate
WORKDIR /licensespring-poc/sources/release_static

# below command will force to RUN everytime 
ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache
RUN ./presien-lic-app

