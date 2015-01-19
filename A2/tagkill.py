#!/usr/bin/python
""" simple utility to kill a process 
	based on a tag associated with it
"""

import os
import sys
import signal

tagToFind = sys.argv[1]
lines = open("/proc/ptags", 'r').readlines()
killed = False
for line in lines:
	pid = line.split(" : ")[0]
	afterpid = line.split(" : ")[1]
	tag = afterpid.rsplit(' |',1)[0]
	if (tag == tagToFind):
		print("killing pid: " + pid)
		os.kill(int(pid), signal.SIGKILL)
		killed = True

if not killed:
	print("No processes have tag: " + tagToFind)