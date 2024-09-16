server := "3dgi-server"

download-data:
    wget https://data.3dgi.xyz/geodepot-test-data/data.zip
    unzip -o data.zip
    rm data.zip
