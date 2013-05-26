from __future__ import division, print_function

from PIL import Image


f = open("output_map", "r")
i = Image.new("RGB", (4147, 1200))

lines = f.readlines()
x = 0
y = 0
print(len(lines))
for line in lines:
    for char in line:
        #print(x, y)
        if char == "\n":
            continue
        elif char == "$":
            i.putpixel((x, y), (0, 0, 0))
        else:
            i.putpixel((x, y), (255, 255, 255))
        x += 1
    x = 0
    y += 1
i.save("map.png")
