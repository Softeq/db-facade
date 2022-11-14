#
# Docker image to build Softeq dbfacade library
#
FROM cmake_builder:ubuntu20

RUN apt-get -qq update && \
    apt-get -q -y upgrade

# install dependendant packages
RUN apt-get install -y \
    libsqlite3-dev \
    libmysqlclient-dev

# install Linux Common Lib dependendant packages
RUN apt-get install -y \
    libcurl4-openssl-dev \
    libsystemd-dev \
    libmicrohttpd-dev \
    nlohmann-json3-dev \
    libxml2-dev \
    uuid-dev

RUN apt-get clean && rm -rf /var/lib/apt/lists/*


