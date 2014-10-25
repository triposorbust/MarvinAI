import MarvinAI
import math

def stupid_cast(x):
    if x == 0: return "0"
    else:      return str(int(math.log(x,2)))

amap = [[0,4,16,2],[2,0,4,16],[16,0,2,4],[32,16,8,4]]
for row in amap:
    print(",".join(map(stupid_cast, row)))

MarvinAI.initialize()
m = MarvinAI.evaluate(amap)

print("MOVE: {0}".format(m))
