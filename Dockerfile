FROM mppmu/gerda-sw-all:7.0.0-mage-v1.1.1.p1-larmap-t4z

USER root

ENV PATH="$PATH:/opt/bin"
COPY . /opt/src/gerda-larmap

WORKDIR /opt/src/gerda-larmap
RUN make -j"$(nproc)" && PREFIX=/opt make install

WORKDIR /data
CMD /bin/bash
