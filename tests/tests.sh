#!/bin/bash
pkill nginx

cat << EOF > test.conf
error_log $PWD/nginxerror.log warn;
http {
    access_log $PWD/nginxaccess.log;
    server {
        listen         8000;
        root $PWD;
    }
}
events {
    worker_connections  1000;
}
EOF

nginx -c $PWD/test.conf

curl 127.0.0.1:8000 > nginx_out

pkill nginx

#pkill webserv
#../webserv test.conf
#curl 127.0.0.1:8000 > webserv_out
#diff nginx_out webserv_out

