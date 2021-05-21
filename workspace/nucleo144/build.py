# Todo: Add git python to requirements.txt
from git import Repo
import socket
import os.path as path
import os
import subprocess
import argparse
import shutil
import hashlib
import time

# __file__ references the path to this specific build.py file
base_path = path.dirname(path.realpath(__file__))
print(base_path)

# Define other paths based off of previous base path
build_path = path.join(base_path, "cmake-build-ninja")
toolchain_path = path.join(base_path, "arm-none-eabi-gcc.cmake")
print(toolchain_path)


def build(clean=True):
    if clean:
        shutil.rmtree(build_path, ignore_errors=True)

    os.makedirs(build_path, exist_ok=True)

# Generate Ninja build files
    cmake_cmd = ["cmake",
                "-GNinja",
                "-DCMAKE_TOOLCHAIN_FILE={}".format(toolchain_path),
                "-DCMAKE_BUILD_TYPE=Debug",
                base_path]

    subprocess.check_call(cmake_cmd, cwd=build_path)

# Actually execute Ninja build files to generate binary
    build_cmd = ["cmake",
                 "--build",
                 "."]
    if clean:
        build_cmd.append("--clean-first")

    subprocess.check_call(build_cmd, cwd=build_path)

# Generate some metadata about the binary (hash, date, etc)
    md5_hash = hashlib.md5()
    bin_path = path.join(base_path, "output", "SpeakerRecognition.bin")
    info_path = path.join(base_path, "output", "info.txt")

    with open(bin_path, "rb") as f:
        content = f.read()
        md5_hash.update(content)
        digest = md5_hash.hexdigest()

    local_repo = Repo(search_parent_directories=True)
    with open(info_path, "w") as f:
        f.write("md5: {}\r\n".format(digest))
        f.write("Epoch time: {}\r\n".format(time.time()))
        f.write("Branch: {}\r\n".format(local_repo.active_branch.name))
        f.write("Commit hash: {}\r\n".format(local_repo.head.object.hexsha))
        f.write("System: {}\r\n".format(socket.gethostname()))



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "Build STM32 firmware")
    parser.add_argument("--clean", action="store_true", help="Delete previous build")
    args = parser.parse_args()

    build(args.clean)
