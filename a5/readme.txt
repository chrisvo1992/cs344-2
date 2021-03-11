
To compile, use:

		bash compileall.sh

Start the servers:

		enc_server <port1>
		dec_server <port2>

Start the client:

		enc_client <plaintext> <keyfile> <port1> 
		dec_client <plaintext> <keyfile> <port2> 
	

************
Current issues:

1.	plaintext3 file is aborting on os1 but runs and produces
		expected output on kali distro. 

2.	plaintext4 isnt being recieved completely. Still working
		on that. 
