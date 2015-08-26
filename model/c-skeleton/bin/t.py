fr = open('result')

for line in fr.readlines():
	lines = line[:-1]
	data =lines.split()
	slen = data[0].split('|')[1]
	if int(slen) > 8000:
		print lines

fr.close()
