#!bin/bash

enc_server 23456 & 
dec_server 34324 & 
enc_client plaintext1 key1 > ciphertext1 
dec_client ciphertext1 key1 > plaintext1_a
