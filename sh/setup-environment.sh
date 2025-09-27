#!/bin/sh

if ! command -v apt-get >/dev/null 2>&1; then
    echo "apt-get not found, you must deploy this on an Ubuntu/Debian system"
    exit 1
fi

# install ssock if unavailable
[ ! -f "/usr/local/include/ssock.hpp" ] && [ ! -f "/usr/include/ssock.hpp" ] {
    git clone --recursive https://github.com/jacnils/ssock
    cd ssock
    mkdir -p build; cd build
    cmake ..
    cmake --build .
    cmake --install .
    cd ../../
    [ -d "ssock/" ] && rm -rf ssock/
}

apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    git \
    libboost-all-dev \
    libssl-dev \
    libyaml-cpp-dev \
    libpq-dev \
    libsqlite3-dev \
    nlohmann-json3-dev \
    libxml2-dev \
    libc6-dev \
    nodejs \
    postgresql \
    sqlite3 \
    ffmpeg \
    imagemagick \
    libmagick++-dev \
    libavcodec-dev \
    libavformat-dev \
    libavdevice-dev \
    libpostproc-dev \
    libavutil-dev \
    libswscale-dev \
    npm || exit 1

npm install -g uglify-js

groupadd -r ff-api && useradd -r -g ff-api ff-api
mkdir -p /etc/ff /var/log/ff /var/db/ff /var/lib/ff

rm -rf ff-api
git clone --recursive https://github.com/ForwarderFactory/ff-api; cd ff-api || exit 1
mkdir -p build && cd build || exit 1
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make && make install || exit 1
cd ..; rm -rf build

[ ! -f "/etc/ff/config.yaml" ] && ff-api -gc > /etc/ff/config.yaml

chown -R ff-api:ff-api /etc/ff /var/log/ff /var/db/ff /var/lib/ff
chmod -R 755 /etc/ff /var/log/ff /var/db/ff /var/lib/ff

cat > /etc/systemd/system/ff-api.service <<EOF
[Unit]
Description=ff-api
After=network.target

[Service]
Type=simple
User=ff-api
Group=ff-api
ExecStart=ff-api
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable ff-api
systemctl restart ff-api
