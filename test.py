import io
import os
import re
import sys

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

path = sys.argv[1] if len(sys.argv) > 1 else "raw_data.bin"
notes_path = os.path.splitext(path)[0] + "_mathcad.txt"


def read_notes(name):
    values = {}
    if not os.path.exists(name):
        return values
    for line in io.open(name, encoding="utf-8"):
        found = re.match(r"(width|height|black level|white level)\s+(\d+)", line)
        if found:
            values[found.group(1)] = int(found.group(2))
    return values


def size_from_name(name):
    found = re.search(r"(\d+)x(\d+)", os.path.basename(name))
    return (int(found.group(1)), int(found.group(2))) if found else None


notes = read_notes(notes_path)
guess = size_from_name(path)

if "width" in notes and "height" in notes:
    width = notes["width"]
    height = notes["height"]
    black = notes.get("black level", 0)
    white = notes.get("white level", 0)
elif len(sys.argv) > 3:
    width = int(sys.argv[2])
    height = int(sys.argv[3])
    black = 0
    white = 0
elif guess:
    width, height = guess
    black = 0
    white = 0
else:
    raise SystemExit("no %s next to the file, run as: python test.py raw_data.bin WIDTH HEIGHT"
                     % os.path.basename(notes_path))

expected = width * height * 2
actual = os.path.getsize(path)

if actual != expected:
    raise SystemExit("size mismatch: %s has %d bytes, but %dx%d uint16 needs %d bytes"
                     % (path, actual, width, height, expected))

data = np.fromfile(path, dtype="<u2").reshape(height, width)

low = black if white > black else int(data.min())
high = white if white > black else int(data.max())

normalized = np.clip((data.astype(np.float32) - low) / max(1, high - low), 0.0, 1.0)

print("%s  %d x %d  min %d  max %d  mean %.1f"
      % (os.path.basename(path), width, height, data.min(), data.max(), data.mean()))

figure, axes = plt.subplots()
plt.subplots_adjust(bottom=0.25)

display = axes.imshow(normalized, cmap="gray", vmin=0, vmax=1)
axes.set_title("%s  %d x %d" % (os.path.basename(path), width, height))

slider_axes = plt.axes([0.25, 0.1, 0.5, 0.03])
gamma_slider = Slider(slider_axes, "Gamma", 0.1, 3.0, valinit=2.2)


def update(value):
    display.set_data(np.power(normalized, 1.0 / gamma_slider.val))
    figure.canvas.draw_idle()


gamma_slider.on_changed(update)
plt.show()
