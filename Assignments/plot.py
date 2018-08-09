import matplotlib.pyplot as plt

avgTime = [float(line) for line in open("avg_time_output.txt", "r")]
x = [i+1 for i in range(len(avgTime))]
plt.plot(x, avgTime)
plt.title("Average RTT vs Time")
plt.xlabel("Time(milliseconds)")
plt.ylabel("Average RTT(microseconds)")
plt.show()

throughput = [float(line) for line in open("tp_output.txt", "r")]
x = [i+1 for i in range(len(throughput))]
plt.plot(x, throughput)
plt.title("Throughtput vs Time")
plt.xlabel("Time(milliseconds)")
plt.ylabel("Throughtput")
plt.show()
