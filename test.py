import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider


data = np.fromfile('raw_data.bin', dtype=np.uint16).reshape(5552, 7424)


normalized = (data.astype(float) - data.min()) / (data.max() - data.min())

fig, ax = plt.subplots()
plt.subplots_adjust(bottom=0.25) 

img_display = ax.imshow(normalized, cmap='gray', vmin=0, vmax=1)
ax.set_title('Gamma preview')


ax_gamma = plt.axes([0.25, 0.1, 0.5, 0.03])
gamma_slider = Slider(ax_gamma, 'Gamma', 0.1, 3.0, valinit=1.0)

def update(val):
    gamma = gamma_slider.val
    corrected = np.power(normalized, 1.0 / gamma)
    img_display.set_data(corrected)
    fig.canvas.draw_idle()

gamma_slider.on_changed(update)

plt.show()