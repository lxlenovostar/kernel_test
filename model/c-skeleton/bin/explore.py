"""
"""

def main():
	"""
	read the data file.
	"""
	remain = 0
	result = []
	f = open('result', 'r')
	for line in f.readlines():
		data = line[:-1]
		split_data = data.split('|')
		
		if len(split_data) < 2:
			print "error data"
			print line 
			print split_data
			exit(-1)
			
		index = split_data[0]
		data_len = split_data[1]
		point = split_data[2].split()
 	
		if len(point) != 0:
			for i in range(len(point)):
				if i == 0:
					dis = remain + int(point[0])
					result.append(dis)
					pre = point[0]
		
					if len(point) == 1:
						remain = int(data_len) - int(point[i])			
				elif i == len(point) - 1:
					dis = int(point[i]) - int(pre)
					if dis == 0:
						print 'error: ' + str(i) + ' ' + pre
					result.append(dis)
					remain = int(data_len) - int(point[i])			
				else:
					dis = int(point[i]) - int(pre)
					if dis == 0:
						print 'error: ' + str(i) + ' ' + pre 
					result.append(dis)
					pre = point[i]					
		else:
			remain += int(data_len)

	fw = open('data', 'w')
	for i in result:
		fw.write(str(i) +' \n')
	fw.close()
	f.close()

main()						
