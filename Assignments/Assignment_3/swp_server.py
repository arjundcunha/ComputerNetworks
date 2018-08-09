# Code for Stop and Wait Protocol Server
import socket
import datetime
import signal
import os
import hashlib

# Chunks of the File
file_chunks = []

# Function to break up the file in chucks of size Block
def break_File(filename, block):
	file_chunks.append(str(os.stat(filename).st_size).encode())
	with open(filename, 'rb') as fil:
		while 1:
			chunky = fil.read(block)
			if not chunky:
				break
			file_chunks.append(chunky)

# handler for timeout
def handler(signum, frame):
	raise Exception("Timeout Encountered")


def sendFile(Socket, rec_ip, rec_port, rtt=1, alpha=0.125):
	#Boiler Code
	Socket.settimeout(None)
	signal.signal(signal.SIGALRM, handler)
	seq_num = 0
	tot_size = -1
	send_size = 0
	
	# For each chunk to be sent
	for chunk in file_chunks:

		# Create Packet
		data = str(seq_num).encode() + chunk
		packet = hashlib.md5(data).hexdigest().encode() + data

		# First send the File Size
		if tot_size == -1:
			tot_size = int(chunk)
		else:
			send_size += len(chunk)
		
		#Timestamp of sent packet
		sendTime = datetime.datetime.now()
		Socket.sendto(packet, (rec_ip, rec_port))
		# To check for Timeout
		signal.setitimer(signal.ITIMER_REAL, 0.2 + 3 * rtt)
		
		while True:
			try:
				recv, addr = Socket.recvfrom(33)	# Recive Ack
				last_rtt = datetime.datetime.now() - sendTime	# Get Timestamp
				rtt = alpha * last_rtt.total_seconds() + (1 - alpha) * rtt # Calc new rtt
				
				seq, cs = str(chr(recv[0])), recv[1:]		# Break to get seq_num and checksum

				# If corrupt or wrong seq-Num
				if int(seq) != seq_num or cs != hashlib.md5(seq.encode()).hexdigest().encode():
					continue
				else:
					# Correct Ack
					sendTime = datetime.datetime.now()
					signal.setitimer(signal.ITIMER_REAL, 0)
					seq_num = 1 - seq_num
					break

				#Timeout
			except Exception as e:
				Socket.sendto(packet, (rec_ip, rec_port))
				signal.setitimer(signal.ITIMER_REAL, 0.2 + 3 * rtt)
				continue


port = int(input('Please Input the Sender Port\n'))
rec_ip = socket.gethostbyname('127.0.0.1')
rec_port = int(input('Please Input the Receiver Port\n'))

Socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
Socket.bind(('', port))
break_File(input('Enter File name\n'), 1000)

sendFile(Socket, rec_ip, rec_port)