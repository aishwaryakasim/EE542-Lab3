# EE542-Lab3
Fast, Reliable File Transfer using UDP
+------------------------+
| BUILD & RUN            |
+------------------------+

To compile the code: 
1. For client: make client
2. For server: make server

Additional instructions:
To clean the files: make clean

+-------------------------+
| EXECUTION               |
+-------------------------+
1. To run the server side of the code: ./server <portnumber>        #Example ./server 20001
2. To run the client size of the code: ./client <server_ip_address> <portnumber>   #Example ./client 192.168.1.42 20001
3. To request a file from server enter the file name after executing the client side of the code: <filename>        #Example  data.bin

To add Loss in the link:
sudo tc qdisc change dev enp1s0 root netem loss <LOSS %>

To add Delay in the link:
sudo tc qdisc change dev enp1s0 root netem delay <DELAY ms>

To verify the file was transferred correctly:
md5sum <filename>        #Example   md5sum data.bin
This md5sum should match on both server and client side


Note: The run time for 1GB file over 200ms delay and 20% loss takes over 15mins
+---------------------------+
| DEMONSTRATION 			      |
+---------------------------+
https://youtu.be/6MLcDJsctnA

+---------------------------+
| MEMBERS 					        |
+---------------------------+
Names and USC e-mail addresses of team members: Aishwarya Kasim (akasim@usc.edu), Shreyas Rao (srrao@usc.edu)
