#!/bin/bash
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
    worker_connections  1000;
}
EOF

nginx -c $PWD/test.conf 2> /dev/null

curl 127.0.0.1:8080 > nginx_out 2> /dev/null

pkill nginx
kill $(lsof -ti:8080)

pkill webserv
../webserv test.conf
curl 127.0.0.1:8081 > webserv_out 2> /dev/null
diff nginx_out webserv_out
