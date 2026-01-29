FROM nginx:alpine
WORKDIR /usr/share/nginx/

COPY nginx-default.conf /etc/nginx/conf.d/default.conf
COPY ssl/ /etc/nginx/ssl/

EXPOSE 443
