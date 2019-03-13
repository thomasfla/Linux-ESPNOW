import serial
import time
import csv
import sys

assert len(sys.argv)==3


ser = serial.Serial(sys.argv[1], 115200)
ser.flushInput()

print "Serial opened : " + str(sys.argv[1])

while True:
	try:
		ser_bytes = ser.readline()
		#print(ser_bytes)
		with open(sys.argv[2],"a") as f:
			f.write(ser_bytes)
	except:
		print("Keyboard Interrupt")
		break
