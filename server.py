#!usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
"""   Initialized default values for the settings*/"""
configuration_file = "server.cfg"
debug = False
authorized_machines = "equips.dat"
def read_parameters():
    global configuration_file
    global debug
    global authorized_machines
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
                authorized_machines = sys.argv[parameter + 1]
        elif sys.argv[parameter - 1] == "-u":
            pass
        else:
            print("Parameter error, possible parameters:\n\t-c:\tAllows you to specify the file where configuration is stored, followed by the route of the configuration file.\n\t-d:\tActivates the debug mode.\n\t-u:\tAllows you to specify the file where authorized machines data is stored, followed by the route of the authorized machines file.");
            sys.exit()
read_parameters()
print authorized_machines, configuration_file, debug
def read_authorized_machines(file):
    pass
