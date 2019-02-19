#!/usr/bin/python
# -*- coding: utf-8 -*-
import socket
import random
import struct
import time
import threading
import os
import sys

global actualstat, num_subs
num_subs = 0

DISCONNECTED = 0XA0
NOT_SUBSCRIBED = 0XA1
WAIT_ACK_SUBS = 0XA2
WAIT_INFO = 0XA3
WAIT_ACK_INFO = 0XA4
SUBSCRIBED = 0XA5
SEND_HELLO = 0XA6

SUBS_REQ = 0X00
SUBS_ACK = 0X01
SUBS_REJ = 0X02
SUBS_INFO = 0X03
INFO_ACK = 0X04
SUBS_NACK = 0X05

HELLO = 0X10
HELLO_REJ = 0X11

stats = { 0XA0:'DISCONNECTED', 0XA1:'NOT_SUBSCRIBED', 0XA2:'WAIT_ACK_SUBS', 0XA3:'WAIT_INFO', 0XA4:'WAIT_ACK_INFO', 0XA5:'SUBSCRIBED', 0XA6:'SEND_HELLO', 0X00:'SUBS_REQ', 0X01:'SUBS_ACK', 0X02:'SUBS_REJ', 0X03:'SUBS_INFO', 0X04:'INFO_ACK', 0X05:'SUBS_NACK', 0X10:'HELLO', 0X11:'HELLO_REJ'}

def readFile():
	c_file = "client.cfg"
	if len(sys.argv) > 1:
		for x in xrange(len(sys.argv)-1):
			if sys.argv[x] == "-c":
				c_file = sys.argv[x+1]
	f=open(c_file,"r")
	l = f.readlines()
	f.close()
	name = takeArg(0,l)
	situation = takeArg(1,l)
	elem = takeArg(2,l)
	MAC = takeArg(3,l)
	TCP = takeArg(4,l)
	host = takeArg(5,l)
	port = int(takeArg(6,l))
	return name, situation, elem, MAC, TCP, host, port
	
def takeArg(x,l):
	h = l[x].split(' = ')
	h = h[1].split('\n')
	h = h[0].split(' ')
	return h[0]

def subscribe():
	global sock_udp, randnum, mac_srv, actualstat,stats
	randnum = '00000000'
	timelist= [1, 1, 1, 2, 3, 3, 3]
	actualstat = NOT_SUBSCRIBED
	sock_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock_udp.settimeout(timelist[0])
	n=0
	sock_udp.settimeout(timelist[n])
	pack_SUBS = createPack(SUBS_REQ)
	while True:
		while(n<7):
			sock_udp.settimeout(timelist[n])
			sock_udp.sendto(pack_SUBS, (host, port))
			actualstat = WAIT_ACK_SUBS
			print time.strftime( '%X' ) + " MSG.  =>  Controlador passa a l'estat: WAIT_ACK_SUBS"
			
			try:
				recieved = sock_udp.recv(103)
			except socket.timeout:
				recieved = ''
			if len(recieved) == 103:
				n=8
			else:
				sock_udp.settimeout(timelist[n])
				n+=1
		if n == 7:
			time.sleep(2)
			return False
		if recieved[0] == struct.pack('B', SUBS_ACK):
			randnum = recieved[14:22]
			srv_TCP = int(recieved[23:28])
			mac_srv = recieved[1:14]
			sock_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			print time.strftime( '%X' ) + " MSG.  =>  Obert port TCP " + str(srv_TCP) + "per la comunicació amb el servidor"
			pack_INFO = createPack(SUBS_INFO)
			sock_udp.sendto(pack_INFO, (host, srv_TCP))
			actualstat = WAIT_ACK_INFO
			print time.strftime( '%X' ) + " MSG.  =>  Controlador passa a l'estat: WAIT_ACK_INFO"
			recieved = sock_udp.recv(103)
			if recieved[0] == struct.pack('B', INFO_ACK) and checkPack(recieved) == True:
				actualstat = SUBSCRIBED
				print time.strftime( '%X' ) + " MSG.  =>  Controlador passa a l'estat: SUBSCRIBED"
				return True
			else:
				actualstat = NOT_SUBSCRIBED
				return False
		elif recieved[0] == struct.pack('B', SUBS_NACK):
			actualstat = NOT_SUBSCRIBED
			sock_udp.sendto(pack_SUBS, (host,port))
		else:
			actualstat = NOT_SUBSCRIBED
			return False

def checkPack(package):
	global mac_srv, randnum
	if package[1:14] == mac_srv and package[14:22] == randnum:
		return True
	else:
		return False
		
def createPack(packtype):
	if packtype == SUBS_REQ:
		data = name+','+situation
		package = struct.pack('B', packtype)+MAC+'\0'+randnum+'\0'+data.ljust(80,'\0')
		return package
	elif packtype == SUBS_INFO:
		data = TCP+','+elem
		package = struct.pack('B', packtype)+MAC+'\0'+randnum+'\0'+data.ljust(80,'\0')
		return package
	elif packtype == HELLO:
		data = name+','+situation
		package = struct.pack('B', packtype)+MAC+'\0'+randnum+'\0'+data.ljust(80,'\0')
		return package

def hellos():
	global sock_udp
	pack_HELLO = createPack(HELLO)
	while True:
		time.sleep(2)
		sock_udp.sendto(pack_HELLO, (host,port))
		
def completeSubs():
	global num_subs, actualstat, stats
	k = num_subs
	while k < 3:
		print time.strftime( '%X' ) + " MSG.  =>  Controlador en l'estat: " + stats[actualstat] + ", procés de subscripció: " + str(k+1)
		time.sleep(0.5)
		if subscribe() == True:
			k = 4
		else:
			k += 1
	if k == 3:
		print time.strftime( '%X' ) + " MSG.  =>  Superat el nombre de processos de subscripció ( 3 )"
	if k == 4:
		actualstat = SEND_HELLO
		print time.strftime( '%X' ) + " MSG.  =>  Controlador passa a l'estat: " + stats[actualstat]
		try:
			recieved = sock_udp.recv(103)
		except socket.timeout:
			recieved = ''
		if len(recieved) == 103:
			print recieved
			if checkPack(recieved) == False:
				completeSubs()
			else:
				actualstat = SEND_HELLO
		threading.Thread(target = hellos).start()
		threading.Thread(target = waitHellos).start()
		threading.Thread(target = commands).start()
		

def waitHellos():
	global sock_udp, actualstat
	sock_udp.settimeout(2)
	k = 0
	while k < 3:
		try:
			recieved = sock_udp.recv(103)
		except socket.timeout:
			recieved = ''
		if len(recieved) == 103:
			if checkPack(recieved) == False:
				actualstat = NOT_SUBSCRIBED
				completeSubs()
		if len(recieved) == 0:
			k = k + 1
	if k == 3:
			actualstat = NOT_SUBSCRIBED
			completeSubs()

def commands():
	global elem, name, stiuation, stats, actualstat
	while True:
		line = raw_input("Introduir comandes: ")
		if line == "quit":
			os._exit(0)
		if line == "stat":
			elem = str(elem).split(';')
			print "******************** DADES CONTROLADOR *********************"
			print "  MAC: " + MAC + ", Nom: " + name + ", Situació: "+situation
			print "\n"
			print "   Estat: " + stats[actualstat]
			print "\n"
			print "    Dispos.	 valor"
			print "    -------	 ------"
			for x in xrange(len(elem)):
				print "     " +elem[x] + "	 NONE"
			print "\n"
			print "************************************************************"
				
		

	
	


actualstat = NOT_SUBSCRIBED
name, situation, elem, MAC, TCP, host, port = readFile()
completeSubs()
