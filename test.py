from PIL import Image
import numpy as np
import matplotlib.pyplot as plt


tiff_img = Image.open('raw_mosaic.tiff')
tiff_matrix = np.array(tiff_img)

print("TIFF shape:", tiff_matrix.shape)
print("TIFF dtype:", tiff_matrix.dtype)
print("TIFF min:", tiff_matrix.min(), "max:", tiff_matrix.max())


plt.imshow(tiff_matrix, cmap='gray')
plt.title('Raw Bayer mosaic (from TIFF)')
plt.colorbar()
plt.show()


bin_data = np.fromfile('raw_data.bin', dtype=np.uint16).reshape(3024, 4032)

if np.array_equal(bin_data, tiff_matrix):
    print("MATCH: TIFF and binary dump contain identical data")
else:
    print("MISMATCH: TIFF and binary dump differ")
    diff = np.abs(bin_data.astype(int) - tiff_matrix.astype(int))
    print("Max difference:", diff.max())