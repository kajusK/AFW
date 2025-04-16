#!/usr/bin/python
# -*- coding: utf-8 -*-
'''
Generate firmware images

* `--image`: FW update image starts with header followed by a binary data.
* `--bl`: Image that can be flashed directly, contains both the bootloader and firmware image
* `--uf2`: FW update image in UF2 format
'''

import re
import ctypes
import crcmod
import argparse
import subprocess
from typing import Optional
from bin2uf2 import bin2uf2


FW_HDR_SIZE = 0x80


class FwMetaData(ctypes.Structure):
    _pack_ = 1  # Zajistí že není žádný padding (jako __attribute__((packed)))
    _fields_ = [
        ('major', ctypes.c_uint8),
        ('minor', ctypes.c_uint8),
        ('patch', ctypes.c_uint8),
        ('git_hash', ctypes.c_char * 47),
        ('description', ctypes.c_char * 68),
    ]


class FwHdr(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('magic', ctypes.c_uint32),
        ('len', ctypes.c_uint32),
        ('crc', ctypes.c_uint16),
        ('meta', FwMetaData),
    ]


class Version:
    major: int
    minor: int
    patch: int

    def __init__(self, data: Optional[str] = None):
        if data is None:
            self.major = 0
            self.minor = 0
            self.patch = 0
        else:
            match = re.search(r'^(\d+).(\d+).(\d+)$', data.strip())
            if match is None:
                raise ValueError('Invalid version format, expected `1.2.3`')
            self.major = int(match.group(1))
            self.minor = int(match.group(2))
            self.patch = int(match.group(3))


def get_git_hash() -> str:
    try:
        result = subprocess.run(
            ['git', 'describe', '--always', '--dirty', '--abbrev=40'],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        raise ValueError('Unable to read latest git commit hash')


def gen_header(binary: bytes, version: Version, magic: int, description: str) -> bytes:
    '''
    Generate fw image header
    '''
    crcfunc = crcmod.mkCrcFun(0x11021, rev=False, initCrc=0xffff, xorOut=0)

    hdr = FwHdr()
    hdr.magic = magic
    hdr.len = len(binary)
    hdr.crc = crcfunc(binary)
    hdr.meta.major = version.major
    hdr.meta.minor = version.minor
    hdr.meta.patch = version.patch
    hdr.meta.git_hash = bytes(get_git_hash(), encoding='ascii')
    hdr.meta.description = bytes(description, encoding='ascii')

    data = bytes(hdr)
    if len(data) > FW_HDR_SIZE:
        raise ValueError('Invalid header length, internal error...')
    return data.ljust(FW_HDR_SIZE, b'\x00')


def gen_img_with_bl(image: bytes, bootloader: bytes, image_offset: int) -> bytes:
    '''
    Merge fw image together with bootloader into all in one image
    '''
    if len(bootloader) > image_offset:
        raise ValueError('The bootloader won\'t fit the expected area')

    return bootloader.ljust(image_offset, b'\x00') + image


if __name__ == "__main__":
    desc = "Firmware images generator"
    parser = argparse.ArgumentParser(
            description="Firmware images generator")
    parser.add_argument('magic', type=lambda x: int(x, 16),
                        help="Firmware magic number (e.g. 0xaabbccddee)")
    parser.add_argument('source', type=argparse.FileType('rb'),
                        help="Source .bin file with the firmware")
    parser.add_argument('dest', type=argparse.FileType('wb'),
                        help="Destination .bin file")

    parser.add_argument('--version', type=Version, default=Version(),
                        help='Release version, e.g. `1.2.3`')
    parser.add_argument('--image-start', type=int, default=0x1400,
                        help='Address of the image in the flash (after bootloader)')
    parser.add_argument('--description', type=str, default='devel',
                        help='FW image textual description')

    excl_group = parser.add_mutually_exclusive_group(required=True)
    excl_group.add_argument('--bl', type=argparse.FileType('rb'),
                            help="Build a flashable firmware image, "
                                 "use this bootloader .bin image")
    excl_group.add_argument('--image', action="store_true",
                            help="Build a fw update image")
    excl_group.add_argument('--uf2', action="store_true",
                            help="Build a UF2 update image")
    args = parser.parse_args()

    # generate fw image
    binary = args.source.read()
    header = gen_header(binary, args.version, args.magic, args.description)
    image = header + binary
    args.source.close()

    # Process outputs
    if args.image:
        args.dest.write(image)
        args.dest.close()
        print(f'FW Image written to {args.dest.name}')
    elif args.uf2:
        uf2 = bin2uf2(image)
        args.dest.write(uf2)
        args.dest.close()
        print(f'UF2 Image written to {args.dest.name}')
    elif args.bl:
        bl = args.bl.read()
        args.bl.close()

        combined = gen_img_with_bl(image, bl, args.image_start)
        args.dest.write(combined)
        args.dest.close()
        print(f'Combined BL+FW Image written to {args.dest.name}')
