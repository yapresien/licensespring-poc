FROM ubuntu:22.04
LABEL maintainer="anshul.yadav@presien.com"

# Install dependencies.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       apt-utils \
       build-essential \
       cmake \
       git \
       software-properties-common \
       rsyslog systemd systemd-cron sudo \
    && apt-get clean \
    && rm -Rf /var/lib/apt/lists/* \
    && rm -Rf /usr/share/doc && rm -Rf /usr/share/man
RUN sed -i 's/^\($ModLoad imklog\)/#\1/' /etc/rsyslog.conf


RUN git clone https://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@github.com/yapresien/licensespring-poc.git

WORKDIR /licensespring-poc/sources/
RUN git reset --hard
RUN git pull

RUN chmod +x build.sh
RUN bash ./build.sh
#deactivate
RUN ./release_static/presien-lic-app 1 

#deactivate
RUN ./release_static/presien-lic-app 0
 