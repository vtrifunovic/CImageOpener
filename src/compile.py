# simple python script so I can quickly compile and test new features
import subprocess
import platform
import os

def main():
    if platform.system() != "Linux":
        print("lol")
        return
    compilation = "gcc main.c render/render.c masks/masks.c conversions/conversions.c binaryproc/binaryproc.c tools/basic_tools.c -lGL -lglfw -lm -Werror"
    ret_code = subprocess.call(compilation, shell = True)
    if ret_code != 0:
        print("Code could not compile... Terminating")
        return
    test_run = input("Run tests? (Y/N): ")
    if test_run.lower() != 'y':
        print("Done")
        return
    all_files = os.listdir()
    test_files = []
    for i in all_files:
        if ".png" in i or ".jpg" in i or ".JPG" in i:
            test_files.append(i)
    # Currently not needed
    #for image in test_files:
    #    subprocess.call(["./a.out", image])

if __name__ == "__main__":
    main()