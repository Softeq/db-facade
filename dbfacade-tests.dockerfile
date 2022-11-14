#
# Docker image to test Softeq dbfacade library
#
FROM dbfacade_builder

RUN apt-get -qq update && \
    apt-get -q -y upgrade

# install dependendant packages
RUN apt-get install -y \
    libaio1 \
    mysql-server \
    libmysqlclient21

# install Linux Common Lib dependendant packages
RUN apt-get install -y \
    libcurl4 \
    systemd \
    libmicrohttpd12

RUN apt-get clean && rm -rf /var/lib/apt/lists/*

RUN usermod -d /var/lib/mysql/ mysql

COPY ./docker_entrypoint.sh /start.d/001-dbfacade.sh
RUN chmod 755 /start.d/001-dbfacade.sh
