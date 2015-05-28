#!/usr/bin/python

import commands
dict = {}

fw = file('result', 'w')
ferror = file('error', 'w')
f = file('count')
while True:
		line = f.readline()
		line = line.strip('\n')

		if len(line) == 0: 
			break

		if line not in dict:
			dict[line] = 1
		else:
			dict[line] += 1

f.close() # close the file

temp = 0
for key in dict.keys():
	temp += dict[key]
	fw.write(key + ' ' + str(dict[key]) + '\n')
fw.close()

filename = '/root/kernel_test/model/c-skeleton/bin/a'
for key in dict.keys():
	result, output = commands.getstatusoutput('grep -nre ' + key + ' ' + filename + ' | wc | awk \'{print $1}\'')
	exit_code = result >> 8
	if exit_code != 0:
		exit(1)
	if dict[key] + 1 != int(output):	
		ferror.write(key + ' ' + str(dict[key]) + ' ' + output + '\n')
ferror.close()
		

