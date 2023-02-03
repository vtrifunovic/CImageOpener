# simple python script so I can quickly compile and test new features
import subprocess
import platform
import os
import time

class colors:
    ULINE = "\033[4m"
    RESET = "\033[0m"

def main():
    if platform.system() != "Linux":
        print("lol")
        return
    comp_file = "solvemazes.c"
    compilation = "gcc " +comp_file+" global.c render/render.c masks/masks.c conversions/conversions.c binaryproc/binaryproc.c tools/basic_tools.c opencl_support/gpu_setup.c -lGL -lglfw -lm -Werror -lOpenCL"
    ret_code = subprocess.call(compilation, shell = True)
    if ret_code != 0:
        print("Code could not compile... Terminating")
        return
    test_run = input("Run tests? (Y/N): ")
    if test_run.lower() != 'y':
        print("Done")
        return
    test_run = input("Speed or view? (S/V): ")
    if test_run.lower() == "s":
        comp_file = "speedtest.c"
    compilation = "gcc -o speedtest " +comp_file+" global.c render/render.c masks/masks.c conversions/conversions.c binaryproc/binaryproc.c tools/basic_tools.c opencl_support/gpu_setup.c -lGL -lglfw -lm -Werror -lOpenCL"
    ret_code = subprocess.call(compilation, shell = True)
    all_files = os.listdir("./mazes")
    test_files = []
    for i in all_files:
        if ".png" in i or ".jpg" in i.lower()  or ".jpeg" in i:
            test_files.append("./mazes/" + i)
    test_files.sort()
    for image in test_files:
        if test_run.lower() == "s":
            start = time.time()
        subprocess.call(["./speedtest", image])
        if test_run.lower() == "s":
            print(f"Elapsed time: {colors.ULINE}{time.time() - start:.4f}{colors.RESET} sec\n")
        if test_run.lower() == "s":
            start = time.time()
            subprocess.call(["./speedtest", image, "nogpu"])
            print(f"Elapsed time: {colors.ULINE}{time.time() - start:.4f}{colors.RESET} sec\n")
    keep = input("Keep speedtest file? (Y/N): ")
    if keep.lower() != "y":
        os.remove("speedtest")
        print("Removed.")

if __name__ == "__main__":
    main()