FROM beequant/iridium:1.0
# Args
ARG account_type
ARG account_token
ARG account_id
ARG logger_api_key
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
# Make bash script executable
RUN chmod +x run.sh
# Switch user
RUN useradd -m iridium77
USER iridium77
# Entry point
CMD ./run.sh
