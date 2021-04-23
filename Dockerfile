FROM beequant/iridium:1.0
# Args
ARG account_type
ARG account_token
ARG account_id
ENV ACCOUNT_TYPE ${account_type}
ENV ACCOUNT_TOKEN ${account_token}
ENV ACCOUNT_ID ${account_id}
# Copy source files
COPY . /usr/src/app
WORKDIR /usr/src/app
# Build
RUN mkdir cmake-build-release \
    && cd cmake-build-release \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && cd live-trading \
    && make
# Switch user
RUN apt-get update \
    && apt-get install sudo \
    && useradd -m iridium77 \
    && usermod -g sudo iridium77 \
    && echo "iridium77 ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
USER iridium77
CMD /usr/src/app/cmake-build-release/live-trading/live-trading \
    -e ${ACCOUNT_TYPE} \
    -t ${ACCOUNT_TOKEN} \
    -a ${ACCOUNT_ID}