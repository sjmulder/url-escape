FROM debian AS build
RUN apt-get update && \
    apt-get install -y build-essential pkg-config libcurl4-openssl-dev && \
    apt-get clean
WORKDIR /work
COPY . .
RUN make install DESTDIR=dest

FROM debian
RUN apt-get update && \
    apt-get install -y libcurl4 && \
    apt-get clean
COPY --from=build /work/dest /
CMD ["/usr/local/bin/url-escape"]
