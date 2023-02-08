# simple python script so I can quickly compile and test new features
import subprocess
import platform
import os
import time
import argparse

compiler = "gcc "
includes = " global.c render/render.c render/render_internal/render_internal.c masks/masks.c conversions/conversions.c binaryproc/binaryproc.c tools/basic_tools.c opencl_support/gpu_setup.c -lGL -lglfw -lm -Werror -lOpenCL -lGLEW "

class colors:
    ULINE = "\033[4m"
    RESET = "\033[0m"

def main(extra, comp_file):
    if platform.system() != "Linux":
        print("lol")
        return
    compilation = compiler + comp_file + includes + extra
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
    compilation = compiler + "-o speedtest " + comp_file + includes + extra
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
    parser = argparse.ArgumentParser()
    parser.add_argument('--o', dest="extr", required=False)
    parser.add_argument('--f', dest="file", required=True)
    args = parser.parse_args()
    extra = ""
    file = ""
    if args.extr:
        extra = "-" +args.extr
    main(extra, args.file)
