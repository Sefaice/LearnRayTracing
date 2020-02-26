import numpy as np
import matplotlib.pyplot as plt
import cv2

# Define width and height
w, h = 512, 512

# Read file using numpy "fromfile()"
with open('color_spp_4.fgg', mode='rb') as f:
    img_noisy = np.fromfile(f, dtype = np.uint8, count = w * h * 3).reshape(h, w, 3) / 255.
# origin at top left corner
print(img_noisy.shape)
plt.imshow(img_noisy)
plt.show()

with open('color_spp_8192.fgg', mode='rb') as f:
    img_gt = np.fromfile(f, dtype = np.uint8, count = w * h * 3).reshape(h, w, 3) / 255.
print(img_gt.shape)
plt.imshow(img_gt)
plt.show()

with open('normal_spp_128.fgg', mode='rb') as f:
    img_normal = np.fromfile(f, dtype = np.float32, count = w * h * 3).reshape(h, w, 3)
print(img_normal.shape)
plt.imshow(img_normal)
plt.show()

with open('worldpos_spp_128.fgg', mode='rb') as f:
    img_worldpos = np.fromfile(f, dtype = np.float32, count = w * h * 3).reshape(h, w, 3)
print(img_worldpos.shape)
plt.imshow(img_worldpos)
plt.show()