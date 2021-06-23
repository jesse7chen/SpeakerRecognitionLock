import os.path as path
import os
import subprocess
import filecmp
import time

# This may need to be changed depending on actual location of STM32CubeProgrammer
stm32cubeprogrammer_path = "/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLI"

# __file__ references the path to this specific flash.py file
base_path = path.dirname(path.realpath(__file__))

bin_path = path.join(base_path, "output", "SpeakerRecognition.bin")

# Connect to device and erase - default to using SWD with freq of 4000 kHz and AP index of 0
erase_cmd = [stm32cubeprogrammer_path,
               "--connect",
               "port=SWD",
               "--erase",
               "all"]

subprocess.check_call(erase_cmd, cwd=base_path)

time.sleep(0.5)

# Download flash
download_cmd = [stm32cubeprogrammer_path,
               "--connect",
               "port=SWD",
               "--write",
               bin_path,
               "0x08000000"]

subprocess.check_call(download_cmd, cwd=base_path)

time.sleep(0.5)

# Read and verify flash was downloaded correctly

# Get size of binary in bytes
bin_size = path.getsize(bin_path)
output_file = path.join(base_path, "validation.bin")

read_cmd = [stm32cubeprogrammer_path,
               "--connect",
               "port=SWD",
               "--read",
               "0x08000000",
               str(bin_size),
               output_file]

subprocess.check_call(read_cmd, cwd=base_path)

if (filecmp.cmp(output_file, bin_path, shallow=False) == False):
    raise OSError("Verification of binary failed!")

time.sleep(0.5)

# Start program
start_cmd = [stm32cubeprogrammer_path,
               "--connect",
               "port=SWD",
               "--start",
               "0x08000000"]

subprocess.check_call(start_cmd, cwd=base_path)
