# Code for Stop and Wait Protocol Client
import socket
import hashlib

def receiveFile(Socket, sender_ip, sender_port):
	# Boiler Code
	Socket.settimeout(None)
	seq_num = 0 	#Expected Seq Num
	total_size = -1
	currSize = 0
	with open("Output.bin", "wb") as outFile:
		while True:
			data, addr = Socket.recvfrom(1033)	# Recive Packet
			cs, rcv_seq, chunk = data[:32], int(chr(data[32])), data[33:]	# Break up into components
			dat = data[32:]

			if  cs != hashlib.md5(dat).hexdigest().encode() or int(rcv_seq) != seq_num:	# If Corrupt or Wrong Seq-Num
				acknowledgment = str(1-seq_num) + hashlib.md5(str(1-seq_num).encode()).hexdigest()
			else:
				# Correct Order
				acknowledgment = str(seq_num) + hashlib.md5(str(seq_num).encode()).hexdigest()
				# NEw Expected Seq_Num
				seq_num = 1 - seq_num
				if total_size == -1:	# First packet has file size
					total_size = int(chunk)
					print (total_size)
				else:
					outFile.write(chunk)	# Write to file
					currSize += len(chunk)
					if currSize >= total_size:
						exit(0)
			Socket.sendto(acknowledgment.encode(), (sender_ip, sender_port))	# Send Ack

port = int(input('Please Input the Receiver Port\n'))
sender_ip = socket.gethostbyname('127.0.0.1')		# This is Local Host
sender_port = int(input('Please Input the Sender Port\n'))

Socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
Socket.bind(('', port))

receiveFile(Socket, sender_ip, sender_port)