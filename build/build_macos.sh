mkdir -p build
cd build
OPENSSL_ROOT_DIR=/usr/local/opt/openssl cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib ..
make -j
