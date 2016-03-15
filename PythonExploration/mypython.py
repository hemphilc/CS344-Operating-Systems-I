#!/usr/bin/python

# Corey Hemphill
# hemphilc@oregonstate.edu
# CS344_001 Operating Systems I
# Assignment 5 - Python Exploration
# January 16, 2016

import string
import random
import os

# Seed random numbers
random.seed()

# Set alpha to a string of all of the lowercase characters
alpha = string.ascii_lowercase

# Print file content header with fancy colors
print '\033[1;32;40m\n*****FILE CONTENTS*****\033[1;37;40m'

# For loop to create the three random strings -- executes three times
for x in range(1, 4):
	# Initialize random string as a blank string
	rand_string = ''
	# For loop for inserting the random characters into the random string -- executes ten times
	for y in range(0, 10):
		# Insert the random character at the end of the string
		rand_string += random.choice(alpha)
	# Concatenate a string for the file name for storing the random string
	print 'myfile' + str(x) + '.txt: ' + rand_string
	# Insert a new line character at the end of the string
	rand_string += '\n'
	# Create and open the file for storing the random string
	myfile = open('myfile' + str(x) + '.txt', 'w+')
	# Write the random string to file
	myfile.write(rand_string)
	# Close the file
	myfile.close()

# Print random product header with fancy colors
print '\033[1;32;40m\n*****RANDOM PRODUCT*****\033[1;37;40m'

# Get first random integer between 1 and 42
rand_int1 = random.randint(1, 42)
# Get second random integer between 1 and 42
rand_int2 = random.randint(1, 42)
# Get the product of the two random integers
product = rand_int1 * rand_int2

# Print the arithmetic and product of the two random integers to console
print '     ' + str(rand_int1) + ' * ' + str(rand_int2) + ' = ' + str(product) + '\n'

