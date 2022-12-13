#!/usr/bin/env python3

import sys
import os
import re
import io
import subprocess
import pprint

debug = os.getenv("DEBUG") or False

os.system("make")

TRACES = [
    "binary-bal.rep",
    "binary2-bal.rep",
    "cccp-bal.rep",
    "coalescing-bal.rep",
    "cp-decl-bal.rep",
    "random-bal.rep",
    "random2-bal.rep",
    "realloc-bal.rep",
    "realloc2-bal.rep"
]

reCorrect = re.compile(r"correct:(\d+)")
rePerfidx = re.compile(r"perfidx:(\d+)")

#
# Traces-handout is used in the solution file, traces is in the student version.
#
TRACEDIR = 'traces-handout' if os.path.isdir('traces-handout') else 'traces'
if debug:
    print('traces', TRACEDIR)


def is_exe(fpath):
    return os.path.isfile(fpath) and os.access(fpath, os.X_OK)


MDRIVERS = ['./mdriver']
if len(sys.argv) > 1:
    MDRIVERS = [file for file in sys.argv[1:] if is_exe(file)]


def runCmd(cmd):
    proc = subprocess.Popen(cmd, bufsize=0, text=True, stdout=subprocess.PIPE)
    try:
        stdout, stderr = proc.communicate(timeout=60)
    except subprocess.TimeoutExpired:
        print("Timeout running command:", cmd)
        proc.kill()
        stdout = ""
        stderr = ""
    return stdout, stderr


def runTrace(program, tracefile):
    if debug:
        print("Run", program, "on", tracefile)
    if not is_exe(program):
        print(f"Your program file {program} does not exist or is not executable")
        return False, None

    test_out, test_err = runCmd([program,
                                 "-g",
                                 "-a",
                                 "-f", os.path.join(TRACEDIR, tracefile)])

    test = io.StringIO(test_out)

    perfidx = None
    correct = False
    for line in test:
        if debug:
            print(line.strip())
        cor = reCorrect.match(line)
        if cor:
            correct = cor.group(1)
        else:
            per = rePerfidx.match(line)
            if per:
                perfidx = per.group(1)
            else:
                print(line.strip())
    return correct, perfidx

def computeScore(x):
    if len(x) != 9:
        print("Bogus score vector")
    
    binary = (x[0] + x[1])/2
    prog = (x[2] + x[4])/2
    coal = x[3]
    rand = (x[5] + x[6])/2
    realloc = (x[7] + x[8])/2
    print(binary, prog, coal, rand, realloc)
    return min(110, (binary*4 + prog*0.25 + coal*0.25 + rand*3 + realloc*3)/9 + 15)

perfDict = {}
for mdriver in MDRIVERS:
    performanceList = []
    allCorrect = True
    for trace in TRACES:
        correct, perfidx = runTrace(mdriver, trace)
        allCorrect = allCorrect and correct
        performanceList.append(perfidx)
    if allCorrect:
        perfDict[mdriver] = [int(x) for x in performanceList]
    else:
        print("%-20s:" % (mdriver), "failed")

#print(perfDict)
for mdriver in perfDict.keys():
    trimmed = perfDict[mdriver]
    score = min(110, computeScore(trimmed))
    print(f"Grade {mdriver:<30s} {score:3.1f}")
