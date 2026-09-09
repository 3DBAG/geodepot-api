download-data:
    wget https://data.3dgi.xyz/geodepot-test-data/data.zip
    unzip -o data.zip
    rm data.zip

test-integration:
    cmake -S . -B build/integration -DGD_BUILD_TESTING=ON -DGD_BUILD_APPS=ON
    cmake --build build/integration --target test-integration
