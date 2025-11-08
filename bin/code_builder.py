code=""

f = open('raw_walls_1.txt','r')
lines = f.readlines()
f.close()
line=0

for i in range(16):
    for j in range(16):
        code_now = f"\nmaze->cells[{j}][{i}].wallLeft = {lines[line][0]};\nmaze->cells[{j}][{i}].wallBack = {lines[line][1]};\nmaze->cells[{j}][{i}].wallRight = {lines[line][2]};\nmaze->cells[{j}][{i}].wallFront = {lines[line][3]};\n"       
        code+=code_now
        line+=1

f = open('code.txt','w')
print(code,file=f)
f.close()
