import smbus
import time

def print_block(addr, data):
	hex_addr = "%04x" % addr
	hex_data = "".join("%02x " % b for b in data)
	print hex_addr + ": " + hex_data

# Using i2c bus number 1
bus = smbus.SMBus(1)

# Address of the eeprom,
# 7 bits, will be automatically shifted left
address = 0x50

# Memory size to read
memory_size = 256
read_block_size = 16

remaining = memory_size
mem_address = 0

# Write address 0, then read 32 bytes (maximum)
length = read_block_size
if length > remaining: length = remaining
block = bus.read_i2c_block_data(address, 0, length)
remaining -= length

print_block(mem_address, block)
mem_address += length

# Continue reading until end
while remaining > 0:
	length = read_block_size
	if length > remaining: length = remaining

	remaining -= length
	new_address = mem_address + length

	block = []
	while length > 0:
		data = bus.read_byte(address)
		block.append(data)
		length -= 1


	print_block(mem_address, block)
	mem_address = new_address
