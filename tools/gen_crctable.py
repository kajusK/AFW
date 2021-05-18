#!/usr/bin/python
# Calculate crc lookup table for given polynomial

import sys


def gen_table(polynomial, length):
    table = [0 for i in range(256)]

    for i in range(256):
        remainder = i << (length-8)
        for bit in range(8, 0, -1):
            if remainder & (1 << (length-1)):
                remainder = (remainder << 1) ^ polynomial
            else:
                remainder <<= 1
        table[i] = remainder & (2**length-1)
    return table


def print_table(table, length):
    linelen = 80
    out = ""
    itemlen = 2+length/4+2
    out = "static const uint%d_t crc%di_lut_table[256] = {" % (length, length)

    for item in table:
        if linelen + itemlen > 80:
            out += '\n    '
            linelen = 4
        out += "0x%0*x, " % (int(length/4), item)
        linelen += itemlen
    out = out[:-2]
    out += '\n};\n'
    print(out)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Lookup table generator for CRC")
        print("Usage: %s polynomial bytes_len" % (sys.argv[0]))
        exit(0)

    polynomial = int(sys.argv[1], 0)
    len = int(sys.argv[2])

    print_table(gen_table(polynomial, len), len)
