FROM yijun/fast
RUN apk add -U --no-cache --force-broken-world git make g++ protobuf-dev parallel
ADD . /git-dl
RUN cd /git-dl \
 && make \
 && make install
ENTRYPOINT ["/usr/bin/git", "dl", "$@"]
