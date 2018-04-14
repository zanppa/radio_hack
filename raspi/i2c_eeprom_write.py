import sys
import smbus
import time

def print_block(addr, data):
        hex_addr = "%04x" % addr
        hex_data = "".join("%02x " % b for b in data)
        print hex_addr + ": " + hex_data

# File name to write to eeprom
filename = "write.txt"

# Address of the eeprom,
# 7 bits, will be automatically shifted left
address = 0x50

# Memory size
memsize = 256		# 256 bytes

# Write cycle delay time, must be at least 5 ms
wr_cycle = 0.06

# Using i2c bus number 1
bus = smbus.SMBus(1)

try:
	file = open(filename, "rt")
	contents = file.readlines()
	file.close()
except:
	print "Could not open file"
	sys.exit(-1)

# Data buffer to store hex data
buffer = []
addr = 0
for line in contents:
	# Split with spaces
	columns = line.split(" ")

	# First column is address, forget it, parse others to decimal and store
	# Also forget last item, which is newline
	try:
		file_addr = int(columns[0][:-1], 16)
	except:
		print "Could not convert address to integer"
		print "Line: " + line
		sys.exit(-1)

	if addr != file_addr:
		print "File contents not sequential"
		print "Internal address: " + "%04x" % addr
		print "Line: " + line
		sys.exit(-1)

	items = len(columns[1:-1])
	addr += items

	for item in columns[1:-1]:
		print item
		try:
			buffer.append(int(item, 16))
		except:
			print "Could not convert byte to integer"
			print "Line: " + columns[0][:-1] + ", byte: " + item
			sys.exit(-1)

# Verify that the file fits
if addr > memsize or len(buffer) > memsize:
	print "File contains too much data to fit into EEPROM!"
	sys.exit(-1)


print "Writing..."
write_addr = 0
for byte in buffer:
	bus.write_byte_data(address, write_addr, byte)
	time.sleep(wr_cycle)
	write_addr += 1
	print "%04x: %02x" % (write_addr, byte)

print "Done"
