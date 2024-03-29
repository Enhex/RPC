import os
from sys import platform
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-r", "--release", help="Generate release build (debug by default).", action="store_true")
args = parser.parse_args()

build_type = 'Release' if args.release else 'Debug'

source = os.path.dirname(os.path.realpath(__file__))

def create_symlink(src, dst):
    # create all parent directories of the symlink one
    parent_dir = os.path.dirname(dst)
    os.makedirs(parent_dir, exist_ok=True)

    try:
        os.symlink(src, dst)
    except:
        pass

def build(source, build_type, symlinks = [], symlink_pairs = []):
    build_dir = '../build/' + build_type + '/'

    # create build directory
    os.makedirs(build_dir, exist_ok=True)
    os.chdir(build_dir)

    # symlinks
    for path in symlinks:
        create_symlink(source + '/' + path, './' + path)

    for src_path, dst_path in symlink_pairs:
        create_symlink(source + '/' + src_path, './' + dst_path)

    # conan
    os.system('conan install "' + source + '/" --build=outdated -s arch=x86_64 -s build_type=' + build_type) # -o RPC:system_openssl=True

    # choose premake generator based on OS
    os.chdir(source)

    def premake_generate(generator):
        os.system('premake5 ' + generator + ' --location="' + build_dir + '"')

    if platform == 'win32':
        premake_generate('vs2019')
    else:
        premake_generate('gmake2')
        premake_generate('vscode')

    # generate TLS private key and certificate for the test
    os.chdir(build_dir)
    os.system('openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -batch')


build(source, build_type)