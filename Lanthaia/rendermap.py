#!/usr/bin/env python3

import argparse
import math
import cairo
import struct
import sys

from pathlib import Path

from PIL import Image
import PIL.ImageOps


def pil2cairo(im):
    """Transform a PIL Image into a Cairo ImageSurface."""

    assert sys.byteorder == "little", "We don't support big endian"

    if im.mode != "RGBA":
        im = im.convert("RGBA")

    s = im.tobytes("raw", "BGRA")
    a = bytearray(s)  # array.array('B', s)
    dest = cairo.ImageSurface(cairo.FORMAT_ARGB32, im.size[0], im.size[1])
    ctx = cairo.Context(dest)
    non_premult_src_wo_alpha = cairo.ImageSurface.create_for_data(
        a, cairo.FORMAT_RGB24, im.size[0], im.size[1]
    )
    non_premult_src_alpha = cairo.ImageSurface.create_for_data(
        a, cairo.FORMAT_ARGB32, im.size[0], im.size[1]
    )
    ctx.set_source_surface(non_premult_src_wo_alpha)
    ctx.mask_surface(non_premult_src_alpha)
    return dest


sect_min = (0, 0)
sect_max = (3, 3)
sect_size = 200
im_sect_size = 200

WIDTH, HEIGHT = (
    (sect_max[0] - sect_min[0]) * im_sect_size,
    (sect_max[1] - sect_min[1]) * im_sect_size,
)

surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, WIDTH, HEIGHT)
ctx = cairo.Context(surface)

ctx.set_source_rgb(1, 1, 1)
ctx.paint()


def readsz(f):
    s = b""
    while True:
        ch = f.read(1)
        if not ch or ch == b"\x00":
            break
        s += ch
    return s


# sectors are 200m x 200m
for sect_x in range(sect_min[0], sect_max[0]):
    for sect_y in range(sect_min[1], sect_max[1]):
        sect_base_x = sect_x * sect_size
        sect_base_y = sect_y * sect_size
        im_base_x = (sect_x - sect_min[0]) * im_sect_size
        im_base_y = (sect_y - sect_min[1]) * im_sect_size

        ctx.set_source_rgba(0, 0, 0, 0.2)
        ctx.set_dash([10])
        # ctx.move_to(im_base_x, im_base_y)
        ctx.move_to(im_base_x + im_sect_size, im_base_y)
        ctx.line_to(im_base_x + im_sect_size, im_base_y + im_sect_size)
        ctx.line_to(im_base_x, im_base_y + im_sect_size)
        # ctx.line_to(im_base_x, im_base_y)
        ctx.stroke()
        ctx.set_dash([])

        path = Path(f"dist/tolcl/area/{sect_x}+{sect_y}.sect")

        if not path.exists():
            ctx.set_source_rgba(1, 0, 0, 0.1)
            ctx.rectangle(im_base_x, im_base_y, im_sect_size, im_sect_size)
            ctx.fill()
            continue
        else:
            ctx.set_source_rgba(0, 0, 1, 0.1)
            ctx.rectangle(im_base_x, im_base_y, im_sect_size, im_sect_size)
            ctx.fill()

        # load sector file
        with open(path, "rb") as f:
            while True:
                entType = f.read(1)
                if not entType or entType == b"\x00":
                    break
                elif entType == b"\x01":  # WorldMesh
                    wmid, = struct.unpack("<H", f.read(2))
                    print("WorldMesh", wmid)

                    with open(f"dist/tolcl/world/{wmid}.mesh") as wmf:
                        print(wmf.readline().strip())
                        type = wmf.readline().strip()

                        if type == "heightmap":
                            source = wmf.readline().strip()
                            texture = wmf.readline().strip()
                            x = float(wmf.readline())
                            y = float(wmf.readline())
                            z0 = float(wmf.readline())
                            w = float(wmf.readline())
                            h = float(wmf.readline())
                            range_ = float(wmf.readline())
                            print("HeightMap", source, texture, x, y, z0, w, h, range_)

                            heightmap_pil = PIL.ImageOps.invert(
                                Image.open("dist/" + source)
                            )
                            heightmap = pil2cairo(heightmap_pil)
                            h_w = heightmap.get_width()
                            h_h = heightmap.get_height()
                            ctx.save()
                            ctx.translate(
                                im_base_x + (x - sect_base_x),
                                im_base_y + (y - sect_base_y),
                            )
                            ctx.scale(w / h_w, h / h_h)
                            ctx.set_source_surface(heightmap, 0, 0)
                            ctx.paint()
                            ctx.restore()
                        else:
                            print(f"Warning: unknown worldmesh type {type}")
                elif entType == b"\x02":  # WorldObj
                    name = readsz(f)
                    x, y, o = struct.unpack("<fff", f.read(12))
                    print("WorldObj", name, x, y, o)

                    ctx.set_source_rgb(0, 0, 0)

                    def cross(ctx, x, y, sz=5):
                        ctx.move_to(x - sz, y)
                        ctx.line_to(x + sz, y)
                        ctx.move_to(x, y - sz)
                        ctx.line_to(x, y + sz)
                        ctx.stroke()

                    cross(ctx, im_base_x + x, im_base_y + y)

                    ctx.set_font_size(10)
                    ctx.move_to(im_base_x + x + 3, im_base_y + y - 3)
                    ctx.show_text(name.decode())
                else:
                    raise Exception(f"Unknown entry type {entType:02X}h")

surface.write_to_png("map.png")  # Output to PNG
