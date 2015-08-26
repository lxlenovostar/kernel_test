char_num = {}
save_bytes = 0
fr = open("source", 'r')
for line in fr.readlines():
	data = line[:-1].split('|')

	if len(data) > 1  and data[0] != 'sha':
		#print line[:-1], data, len(data)
		save_bytes = int(data[1])
		continue
		
	if data[0] != 'sha' or len(data) <= 0:
		continue

	key = data[1]
	if key in char_num:
		char_num[key] += save_bytes
	else:
		char_num[key] = save_bytes
	
#for ky in char_num.keys():
#	print 'char :', ky , 'count :', char_num[ky]

for w in sorted(char_num, key=char_num.get, reverse=True):
	print w, "  : ", float(char_num[w])

fr.close()	
