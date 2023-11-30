FROM debian AS build
RUN apt-get update && \
    apt-get install -y build-essential && \
    apt-get clean
WORKDIR /work
COPY . .
RUN env LDFLAGS=-static make

FROM scratch
COPY --from=build /work/url-escape /
CMD ["/url-escape"]
