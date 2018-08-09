NOTE : ALL THE SUBMITTED CODE HAS BEEN TESTED AND WORKS ON MacOS El-Capiton

To run my code : 
	1) Create the server :  $g++ -std=c++11 server.cpp
							$./a.out

		The first output of this command is a PORT NUMBER, please copy this number as it will be used.

	2) Once the server is created, for Question1
				$g++ -std=c++11 server.cpp
				$./a.out <Address> <Port Num(From server)> <Number of Packets> <Packet-Size> <interval>

	3) For Question2, continuing on the same server
				$g++ -std=c++11 server.cpp
				$./a.out <Address> <Port Num>
			The required outputs are in the created files.

	4) You may run the python code to create your own graphs (May vary as systems change)
				$python plot.py