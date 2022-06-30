#!/bin/bash
set -x
pkill nginx
kill $(lsof -ti:8080)

cat << EOF > test.conf
#THIS FILE IS AUTO GENERATED!
error_log $PWD/nginxerror.log warn;
http {
    access_log $PWD/nginxaccess.log;
    server {
        listen         8080;
		location /listing {
			alias $PWD/listing;
			autoindex on;
		}
    }
}
events {
}
EOF

nginx -c $PWD/test.conf
curl 127.0.0.1:8080/listing/ -v
curl 127.0.0.1:8080/listing/hoi -v
curl 127.0.0.1:8080/listing/plaatje.png -v
pkill nginx
kill $(lsof -ti:8080)
