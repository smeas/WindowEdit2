#
# Generates zlib compressed blobs intended for packing fonts into an executable.
#
# Use it like:
#     python compress_font.py myfont.ttf g_myfontTTF > myfont_compressed.h
#

import zlib
import struct
import sys

if len(sys.argv) <= 1:
	print("Usage: ")
	print("\tpython compress_font.py myfont.ttf g_myfontTTF > myfont_compressed.h")
	exit()

infile = sys.argv[1]
indata = open(infile, "rb").read()
outdata = zlib.compress(indata, level=9)

with open(infile + ".z", "wb") as file:
	file.write(struct.pack("<ii", len(outdata), len(indata)))
	file.write(outdata)

if len(sys.argv) <= 2:
	exit()

# C header
symname = sys.argv[2]
out = sys.stdout

out.write(f"static const unsigned int {symname}_compressed_size = {len(outdata)};")
out.write(f"static const unsigned int {symname}_uncompressed_size = {len(indata)};")
out.write(f"static const unsigned int {symname}_compressed_data[] = {{")

i = 0
while i < len(outdata):
	if i + 4 <= len(outdata):
		chunk = outdata[i:i+4]
	else:
		chunk = outdata[i:] + (i + 4 - len(outdata)) * b"\0"

	num, = struct.unpack("<I", chunk)
	out.write(hex(num))
	out.write(",")
	i += 4

out.write("};")
