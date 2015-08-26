char_num = {}
num = 0
fr = open("char_set", 'r')
for line in fr.readlines():
	data = line[:-1].split('|')
	
	for ky in data:
		vl = ky.split(':')
		
		if len(ky) == 0:
			#print vl
			#print ky
			#print num
			#num += 1
			continue 
		
		key = vl[0]
		value = vl[1]
			
		if key in char_num:
			char_num[key] += value
		else:
			char_num[key] = value
	num += 1
	
#for ky in char_num.keys():
#	print 'char :', ky , 'count :', char_num[ky]

for w in sorted(char_num, key=char_num.get, reverse=True):
	print w, "  : ", char_num[w]

fr.close()	
