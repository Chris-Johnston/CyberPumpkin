"""
Send image

Sends a 128x64 1bpp image to the pumpkin. Can be animated or static.
"""

# could I make this better with argparse? sure?
# will I? hahahahahahhaha no

from PIL import Image
from PIL.GifImagePlugin import GifImageFile

path = "doot.gif" # testing
path = "doot.bmp"
path = "images/smiley.bmp"

image = Image.open(path)

DIM_X = 128
DIM_Y = 64

image_bytes = []

num_frames = 1
if isinstance(image, GifImageFile):
    num_frames = image.n_frames

image_bytes.append(1) # mode
image_bytes.append(25) # animFrameTime
image_bytes.append(num_frames) # num frames

for frame in range(num_frames):
    if isinstance(image, GifImageFile):
        image.seek(frame)
    # bytes will read left to right on each row
    # top to bottom
    pa = image.load()
    for y in range(64):
        for x in range(0, 128, 8):
            byte = 0
            for b in range(8):
                coord = (x + b, y)
                val = pa[coord] & 1 # only want one 1 bit
                byte = byte << 1
                byte |= val
            image_bytes.append(byte)

import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('10.1.1.190', 8001))
s.send(bytearray(image_bytes))
s.close()
