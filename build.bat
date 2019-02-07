set source="%~dp0\"

set dir_name=build

cd ..
mkdir %dir_name%
cd %dir_name%

REM conan install %source% --build=outdated
conan install %source% --build=outdated -s build_type=Debug -s arch=x86

cd %source%
premake5 vs2017 --location=../build/