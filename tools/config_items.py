#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Generate config_items.h context from config_items.ods file
"""
import os
from os.path import dirname, abspath
import argparse
from jinja2 import Template
from pyexcel_odsr import get_data

COL_NAME = 0
COL_TYPE = 1
COL_DEFAULT = 2
COL_DESC = 3

TYPE_INT = 0
TYPE_BOOL = 1
TYPE_STRING = 2
TYPE_FLOAT = 3
TYPE_PTR = 4


def parse_sheet(filename):
    data = list(get_data(filename).values())[0]
    items = [[] for i in range(5)]

    for line in data[1:]:
        if len(line) not in (3, 4):
            print("Skipping '{}', not all fields specified".format(str(line)))
            continue

        type_str = line[COL_TYPE].strip()
        if type_str == "int":
            type = TYPE_INT
        elif type_str == "bool":
            type = TYPE_BOOL
        elif type_str == "string":
            type = TYPE_STRING
        elif type_str == "float":
            type = TYPE_FLOAT
        elif type_str == "ptr":
            type = TYPE_PTR
        else:
            print("Unknown type: {}".format(type))
            continue

        desc = None
        if len(line) == 4:
            desc = line[COL_DESC]
        default = line[COL_DEFAULT]
        if type in (TYPE_STRING, TYPE_PTR):
            default = '"{}"'.format(default)

        items[type].append({'name': line[COL_NAME].strip().upper(),
                            'default': default,
                            'desc': desc})
    return items


def gen_bool_def(items):
    data = []
    for i, item in enumerate(items):
        if i % 8 == 0:
            data.append(0)
        val = 1 if item['default'] else 0
        data[int(i/8)] |= val << (i % 8)
    return data


def shorten_strings(items, max_len):
    for item in items:
        if len(item['default']) > max_len:
            item['default'] = item['default'][:max_len]


desc = "Generate config_items.h from .ods description"
parser = argparse.ArgumentParser(
        description=desc,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('source', help="Source .ods file with config options")
parser.add_argument('-d', '--dest', type=argparse.FileType('w'),
                    help="Save result to file")
parser.add_argument('-m', '--max_len', type=int, default=16,
                    help="Max length of string allowed")
args = parser.parse_args()


items = parse_sheet(args.source)
bool_defs = gen_bool_def(items[TYPE_BOOL])
shorten_strings(items[TYPE_STRING], args.max_len)

cur_path = dirname(abspath(__file__))
with open(os.path.join(cur_path, 'templates/config_items.tpl'), 'r') as tpl_file:
    tpl = tpl_file.read()
template = Template(tpl)

data = [
    {'type': 'int',
     'content': items[TYPE_INT]},
    {'type': 'float',
     'content': items[TYPE_FLOAT]},
    {'type': 'string',
     'content': items[TYPE_STRING]},
    {'type': 'ptr',
     'content': items[TYPE_PTR]},
        ]
content = template.render(data=data, bools=items[TYPE_BOOL],
                          bool_default=bool_defs, max_str_len=args.max_len)

if args.dest:
    args.dest.write(content)
    args.dest.close()
    print('File generated')
else:
    print(content)
