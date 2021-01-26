#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Generate firmware image or header

The image consists of a fw header and the image data itself (and
optionally also the bootloader), this image can be flashed directly to
the MCU.

Additionally, this scrip can also generate image for firmware update
utilities, this image contain basic info about the image and the raw
binary.
"""
import argparse
import crcmod


def gen_header(source):
    """
    Generate image header
    Args:
        source: Binary file to generate header for
    Returns: Bytearray of the header
    """
    crcfunc = crcmod.mkCrcFun(0x11021, rev=False, initCrc=0xffff, xorOut=0)
    crc = crcfunc(source)

    hdr = bytearray(b'\xFF') * 10
    # magic
    hdr[0:4] = bytearray.fromhex('DEADBEEF')[::-1]
    # len
    hdr[4:8] = bytearray.fromhex('%08x' % len(source))[::-1]
    # CRC
    hdr[8:10] = bytearray.fromhex('%04x' % crc)[::-1]
    return hdr


def gen_image(source, bootloader, hdr_start, app_start):
    hdr = gen_header(source)

    image = bytearray(b'\xFF') * (app_start + len(source))
    image[0:len(bootloader)] = bootloader
    image[hdr_start:hdr_start+len(hdr)] = hdr
    image[app_start:app_start+len(source)] = source
    return image

def gen_raw_update(source):
    crcfunc = crcmod.mkCrcFun(0x11021, rev=False, initCrc=0xffff, xorOut=0)
    crc = crcfunc(source)

    image = bytearray()
    image += bytearray.fromhex('%08x' % len(source))[::-1]
    image += bytearray.fromhex('%04x' % crc)[::-1]
    image += source
    return image

if __name__ == "__main__":
    desc = "Firmware images generator"
    parser = argparse.ArgumentParser(
            description="Firmware images generator")
    parser.add_argument('source', type=argparse.FileType('rb'),
                        help="Source .bin file with the firmware")
    parser.add_argument('dest', type=argparse.FileType('wb'),
                        help="Destination .bin file")
    parser.add_argument('--hdr-start', type=int, default=0x800,
                        help="Address of the header in the binary file "
                             "(default 0x%(default)x)")
    parser.add_argument('--app-start', type=int, default=0x880,
                        help="Address of the application in the binary file "
                             "(default 0x%(default)x)")
    excl_group = parser.add_mutually_exclusive_group(required=True)
    excl_group.add_argument('-b', '--bl', type=argparse.FileType('rb'),
                            help="Build a flashable firmware image, "
                                 "use this bootloader .bin image")
    excl_group.add_argument('--hdr', action="store_true",
                            help="Build .bin header for give image")
    excl_group.add_argument('--raw', action="store_true",
                            help="Build a raw fw update image")
    args = parser.parse_args()

    source = args.source.read()
    args.source.close()

    if args.hdr:
        header = gen_header(source)
        args.dest.write(header)
        args.dest.close()
        print("Header written")
    elif args.raw:
        image = gen_raw_update(source)
        args.dest.write(image)
        args.dest.close()
        print("FW image written")
    else:
        bl = args.bl.read()
        args.bl.close()
        image = gen_image(source, bl, args.hdr_start, args.app_start)
        args.dest.write(image)
        args.dest.close()
        print("Flashable image written")
