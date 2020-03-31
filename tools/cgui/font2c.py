#!/usr/bin/python3
# Convert font file to C file

import argparse
from PIL import Image, ImageFont, ImageDraw
from img2array import img2array, array2c, array2img


def font2array(font, c, width, height):
    img = Image.new('L', (width, height), "white")
    draw = ImageDraw.Draw(img)

    print(chr(c))
    draw.text((0, 0), chr(c), font=font)
    return img2array(img, treshold=170)[0]


def font2header(filename, size):
    font = ImageFont.truetype(filename, size)
    width, height = font.getsize('m')

    start = 0x21
    end = 0x7e

    array = []
    for c in range(start, end+1):
        array += font2array(font, c, width, height)

    farraydraw(array, width, height, 0x7e-0x21+1)

    definition = "const cgui_font_t cgui_font_%dx%d = { cguii_font_data_%dx%d"\
                 ", %d, %d, %d, %d };" % (width, height, width, height, width,
                                          height, start, end)

    data = "static " + array2c(array, "cguii_font_data_%dx%d" % (width, height))

    print(data)
    print(definition)


def farraydraw(array, width, height, len):
    img = Image.new('L', (width*len, height))
    posx = 0
    offset = 0
    rot = 0

    for i in range(len):
        for y in range(height):
            for x in range(posx, posx+width):
                val = 0
                if array[offset] & (1 << rot):
                    val = 0xff

                img.putpixel((x, y), val)

                rot += 1
                if rot == 8:
                    rot = 0
                    offset += 1

        posx += width
        offset = i*int(width*height/8+1)
        rot = 0

    img.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(usage="Generate font in cgui format")

    parser.add_argument("filename", help="font file (.ttf)")
    parser.add_argument("size", type=int, help="Font size")
    args = parser.parse_args()

    font2header(args.filename, args.size)
