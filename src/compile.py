# simple python script so I can quickly compile and test new features
import subprocess
import platform
import os
import time
import argparse
import re

compiler = "gcc "
link_files = " -lGL -lglfw -lm -Werror -lOpenCL -lGLEW"

def extract_headers(cfile):
    possible_files = []
    for text in cfile.readlines():
        if re.search("#include", text):
            if not re.search("<", text):
                possible_files.append(text)
    return possible_files

def clean_headers(fil):
    fil = re.sub("#include ", "", fil)
    fil = re.sub("\n", "", fil)
    fil = re.sub(".h", ".c", fil)
    fil = re.sub('"', "", fil)
    return fil

def extract_project_files(comp_file):
    f = open(comp_file, "r")
    possible_files = extract_headers(f)
    f.close()
    valid_files = []
    for fil in possible_files:
        fil = clean_headers(fil)
        if os.path.isfile(fil):
            valid_files.append(fil)
    for x in valid_files:
        g = open(x, "r")
        mpath = x.split("/")[0]
        ph = extract_headers(g)
        for p in ph:
            p = clean_headers(p)
            q = re.sub(r"^../", "", p)
            p = mpath + "/" + p
            if p not in valid_files and os.path.isfile(p) and "/../" not in p:
                valid_files.append(p)
            if q not in valid_files and os.path.isfile(q):
                valid_files.append(q)
        g.close()
    # DO NOT FUCKING MOVE AGAIN
    return " ".join(valid_files)


class colors:
    ULINE = "\033[4m"
    RESET = "\033[0m"

def main(extra, comp_file, folder):
    if platform.system() != "Linux":
        print("lol")
        return
    valid_files = extract_project_files(comp_file)
    compilation = compiler + comp_file +" "+ valid_files + link_files + extra
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
    compilation = compiler + "-o speedtest " + comp_file +" "+ valid_files + link_files + extra
    ret_code = subprocess.call(compilation, shell = True)
    if ret_code != 0:
        print("Could not compile... Terminating")
        return
    all_files = os.listdir(folder)
    test_files = []
    for i in all_files:
        if ".png" in i or ".jpg" in i.lower()  or ".jpeg" in i:
            test_files.append(folder + i)
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
    parser.add_argument('--file', dest="file", required=True)
    parser.add_argument('--folder', dest="folder", required=False)
    args = parser.parse_args()
    extra = ""
    file = ""
    if args.extr:
        extra = "-" +args.extr
    if not args.folder:
        args.folder = "./"
    main(extra, args.file, args.folder)
