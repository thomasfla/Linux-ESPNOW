import serial
import sys
import matplotlib.pyplot as plt

assert len(sys.argv)==3

log_file = open(sys.argv[2],"a+")

ser = serial.Serial(sys.argv[1], 115200)
ser.flushInput()

print "Serial opened : " + str(sys.argv[1])

def read():
	line = ser.readline()
	log_file.write(line)
	return line

def read_int(i):
	return int(read().rstrip().split('\t')[i])

def decode_histo():
	units = read().rstrip().split('\t')[1]
	bounds = list(map(int, read().rstrip().split('\t')[1:]))
	nb_values = read_int(1)
	histo = list(map(int, read().rstrip().split('\t')[1:]))
	avg = read_int(1)
	recv = read_int(1)
	sent = read_int(1)

	overtimed = histo[-1]
	histo = histo[:-1]
	nb_values = nb_values-1

	x_axis = [(x+0.5)*(bounds[1]-bounds[0])/nb_values+bounds[0] for x in range(nb_values)]
	x_axis = list(map(lambda x : x/1000., x_axis))

	histo_cumulate = [0 for i in range(len(histo))]
	histo_cumulate[0] = histo[0]
	for i in range(1, len(histo)):
		histo_cumulate[i] = histo_cumulate[i-1] + histo[i]

	histo = list(map(lambda x:x*100./sent, histo))
	histo_cumulate = list(map(lambda x: x*100./sent, histo_cumulate))

	fig, ax1 = plt.subplots()
	ax2 = ax1.twinx()

	plt.title('Histogram')

	ax1.plot(x_axis, histo, color='tab:blue')
	ax1.set_ylabel('% packets', color='tab:blue')
	ax1.tick_params(axis='y', labelcolor='tab:blue')

	ax2.plot(x_axis, histo_cumulate, color='tab:red')
	ax2.set_ylabel('% packets', color='tab:red')
	ax2.tick_params(axis='y', labelcolor='tab:red')

	ax2.plot([x_axis[0], x_axis[-1]], [95, 95], color='tab:orange')

	ax1.set_xlabel('Delay (' + units + ')')

	fig.tight_layout()
	fig.legend()

	plt.show()


while True:
		#try:
		line = read()
		if("----------" in line):
			decode_histo()
			break
		else:
			print("pas hello")
		#except:
		print("Keyboard Interrupt")
		#break
	
