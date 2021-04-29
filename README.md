# iridium-engine
Iridium C++ engine

Windows: Visual Studio 2019

Install Boost 1.75.0

OPENSSL
Most Unix systems already have OpenSSL preinstalled. If your system does not have OpenSSL, please get it from the OpenSSL website or another source. You do not have to build OpenSSL yourself — a binary distribution is fine.

On Debian'ish Linux systems, you can install OpenSSL with:

$ apt-get install libssl-dev
Starting with El Capitan (10.11), macOS no longer includes OpenSSL. The recommended way to install OpenSSL on Mac OS X is via Homebrew:

$ brew install openssl
The easiest way to install OpenSSL on Windows is to use a binary (prebuild) release, for example the installer from Shining Light Productions.
Depending on where you have installed the OpenSSL libraries, you might have to edit the build script (buildwin.cmd), or add the necessary paths to the INCLUDE and LIB environment variables, or edit the Visual C++ projects if the installed OpenSSL libraries have different names than specified in the project file.

Install poco

cd Poco
mkdir cmake-build
cmake .. -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
cmake --build . --config Release
sudo cmake --build . --target install

Docker
docker build \
 -t live-trading \
 --build-arg account_type=practice \
 --build-arg account_token=f78990252f4a41e03fcbcb5f6cd80da5-06c80ae3156be464806b5a18e6077359 \
 --build-arg account_id=101-011-5886393-001 \
 --no-cache .
 
 Heroku Deployment
 set environment variables
 HEROKU_API_KEY=${HEROKU_API_KEY} heroku container:login
 docker tag live-trading registry.heroku.com/iridium77/worker
 docker push registry.heroku.com/iridium77/worker
 heroku container:release -a iridium77 worker