import sys
save_num = 0	#缓存的数据
all_num = 0		#所有的数据

sha_ref = {} 	#此数据的出现的次数
sha_value = {} 	#此数据的内容
sha_tax = {}  	#此数据的贡献值

fr = open("test", 'r')
for line in fr.readlines():
	data = line[:-1].split()
	
	if len(data) == 3:
		value = data[0]
		sha = data[1]
		value_len = data[2]
	
		all_num += int(value_len)
	
		if sha in sha_value:
			#验证数据是否正确
			if sha_value[sha] != value:
				print "value is err"
				print "\nsha is:", sha, "value is:", sha_value[sha]
				print "\nsha is:", sha, "value is:", value
				print len(sha_value[sha])/3, len(value)/3, value_len
					
				sys.exit(-1)
		else:
			sha_value[sha] = value
		
		if sha in sha_ref:
			sha_ref[sha] += 1
		else:
			sha_ref[sha] = 0
		
		if sha in sha_tax:
			sha_tax[sha] += int(value_len)
			save_num += int(value_len)
		else:
			sha_tax[sha] = 0

	else:
		print data
		print "BUG"
		sys.exit(-1) 

print "save_num is:", save_num
print "all_num is:", all_num
	
head = 5
for w in sorted(sha_tax, key=sha_tax.get, reverse=True):
	if head == 0:
		break
	head -= 1
	print w, "  : ", sha_tax[w]
	

fr.close()	
