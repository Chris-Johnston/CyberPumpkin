# this code was used for the http version
# and is no longer used

import base64
from PIL import Image

path = "doot.bmp" # testing

image = Image.open(path)
image = image.convert('1')

# bytes will read left to right on each row
# top to bottom
pa = image.load()

DIM_X = 128
DIM_Y = 64

image_bytes = []

for y in range(64):
    for x in range(0, 128, 8):
        byte = 0
        for b in range(8):
            coord = (x + b, y)
            val = pa[coord] & 1 # only want one 1 bit
            byte = byte << 1
            byte |= val
        image_bytes.append(byte)

print("bytes: ", image_bytes)

image_bytearr = bytearray(image_bytes)
result = base64.b64encode(image_bytearr)
# print("result:", result)

import requests
url = 'http://10.1.1.190'
data = "5" + result.decode("utf-8")

# print("data", data)

from urllib3.util.retry import Retry
retries = Retry(total=1)
from requests.adapters import HTTPAdapter

s = requests.Session()
s.mount("http://", HTTPAdapter(max_retries=retries))

x = s.post(url, data=data)
print(x.status_code)
print(x)
# print(x.text)