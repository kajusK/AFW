#!/usr/bin/env python
# Create a UF2 format binary from .bin file for flashing firmware over USB MSC

import argparse
import crcmod
import math

UF2_FLAG_NOT_MAIN_FLASH = 0x00000001
UF2_FLAG_FILE_CONTAINER = 0x00001000
UF2_FLAG_FAMILY_ID_PRESENT = 0x00002000
UF2_FLAG_MD5_CHECKSUM = 0x00004000


def gen_block(payload, address, blockNo, numBlocks, fileSize, flags):
    block = bytearray(b'\xff') * 512
    block[0:4] = bytearray.fromhex('0A324655')[::-1]
    block[4:8] = bytearray.fromhex('9E5D5157')[::-1]
    block[8:12] = bytearray.fromhex('%08x' % flags)[::-1]
    block[12:16] = bytearray.fromhex('%08x' % address)[::-1]
    block[16:20] = bytearray.fromhex('%08x' % len(payload))[::-1]
    block[20:24] = bytearray.fromhex('%08x' % blockNo)[::-1]
    block[24:28] = bytearray.fromhex('%08x' % numBlocks)[::-1]
    block[28:32] = bytearray.fromhex('%08x' % fileSize)[::-1]
    block[32:len(payload)+32] = payload
    block[508:512] = bytearray.fromhex('0AB16F30')[::-1]

    return block


def gen_crc(payload):
    crcfunc = crcmod.mkCrcFun(0x11021, rev=False, initCrc=0xffff, xorOut=0)
    return crcfunc(payload)


def bin2uf2(payload, chunk_size):
    uf2 = bytearray()
    num_blocks = math.ceil(len(payload)/chunk_size) + 1
    # first block contains crc and length
    crc = gen_crc(payload)
    block = bytearray(b'\xff') * 6
    block[0:2] = bytearray.fromhex("%04x" % crc)[::-1]
    block[2:6] = bytearray.fromhex("%08x" % len(payload))[::-1]
    uf2 += gen_block(block, 0, 0, num_blocks, len(payload),
                     UF2_FLAG_NOT_MAIN_FLASH)

    pos = 0
    block_no = 1

    while pos < len(payload):
        block = payload[pos:pos+chunk_size]
        uf2 += gen_block(block, pos, block_no, num_blocks, len(payload), 0)
        pos += chunk_size
        block_no += 1

    print("Image %d bytes long, crc 0x%04x" % (len(payload), crc))
    print("%d blocks written" % block_no)
    return uf2


def uf2_size(value):
    ivalue = int(value)
    if ivalue < 1 or ivalue > 476:
        raise argparse.ArgumentTypeError("%s is not within 1 and 476" % value)
    return ivalue


parser = argparse.ArgumentParser(
        description="Generate a UF2 firmware image from .bin file",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('source', help="Source .bin file",
                    type=argparse.FileType('rb'))
parser.add_argument('destination', help="Destination .uf2 file",
                    type=argparse.FileType('wb'))
parser.add_argument('-s', '--chunk_size', help="Destination .uf2 file",
                    type=uf2_size, default=256)
args = parser.parse_args()

print("Generating UF2 image")
args.destination.write(bin2uf2(args.source.read(), args.chunk_size))
args.destination.close()
args.source.close()
