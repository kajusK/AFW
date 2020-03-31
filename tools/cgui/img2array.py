#!/usr/bin/python3
# Generate bitmap in squeezed form (8 pixels per 1 byte = monochromatic)

import argparse
from PIL import Image


def img2array(img, width=None, height=None, treshold=128):
    if width is None and height is not None:
        width = int(img.width * height/img.height)
    if height is None and width is not None:
        height = int(img.height * width/img.width)
    if height is None and width is None:
        width, height = img.size

    img = img.convert("L")
    img = img.resize((width, height))
    values = list(img.getdata())

    array = [0]
    offset = 0
    rot = 0

    for value in values:
        bit = 1 << rot
        if value >= treshold:
            array[offset] |= bit
        else:
            array[offset] &= ~bit

        rot += 1
        if rot == 8:
            rot = 0
            offset += 1
            array.append(0)

    return (array, width, height)


def array2c(array, name):
    out = "const uint8_t %s[] = {\n   " % (name)
    chars = 3

    for i in array:
        out += " 0x%02x," % (i)
        chars += 6
        if chars > 80 - 5:
            chars = 3
            out += "\n   "

    out += "\n};"
    return out


def array2img(array, width, height):
    img = Image.new('L', (width, height))
    offset = 0
    rot = 0

    for y in range(height):
        for x in range(width):
            val = 0
            if array[offset] & (1 << rot):
                val = 0xff

            img.putpixel((x, y), val)

            rot += 1
            if rot == 8:
                rot = 0
                offset += 1

    img.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(usage="""
Generate array for cgui img structure from given image file

If either width or height is supplied,
second dimension is calculated, if both are given, image size is forced""")

    parser.add_argument("filename", help="Image file")
    parser.add_argument("-x", "--width", type=int, help="Required width")
    parser.add_argument("-y", "--height", type=int, help="Required height")

    args = parser.parse_args()

    img = Image.open(args.filename)
    array, width, height = img2array(img, args.width, args.height)
    print(array2c(array, "test"))

    print("Dimensions are %dx%d" % (width, height))
