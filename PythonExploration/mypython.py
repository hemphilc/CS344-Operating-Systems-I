#!/bin/bash
""":"
exec python $0 ${1+"$@"}
"""

# Corey Hemphill
# hemphilc@oregonstate.edu
# CS344_100 Operating Systems I
# Assignment 5 - Python Exploration
# January 16, 2016

import string
import random
import os

random.seed()

alpha = string.ascii_lowercase

print '\033[1;32;40m\n*****FILE CONTENTS*****\033[1;37;40m'

for x in range(1, 4):
	rand_string = ''
	for y in range(0, 10):
		rand_string += random.choice(alpha)
	print 'myfile' + str(x) + '.txt: ' + rand_string
	rand_string += '\n'
	myfile = open('myfile' + str(x) + '.txt', 'w+')
	myfile.write(rand_string)
	myfile.close()

print '\033[1;32;40m\n*****RANDOM PRODUCT*****\033[1;37;40m'

rand_int1 = random.randint(1, 42)
rand_int2 = random.randint(1, 42)
product = rand_int1 * rand_int2

print '     ' + str(rand_int1) + ' * ' + str(rand_int2) + ' = ' + str(product) + '\n'
