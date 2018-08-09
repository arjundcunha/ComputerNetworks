# Code for Go Back N Protocol Server
import socket
import datetime
import signal
import os
import hashlib
import time

# Broken up file
file_chunks = []

# Function inputs file name and breaks it up into required chunks of size block
def break_File(filename, block):

	file_chunks.append(str(os.stat(filename).st_size).encode()) 	#First is File Size
	with open(filename, 'rb') as fil:
		while 1:
			chunky = fil.read(block)	
			if not chunky:
				break
			file_chunks.append(chunky)

# In order to Handle TimeOut Event
def handler(signum, frame):
	raise Exception("Timeout Happened")


def sendFile(Socket, addr, rtt=1, alpha=0.125):
	# Packets ready to be sent : Already created as per requirement
	packets = []
	for x in range(0, len(file_chunks)):
		seq_part = "seq="+str(x)+"end"	# Type of Header File
		data = file_chunks[x]+seq_part.encode()	
		info = hashlib.md5(data).hexdigest().encode() + data
		packets.append(info)
	

	# Basic Boiler Code
	Socket.settimeout(None)
	signal.signal(signal.SIGALRM, handler)
	seq_num = 0
	tot_size = -1
	window = []	# Current window of data
	base = 0
	sendTime = {}
	send = 0

	while True:
		while (len(window) < window_size and send<len(packets)):	#Space left in current window
			window.append(packets[send])
			send += 1
		i = 0
		# Send each packet in the window
		for pac in window:
			Socket.sendto(pac, addr)
			sendTime[base + i] = datetime.datetime.now()	# Sent time for each packet for rtt
			i+=1
		# To assist in Timeout
		signal.setitimer(signal.ITIMER_REAL, 0.3 + 3 * rtt)

		try:
			while True:
				recv, lol = Socket.recvfrom(50)		# Recived Ack
				recvTime = datetime.datetime.now()	# Get TimeStamp

				cs, seq = recv[:32], str(recv[32:].decode("utf-8"))	# Break-up into Seq-Num and Checksum

				if cs != hashlib.md5(seq.encode()).hexdigest().encode():	# If Coruppted
					continue
				rtt = alpha * (recvTime - sendTime[int(seq)]).total_seconds() + (1 - alpha) * rtt #New rtt
				# Remove packets in window for which ack is recieved
				while base < int(seq) + 1:
					base += 1
					del window[0]
				# Stop timer
				if base == seq_num:
					signal.setitimer(signal.ITIMER_REAL, 0)
					break
				else:
					signal.setitimer(signal.ITIMER_REAL, 0.3 + 3 * rtt)

				# Timeout Happened
		except Exception as e:
			if str(e) == "Timeout Happened":
				i = 0
				for packet in window:
					Socket.sendto(packet, addr)
					sendTime[base + i] = datetime.datetime.now()
					i += 1
			else:
				print (e)


port = int(input('Please Input the Sender Port\n'))

rec_ip = socket.gethostbyname('127.0.0.1')
rec_port = int(input('Please Input the Receiver Port\n'))

Socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
window_size = int(input('Please Input the Window Size'))
Socket.bind(('', port))
break_File(input('Enter File name\n'), 1000)
addr = (rec_ip, rec_port)
sendFile(Socket, addr)