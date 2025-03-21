FROM ubuntu:20.04

# Set non-interactive mode for apt-get
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Seoul

# Install necessary C++ build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    pkg-config \
    librdkafka-dev \
    librdkafka1 \
    libsasl2-dev \
    uuid-dev \
    libssl-dev \
    zlib1g-dev \
    cmake \
    curl \
    git \
    gdal-bin \
    libgdal-dev \
    libglib2.0-dev \
    && rm -rf /var/lib/apt/lists/*  # Reduce image size

# Set environment variables
ENV PATH="/usr/local/bin:$PATH"
# Copy project files
COPY . /app
COPY ./ld.so.conf /etc

# Run ldconfig to update shared libraries
RUN /usr/sbin/ldconfig
 
# Check if Makefile exists before running make
WORKDIR /app
#ENTRYPOINT ["/bin/sh", "-c", "make"]
#ENTRYPOINT ["tail", "-f", "/dev/null"]

RUN make
RUN ./ConvertProcessMsg dev-kafka.default.svc.cluster.local:9092 convertProcess
#RUN tail -f /dev/null
