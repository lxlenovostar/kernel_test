"""
count the number of each length.
"""
c_len = {}
fr = open("data", "r")

for l_data in fr.readlines():
    data = l_data[:-1]
    
    if data in c_len:
        c_len[data] += 1
    else:
        c_len[data] = 1

fr.close()
fw = open("count", "w")

for k_data in sorted(c_len, key=c_len.get, reverse=True):
    fw.write(k_data + "  : " + str(c_len[k_data]) + "\n")

fw.close()
