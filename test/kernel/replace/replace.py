#! /bin/python
import sys 

print "read file"

fr = open(sys.argv[1], "r")
content = fr.read();

for i in range(len(content) - 21):
	if content[i] == '@' and content[i + 21] == '@' and content[i+1] == '@':
	#if ord(content[i]) == 0x00 and ord(content[i + 21]) == 0x00:
		print i, content[i], i + 21, content[i+21]


