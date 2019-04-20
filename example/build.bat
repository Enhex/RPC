set source="%~dp0\"

set dir_name=example-build

cd ../..
mkdir %dir_name%
cd %dir_name%

REM conan install %source% --build=outdated
REM conan install %source% --build=outdated -s build_type=Debug

cd %source%
premake5 vs2019 --location=../../example-build/