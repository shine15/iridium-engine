#!/bin/bash
#sudo service datadog-agent restart
/usr/src/app/cmake-build-release/live-trading/live-trading \
    -e ${ACCOUNT_TYPE} \
    -t ${ACCOUNT_TOKEN} \
    -a ${ACCOUNT_ID}