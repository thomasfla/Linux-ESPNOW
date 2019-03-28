import serial
import sys
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator

base = "0123456789+/abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

log_file = None
ser = None

n_plot = 0

if(len(sys.argv)==3):
	log_file = open(sys.argv[2],"a+")
	ser = serial.Serial(sys.argv[1], 115200)
	ser.flushInput()

elif(len(sys.argv)==2):
	ser = open(sys.argv[1],"r")


assert(len(sys.argv)==2 or len(sys.argv)==3)


print "Serial opened : " + str(sys.argv[1])


def read(display=True):
	line = ser.readline().rstrip()
	if(log_file != None):
		log_file.write(line + '\n')
	if(display):
		print('\t' + line)
	return line

def read_int(i):
	return int(read().split('\t')[i])

def decode_histo():
	units = read().split('\t')[1]
	bounds = list(map(int, read().split('\t')[1:]))
	nb_values = read_int(1)
	histo = list(map(int, read(False).split('\t')[1:]))
	avg = read_int(1)
	recv = read_int(1)
	sent = read_int(1)
	recv_detail = read(False)

	overtimed = histo[-1]
	histo = histo[:-1]
	nb_values = nb_values-1

	bounds = list(map(lambda x : x/1000., bounds))
	x_axis = [(x+0.5)*(bounds[1]-bounds[0])/nb_values+bounds[0] for x in range(nb_values)]
	

	histo_cumulate = [0 for i in range(len(histo))]
	histo_cumulate[0] = histo[0]
	for i in range(1, len(histo)):
		histo_cumulate[i] = histo_cumulate[i-1] + histo[i]

	histo = list(map(lambda x:x*100./sent, histo))
	histo_cumulate = list(map(lambda x: x*100./sent, histo_cumulate))


	histo_loss = [0 for _ in range(20)]
	current_serie = 0

	for i in range(len(recv_detail)):
		word = base.index(recv_detail[i])
		for j in range(6):
			if(word&(1<<j) == 0):
				current_serie += 1
			else:
				histo_loss[current_serie] += 1
				current_serie = 0


	x_axis_loss = [i for i in range(len(histo_loss))]
	total = 0
	for h in histo_loss:
		total+=h
	print(total)

	plt.ion()

	fig = plt.figure(2*n_plot)
	ax1 = fig.add_subplot(111)
	ax2 = ax1.twinx()

	plt.title('(' + str(n_plot) + ')' + 'Overall receive ' + str(recv*100./sent) + '% ; Sent : ' + str(sent) + ' ; Longer than ' + str(bounds[1]) + units + ' : ' + str(overtimed))

	ax1.plot(x_axis, histo, color='tab:blue')
	ax1.set_ylabel('% packets', color='tab:blue')
	ax1.tick_params(axis='y', labelcolor='tab:blue')
	ax1.set_yticks([i*0.5  for i in range(int(max(histo)+1)*2)])

	ax1.set_xticks([i*(bounds[1]-bounds[0])/10.+bounds[0]  for i in range(11)])


	ax2.plot(x_axis, histo_cumulate, color='tab:red')
	ax2.set_ylabel('% packets', color='tab:red')
	ax2.tick_params(axis='y', labelcolor='tab:red')
	ax2.set_yticks([i*10  for i in range(11)])

	ax2.plot([x_axis[0], x_axis[-1]], [95, 95], color='tab:orange')

	ax1.set_xlabel('Delay ( 1000' + units + ') ; Avg = ' + str(avg) + units)

	fig.tight_layout()
	fig.legend(["histogram", "cumulated", "95%", "hello"])
	plt.grid()

	plt.plot()




	plt.ion()

	fig = plt.figure(2*n_plot+1)
	ax = fig.add_subplot(111)

	plt.title('(' + str(n_plot) + ')' + 'Detail receive ')

	plt.bar(x_axis_loss,histo_loss)
	ax.set_ylabel('# groups')
	ax.set_xlabel('group size')
	ax.set_xticks(x_axis_loss)

	plt.grid()

	plt.plot()


while n_plot < 3:
	line = read(False)
	if("----------" in line):
		n_plot+=1
		print("\033[31mHisto " + str(n_plot) + " \033[31mreceived\033[0m")
		decode_histo()
	else:
		print("\033[34mLine not recognized \033[0m >> \033[33m" + line + "\033[0m")
	
try :
	raw_input("Press [enter] to continue.")
except:
	pass