import os
import csv


global_max = {}
global_delta = {}
local_max =  {}
local_delta =  {}

prefix_name = ["local_delta", "local_peak", "current", "peak"]
memory_type = ["HEAP", "CHUNK", "AL", "LL", "CL"]

for i in memory_type:
	global_max[i]=0;
	global_delta[i]=0;
	local_max[i]=0;
	local_delta[i]=0;

result_columns = ['code_name']
result_columns.extend([x + '_' + y for y in memory_type for x in prefix_name])

result = []

f = open('data.txt', 'r')
lines = f.readlines()
f.close()
for line_index in xrange(len(lines)):
    line = lines[line_index].strip()
    if line[:4] == 'CODE':
        codename = line[5:]
        for i in memory_type:
            local_delta[i]=0;
            local_max[i]=0;
        continue
    else:
        split_line = line.split()
        local_delta[split_line[0]] += int(split_line[1])
        local_max[split_line[0]] = max(local_max[split_line[0]], local_delta[split_line[0]])
        global_delta[split_line[0]] += int(split_line[1])
        global_max[split_line[0]] = max(global_max[split_line[0]], global_delta[split_line[0]])

        if line_index + 1 == len(lines) or lines[line_index + 1][:4] == 'CODE':
            prefix_dict_array = [local_delta, local_max, global_delta, global_max]
            tmp = [x[y] for y in memory_type for x in prefix_dict_array]
            local_result = [codename]
            local_result.extend(tmp)
            result.append(local_result)

print result

outfile = open('result.csv', 'wb')
writer = csv.writer(outfile)
writer = csv.writer(outfile)
writer.writerow(result_columns)
for row in result:
    writer.writerow(row)
outfile.close()







