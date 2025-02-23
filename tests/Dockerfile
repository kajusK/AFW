FROM throwtheswitch/madsciencelab:1.0.1

USER root
RUN apt-get update && \
    apt-get install -y gcovr && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*
USER dev
