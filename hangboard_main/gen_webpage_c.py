#!/usr/bin/env python3
import os
import sys

source = "webpage.html"
header = "webpage.h"

header_top = """// Serving a web page (from flash memory)
// formatted as a string literal!
#ifndef WEBPAGE_H
#define WEBPAGE_H
char webpage[] PROGMEM = R"=====(
"""

header_bottom = ")=====\";\n#endif"

with open(source, "r") as f:
    with open(header, "w") as fh:
        fh.write(header_top)
        fh.write(f.read())
        fh.write(header_bottom)
