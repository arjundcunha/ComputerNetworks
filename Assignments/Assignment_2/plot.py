import matplotlib.pyplot as plt
import sys

if len(sys.argv) != 2:
    print "Usage: python3 plot.py <Number of Connections>"
    exit(0)  

num = int(sys.argv[1])

for x in xrange(0,num):
	st = "Throughtput_" + str(x) +".txt"
	inpu = [int(line) for line in open(st, "r")]
	bytes = []
	for a in range(1,len(inpu)):
		bytes.append(inpu[a]- inpu[a-1])
	#print bytes
	p = [i+1 for i in range(len(bytes))]

	plt.plot(p, bytes)
	plt.title("Throughtput for connection "+ str(x+1))
	plt.xlabel("Time(Seconds)")
	plt.ylabel("Bytes")
	plt.show()