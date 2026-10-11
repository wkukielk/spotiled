FROM nginxinc/nginx-unprivileged:alpine
WORKDIR /usr/share/nginx/

COPY nginx-default.conf /etc/nginx/conf.d/default.conf
COPY --chown=nginx:nginx ssl/ /etc/nginx/ssl/

EXPOSE 8443
