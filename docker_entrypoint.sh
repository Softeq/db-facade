#!/bin/bash

set -e

echo -n "Adding user '$DOCKER_USER' to group 'mysql' : " && \
    usermod -a -G mysql $DOCKER_USER && \
    echo "done"

#mysql-daemon starts too slow. Start using `CMAKE_EXTRA_ARGS="-e TEST_MODE=1" cmake_build ...`
#so the test container is an additional image with the tag <image>:tests

service mysql start

mysql -u root  << EOF
 CREATE DATABASE IF NOT EXISTS db;
 USE db;
 CREATE USER 'user'@'localhost' IDENTIFIED BY 'secret';
 GRANT ALL PRIVILEGES ON db.* TO 'user'@'localhost';
 FLUSH PRIVILEGES;
EOF
