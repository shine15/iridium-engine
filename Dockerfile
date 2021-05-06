FROM beequant/iridium:1.0
# Args
ARG account_type
ARG account_token
ARG account_id
ARG logger_api_key
ENV ACCOUNT_TYPE ${account_type}
ENV ACCOUNT_TOKEN ${account_token}
ENV ACCOUNT_ID ${account_id}
ENV LOGGER_API_KEY ${logger_api_key}
# Install deps
RUN apt-get update \
    && apt-get install -y sudo curl
# Copy source files
COPY . /usr/src/app
WORKDIR /usr/src/app
# Build
RUN mkdir cmake-build-release \
    && cd cmake-build-release \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && cd live-trading \
    && make
# Logging
RUN cd ~ && DD_AGENT_MAJOR_VERSION=7 DD_API_KEY=${LOGGER_API_KEY} DD_SITE="datadoghq.com" bash -c "$(curl -L https://s3.amazonaws.com/dd-agent/scripts/install_script.sh)" \
    && sed -i 's/# logs_enabled: false/logs_enabled: true/g' /etc/datadog-agent/datadog.yaml \
    && mkdir /etc/datadog-agent/conf.d/cpp.d \
    && chown dd-agent:dd-agent /etc/datadog-agent/conf.d/cpp.d \
    && mkdir /var/log/iridium \
    && chmod -R 777 /var/log/iridium \
    && mv /etc/datadog-agent/security-agent.yaml.example /etc/datadog-agent/security-agent.yaml \
    && chmod -R 777 /etc/datadog-agent/security-agent.yaml
COPY .logging/conf.yaml /etc/datadog-agent/conf.d/cpp.d/conf.yaml
RUN chown dd-agent:dd-agent /etc/datadog-agent/conf.d/cpp.d/conf.yaml
# Make bash script executable
RUN chmod +x run.sh
# Add admin user
RUN useradd -m iridium77 \
    && usermod -g sudo iridium77 \
    && echo "iridium77 ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
# Switch user
USER iridium77
# Entry point
CMD ./run.sh
