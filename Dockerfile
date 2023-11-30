FROM debian AS build
RUN apt-get update && \
    apt-get install -y --no-install-recommends gcc make libc6-dev && \
    apt-get clean
WORKDIR /work
COPY . .
RUN env LDFLAGS=-static make clean all

FROM scratch
COPY --from=build /work/url-escape /
CMD ["/url-escape"]
