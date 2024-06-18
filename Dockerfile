FROM ubuntu:22.04
LABEL maintainer="anshul.yadav@presien.com"

# Install dependencies.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       apt-utils \
       build-essential \
       cmake \
       git \
       pkg-config \
       libcpuinfo-dev \
       software-properties-common \
       rsyslog systemd systemd-cron sudo \
    && apt-get clean \
    && rm -Rf /var/lib/apt/lists/* \
    && rm -Rf /usr/share/doc && rm -Rf /usr/share/man
RUN sed -i 's/^\($ModLoad imklog\)/#\1/' /etc/rsyslog.conf

# WORKDIR /
# RUN mkdir -p sources
# WORKDIR /sources
# RUN cd $WORDKDIR

## dependency cpuinof source
RUN git clone https://github.com/pytorch/cpuinfo.git
WORKDIR /cpuinfo
#RUN pwd
#RUN ls -ll
RUN git pull
RUN mkdir -p ./build
RUN cd ./build
RUN cmake /cpuinfo
RUN cmake --build .
RUN make install


WORKDIR /
RUN git clone --depth 1 https://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@github.com/yapresien/licensespring-poc.git


WORKDIR /licensespring-poc/sources/
RUN git reset --hard
ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache
RUN git pull

RUN chmod +x build.sh
RUN bash ./build.sh

#deactivate
WORKDIR /licensespring-poc/sources/release_static

RUN export PATH=$PATH:$WORKDIR
# below command will force to RUN everytime 
# ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache

CMD ["/licensespring-poc/sources/release_static/presien-lic-app" , "install" ]

