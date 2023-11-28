FROM debian AS build
RUN apt-get update && \
    apt-get install -y build-essential && \
    apt-get clean
WORKDIR /work
COPY . .
RUN make install DESTDIR=dest

FROM debian
COPY --from=build /work/dest /
CMD ["/usr/local/bin/url-escape"]
