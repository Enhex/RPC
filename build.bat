set source="%~dp0\"

set dir_name=build

cd ..
mkdir %dir_name%
cd %dir_name%

REM conan install %source% --build=outdated
conan install %source% --build=outdated -s build_type=Debug -s arch=x86

cd %source%
premake5 vs2019 --location=../build/

REM generate TLS private key and certificate for the test
cd ../%dir_name%
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -batch