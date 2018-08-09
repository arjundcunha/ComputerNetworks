# Code for Go Back N Protocol Client
import socket
import hashlib
import time

# On receiving a chunk from the Server, break it up into data, seq_num and check_sum
def break_input(data):
	check = data[:32]			# Check_Sum
	ind = data.find(b'seq=')	# Seq_Nu,
	da = data[32:ind]			# Data
	return check, data[ind+4:-3],da


def receiveFile(Socket, addr):
	Socket.settimeout(None)
	seq_num = 0		# Expected Seq Num
	total_size = -1 	# File Size
	currSize = 0

	with open("Output.bin", "wb") as outFile:
		while True:
			data, lol = Socket.recvfrom(1100)			# Receive a Packet
			cs, rcv_seq, chunk = break_input(data)		# Get the informations
			#print("Received Seq : ", rcv_seq.decode("utf-8"))			
			dat = data[32:]	
			if  cs != hashlib.md5(dat).hexdigest().encode() or int(rcv_seq) != seq_num:		# Check If Corrupt or Wrong Seq Num
				# Send Ack for Last Seq_Num
				acknowledgment = hashlib.md5(str(seq_num-1).encode()).hexdigest() + str(seq_num-1)
			else:
				# Send Ack for recived current packet
				acknowledgment = hashlib.md5(str(seq_num).encode()).hexdigest() + str(seq_num)
				seq_num += 1 			# Increasent expected Seq
				if total_size == -1: 	# First Message is File Size
					total_size = int(chunk)
				else:
					outFile.write(chunk) 	#Write Data to file
					currSize += len(chunk)

				Socket.sendto(acknowledgment.encode(), addr)	#Send Ack
				if currSize >= total_size:
						exit(0)

port = int(input('Please Input the Receiver Port\n'))
sender_ip = socket.gethostbyname('127.0.0.1')		# This is Local Host
sender_port = int(input('Please Input the Sender Port\n'))

Socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)	# Create Socket
Socket.bind(('', port))				# Bind

receiveFile(Socket, (sender_ip, sender_port))