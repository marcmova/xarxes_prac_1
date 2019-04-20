#!usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os

"""     Initialized default values for the settings"""
configuration_file = "server.cfg"
debug = False
authorized_machines_file = "equips.dat"
machines_data = []

"""   Created a function to read parameters from the program arguments"""
def read_parameters():
    global configuration_file
    global debug
    global authorized_machines_file
    for parameter in range(1, len(sys.argv)):
        if sys.argv[parameter] == "-c":
            if len(sys.argv) < parameter + 2:
                print("Parameter error, possible parameters:\n\t-c:\tAllows you to specify the file where configuration is stored, followed by the route of the configuration file.\n\t-d:\tActivates the debug mode.\n");
                sys.exit()
            else:
                configuration_file = sys.argv[parameter+1]
        elif sys.argv[parameter - 1] == "-c":
            pass
        elif sys.argv[parameter] == "-d":
            debug = True
        elif sys.argv[parameter] == "-u":
            if len(sys.argv) < parameter + 2:
                print("Parameter error, possible parameters:\n\t-c:\tAllows you to specify the file where configuration is stored, followed by the route of the configuration file.\n\t-d:\tActivates the debug mode.\n\t-u:\tAllows you to specify the file where authorized machines data is stored, followed by the route of the authorized machines file.");
                sys.exit()
            else:
                authorized_machines_file = sys.argv[parameter + 1]
        elif sys.argv[parameter - 1] == "-u":
            pass
        else:
            print("Parameter error, possible parameters:\n\t-c:\tAllows you to specify the file where configuration is stored, followed by the route of the configuration file.\n\t-d:\tActivates the debug mode.\n\t-u:\tAllows you to specify the file where authorized machines data is stored, followed by the route of the authorized machines file.");
            sys.exit()
read_parameters()
"""     This function reads the configuration file and stores the needed data of the server"""
def set_parameters(file):
    global configuration_file
    with open(file) as f:
        server_data = f.readlines()
    server_data = [x.strip() for x in server_data]
    for line in range(len(server_data)):
        if server_data[line] == "":
            del server_data[line]
    for line in range(len(server_data)):
            server_data[line] = server_data[line].split()
    return server_data[0][1], server_data[1][1], server_data[2][1], server_data[3][1]

"""     Setted the variables to store the server information"""
name, MAC, udp_port, tcp_port = set_parameters(configuration_file)
print name, MAC, udp_port, tcp_port

"""     This function creates the data structure where the data of the clients will be stored, in order to query it later"""
def initialize_machines_data(file):
    global machines_data
    with open(file) as f:
        machines_data = f.readlines()
    machines_data = [x.strip() for x in machines_data]
    for line in range(len(machines_data)):
        if machines_data[line] == "":
            del machines_data[line]

    for line in range(len(machines_data)):
            machines_data[line] = machines_data[line].split()
    for x in range(len(machines_data)):
        machines_data[x].insert(0, "DISCONNECTED")
        machines_data[x][1] = machines_data[x][1] + "\0"
        machines_data[x][2] = machines_data[x][2] + "\0"
        machines_data[x].append("000000\0")
        machines_data[x].append("")
initialize_machines_data(authorized_machines_file)
