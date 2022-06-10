#!/bin/bash
set -x
pkill nginx
kill $(lsof -ti:8080)

cat << EOF > test.conf
error_log $PWD/nginxerror.log warn;
http {
    access_log $PWD/nginxaccess.log;
    server {
        listen         8080;
        root $PWD;
    }
}
events {
}
EOF

nginx -c $PWD/test.conf
curl 127.0.0.1:8080 > nginx_out
pkill nginx
kill $(lsof -ti:8080)

pkill webserv
../webserv test.conf &
sleep 1
curl 127.0.0.1:8080 > webserv_out

diff nginx_out webserv_out
pkill webserv
