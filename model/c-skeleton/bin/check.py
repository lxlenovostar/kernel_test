import time
ISOTIMEFORMA='%Y-%m-%d %X'
fr = open("sort", "r")
fw = open("moresave", "w")
num = 0 
flag = 0
limit = 24
#max_count = 40 
max_count = 14 
max_len = 218
result = {}
#begin_pos = 14 
begin_pos = 13

data_list = []
max_data = 40
index_i = 0
end_flag = 0
for line in fr.readlines():
    if index_i < max_data:
        data_list.append(line[:-1].split()[0])

    index_i += 1

#print len(data_list)
#print data_list
fr.close()

data_len = 0 
data_sha = ""
data_value = ""
line_value = ""
look_value = ""

for line_data in data_list:
    for look_data in data_list:
        if look_data != line_data:
            f_key = open("char_get", "r")
            end_flag = 0
            for line_key in f_key.readlines():
                data = line_key[:-1]
 
                if flag == 0:
                    data_len = int(data.split("|")[1])
                elif flag == 1:
                    data_sha = data
                elif flag == 2:
                    data_value = data
                else:
                    print 'error case'
                    exit(-1)
                
                if flag == 2:
                    flag = 0

                    if data_sha == line_data:
                        line_value = data_value
                        if end_flag == 0: 
                            end_flag += 1
                            continue
                        else:
                            break
                    
                    if data_sha == look_data:
                        look_value = data_value 
                        if end_flag == 0: 
                            end_flag += 1
                            continue
                        else:
                            break;
                else:
                    flag += 1
                
            f_key.seek(0)

            if look_value.find(line_value) != -1:
                print "line:", line_data, line_value 
                print "look:", look_data, look_value 
                print ""
f_key.close()
exit(0)
