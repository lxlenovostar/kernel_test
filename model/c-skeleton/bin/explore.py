"""
Analysis data.
"""
import random  

result = []
def stdDeviation(a):
	l = len(a)
	m = sum(a) / l
	d = 0
	for i in a: 
		d += (i-m) ** 2
	return (d/(l - 1)) ** 0.5

def getsd():
	data = []
	for dummy_i in result:
		data.append(float(dummy_i))
	print stdDeviation(data)

def main():
	"""
	read the data file.
	"""
	remain = 0
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

def randomdata():
	"""
	generate the random dataset.
	"""
	res = []
	len_set = 792966
	for dummy_i in range(len_set):
		tmp = random.randint(570, 2000)
		res.append(float(tmp))

	fw = open('random', 'w')
	for dummy_i in res:
		fw.write(str(dummy_i) + '\n')
	fw.close()

	print stdDeviation(res)


	
main()
getsd()				
#randomdata()




		
