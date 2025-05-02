FROM alpine:latest
ARG PB_VERSION=0.27.2
# Install unzip and curl to fetch the binary
RUN apk add --no-cache curl unzip \
  && curl -fsSL \
     "https://github.com/pocketbase/pocketbase/releases/download/v${PB_VERSION}/pocketbase_${PB_VERSION}_linux_amd64.zip" \
     -o /tmp/pocketbase.zip \
  && unzip /tmp/pocketbase.zip -d /usr/local/bin/ \
  && rm /tmp/pocketbase.zip \
  && chmod +x /usr/local/bin/pocketbase

EXPOSE 8090

CMD ["pocketbase", "serve", "--http=0.0.0.0:8090"]
