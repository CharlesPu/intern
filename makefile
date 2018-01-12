#!/bin/sh

all:
	cd ./clie && make
	cd ./serv && make
clean:
	cd ./clie && make clean_all
	cd ./serv && make clean_all

	
