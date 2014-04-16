#!/usr/bin/env python

from __future__ import division
from socket import *
from time import sleep
import sys
from time import time
import os
import math

BUFSIZ = 1000000
addr = raw_input('Please input the ip:')
tcpCliSock = socket(AF_INET, SOCK_STREAM)
tcpCliSock.connect((addr,21567))
count = 0
print 'The package is 1Mb'
j   = raw_input('Please input the time:')
D = raw_input('Please input the rate:')
D = int(D)
data = '1'*D*1250
k = int(j)*100
i=0
frecord = time()
while 1:
   record = time()
   data = '1'*int((record-frecord)*D*125000)
   tcpCliSock.send(data)
   sleep(0.01)
   if i == k:
      break
   i += 1
   frecord = record
tcpCliSock.close()
