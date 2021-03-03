#!/bin/bash
gcc -g -std=c99 -o enc_server enc_server.c
gcc -g -std=c99 -o enc_client enc_client.c
gcc -g -std=c99 -o dec_server dec_server.c
gcc -g -std=c99 -o dec_client dec_client.c
gcc -g -std=c99 -o keygen keygen.c
gcc -g -std=c99 -o client client.c
gcc -g -std=c99 -o server server.c
