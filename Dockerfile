FROM docker.pkg.github.com/mppmu/gerda-sw-all/gerda-sw-all:6.1.1-dev

USER root

ENV PATH="$PATH:/opt/bin"
COPY . /opt/src/gerda-larmap

WORKDIR /opt/src/gerda-larmap
RUN make -j"$(nproc)" && PREFIX=/opt make install

WORKDIR /data
CMD /bin/bash
