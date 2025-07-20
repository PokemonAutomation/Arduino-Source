#!/usr/local/bin/python3

"""
A script used to fine-tune detection box placements and check the color stats from those boxes.
After using image_viewer.py to get an initial box placement, you can place the box values
(x, y, width, height) into this script, repeating the line
add_infer_box_to_image(raw_image, <x>, <y>, <width>, <height>, image) 
to add a box onto the image.

In the end of the script, it will call ImageViewer to render the image, with boxes added.
You can inspect the boxes to fine tune their positions.
"""

import cv2
import sys
import numpy as np
from typing import Tuple
from dataclasses import dataclass

from image_viewer import ImageViewer
from image_region_check import RegionCheck, add_rect, add_infer_box_to_image


assert len(sys.argv) == 2

filename = sys.argv[1]

image = cv2.imread(filename)
height = image.shape[0]
width = image.shape[1]
print(f"Size: {width} x {height}")

raw_image = image.copy()

# ==================================================================
# LA map weather symbol
# add_infer_box_to_image(raw_image, 0.028, 0.069, 0.025, 0.044, image)

# ==================================================================
# LA mult-pokemon battle sprite arrow
# for i in range(8):
# 	loc = (0.936 - 0.035*i, 0.018, 0.015, 0.027)
# 	add_infer_box_to_image(raw_image, *loc, image)

# ==================================================================
# LA battle sprite locations
# for i in range(8):
# 	loc = (0.957 - 0.035*i, 0.044, 0.021, 0.035)
# 	add_infer_box_to_image(raw_image, *loc, image)

# ==================================================================
# LA battle + details pokemon info
# add_infer_box_to_image(raw_image, 0.726, 0.133, 0.015, 0.023, image)
# add_infer_box_to_image(raw_image, 0.750, 0.133, 0.015, 0.023, image)
# add_infer_box_to_image(raw_image, 0.777, 0.138, 0.001, 0.015, image)
# add_infer_box_to_image(raw_image, 0.525, 0.130, 0.100, 0.038, image)

# ==================================================================
# LA wild pokemon focus reading pokemon info
# add_infer_box_to_image(raw_image, 0.108, 0.868, 0.135, 0.037, image)
# add_infer_box_to_image(raw_image, 0.307, 0.873, 0.016, 0.030, image)
# add_infer_box_to_image(raw_image, 0.307, 0.920, 0.016, 0.029, image)
# add_infer_box_to_image(raw_image, 0.324, 0.868, 0.023, 0.04, image)
# add_infer_box_to_image(raw_image, 0.244, 0.815, 0.026, 0.047, image)


# ==================================================================
# LA wild pokemon focus 
# add_infer_box_to_image(raw_image, 0.109, 0.857, 0.24, 0.012, image)
# add_infer_box_to_image(raw_image, 0.109, 0.949, 0.24, 0.012, image)
# add_infer_box_to_image(raw_image, 0.102, 0.875, 0.007, 0.073, image)
# add_infer_box_to_image(raw_image, 0.348, 0.873, 0.007, 0.073, image)

# ==================================================================
# LA battle start boundary lines
# add_infer_box_to_image(raw_image, 0.0, 0.113, 1.0, 0.015, image)
# add_infer_box_to_image(raw_image, 0.2, 0.871, 0.63, 0.015, image)


# ==================================================================
# LA map mission tab raised detection
# add_infer_box_to_image(raw_image, 0.9235, 0.617, 0.003, 0.019, image)
# add_infer_box_to_image(raw_image, 0.937, 0.62, 0.0035, 0.012, image)

# ==================================================================
# LA map zoom
# add_infer_box_to_image(raw_image, 0.780, 0.085, 0.008, 0.014, image)
# add_infer_box_to_image(raw_image, 0.795, 0.082, 0.010, 0.019, image)
# add_infer_box_to_image(raw_image, 0.807, 0.081, 0.014, 0.022, image)

# ==================================================================
# LA MMO Map question mark locations
# add_infer_box_to_image(raw_image, 0.362, 0.670, 0.045, 0.075, image)
# add_infer_box_to_image(raw_image, 0.683, 0.555, 0.039, 0.076, image)
# add_infer_box_to_image(raw_image, 0.828, 0.372, 0.042, 0.082, image)
# add_infer_box_to_image(raw_image, 0.485, 0.440, 0.044, 0.080, image)
# add_infer_box_to_image(raw_image, 0.393, 0.144, 0.050, 0.084, image)

# ==================================================================
# LA Map location detector
# add_infer_box_to_image(raw_image, 0.252, 0.400, 0.025, 0.150, image)
# add_infer_box_to_image(raw_image, 0.415, 0.550, 0.025, 0.150, image)
# add_infer_box_to_image(raw_image, 0.750, 0.570, 0.025, 0.150, image)
# add_infer_box_to_image(raw_image, 0.865, 0.240, 0.025, 0.150, image)
# add_infer_box_to_image(raw_image, 0.508, 0.320, 0.025, 0.150, image)
# add_infer_box_to_image(raw_image, 0.457, 0.060, 0.025, 0.150, image)
# add_infer_box_to_image(raw_image, 0.635, 0.285, 0.025, 0.150, image)


# ==================================================================
# LA dropped items detection
# add_infer_box_to_image(raw_image, 0.030, 0.177, 0.020, 0.038, image)
# add_infer_box_to_image(raw_image, 0.030, 0.225, 0.020, 0.038, image)
# add_infer_box_to_image(raw_image, 0.050, 0.177, 0.200, 0.038, image)
# add_infer_box_to_image(raw_image, 0.050, 0.177, 0.055, 0.038, image) # MMO ??????

# ==================================================================
# LA dropped items detection
# add_infer_box_to_image(raw_image, 0.439, 0.819, 0.029, 0.059, image)

# add_infer_box_to_image(raw_image, 0.031, 0.900, 0.024, 0.017, image)
# add_infer_box_to_image(raw_image, 0.026, 0.743, 0.021, 0.038, image)
# add_infer_box_to_image(raw_image, 0.022, 0.581, 0.020, 0.018, image)
# add_infer_box_to_image(raw_image, 0.152, 0.482, 0.017, 0.040, image)
# add_infer_box_to_image(raw_image, 0.016, 0.218, 0.018, 0.021, image)
# add_infer_box_to_image(raw_image, 0.169, 0.103, 0.024, 0.036, image)
# add_infer_box_to_image(raw_image, 0.815, 0.244, 0.026, 0.026, image)
# add_infer_box_to_image(raw_image, 0.737, 0.126, 0.045, 0.056, image)
# add_infer_box_to_image(raw_image, 0.806, 0.431, 0.043, 0.026, image)
# add_infer_box_to_image(raw_image, 0.827, 0.507, 0.023, 0.028, image)
# add_infer_box_to_image(raw_image, 0.747, 0.603, 0.087, 0.086, image)
# add_infer_box_to_image(raw_image, 0.885, 0.653, 0.041, 0.093, image)
# add_infer_box_to_image(raw_image, 0.934, 0.556, 0.039, 0.038, image)
# add_infer_box_to_image(raw_image, 0.761, 0.913, 0.071, 0.051, image)
# add_infer_box_to_image(raw_image, 0.865, 0.098, 0.022, 0.030, image)
# add_infer_box_to_image(raw_image, 0.202, 0.631, 0.057, 0.075, image)
# add_infer_box_to_image(raw_image, 0.099, 0.265, 0.072, 0.070, image)
# add_infer_box_to_image(raw_image, 0.908, 0.313, 0.020, 0.017, image)

# ==================================================================
# LA black out detection
# add_infer_box_to_image(raw_image, 0.068, 0.088, 0.864, 0.581, image)
# add_infer_box_to_image(raw_image, 0.720, 0.842, 0.028, 0.051, image)


# ==================================================================
# LA Transparent Dialogue Yellow arrow detection
# add_infer_box_to_image(raw_image, 0.720, 0.759, 0.049, 0.128, image)

# ==================================================================
# LA Dialogue Ellipse detection
# add_infer_box_to_image(raw_image, 0.741, 0.811, 0.028, 0.023, image)

# ==================================================================
# LA Tenacity path menu
# add_infer_box_to_image(raw_image, 0.56, 0.46, 0.33, 0.27, image)

# ==================================================================
# LA wild pokemon name in focus
# add_infer_box_to_image(raw_image, 0.11, 0.868, 0.135, 0.043, image)

# ==================================================================
# LA camp dialogue box
# add_infer_box_to_image(raw_image, 0.741, 0.811, 0.028, 0.023, image)

# ==================================================================
# LA item compatibility
# add_infer_box_to_image(raw_image, 0.838, 0.1815, 0.090, 0.024, image)

# ==================================================================
# LA battle move image match
# for i in range(4):
	# add_infer_box_to_image(raw_image, 0.66 - i * 0.0205, 0.622 + i * 0.0655, 0.25, 0.032, image)

# ==================================================================
# LA Ingo battle initial dialogue selection
# add_infer_box_to_image(raw_image, 0.50, 0.350, 0.40, 0.400, image)
# add_infer_box_to_image(raw_image, 0.50, 0.350, 0.40, 0.400, image)


# ==================================================================
# LA battle pokemon switch
# add_infer_box_to_image(raw_image, 0.641, 0.178, 0.050, 0.023, image)
# add_infer_box_to_image(raw_image, 0.641, 0.248, 0.050, 0.023, image)
# add_infer_box_to_image(raw_image, 0.517, 0.195, 0.011, 0.061, image)
# add_infer_box_to_image(raw_image, 0.924, 0.185, 0.019, 0.076, image)

# add_infer_box_to_image(raw_image, 0.540, 0.216, 0.016, 0.018, image)
# add_infer_box_to_image(raw_image, 0.676, 0.216, 0.016, 0.018, image)

# add_infer_box_to_image(raw_image, 0.044, 0.091, 0.043, 0.077, image)


# ==================================================================
# LA battle move selection
# for i in range(4):
	# add_infer_box_to_image(raw_image, 0.8 - i * 0.021, 0.622 + i * 0.0655, 0.02, 0.032, image)

# ==================================================================
# LA battle menu
# add_infer_box_to_image(raw_image, 0.056, 0.948, 0.013, 0.020, image)
# add_infer_box_to_image(raw_image, 0.174, 0.948, 0.032, 0.046, image)

# ==================================================================
# LA normal opaque dialogue
# add_infer_box_to_image(raw_image, 0.278, 0.712, 0.100, 0.005, image)
# add_infer_box_to_image(raw_image, 0.278, 0.755, 0.100, 0.005, image)
# add_infer_box_to_image(raw_image, 0.259, 0.715, 0.003, 0.043, image)
# add_infer_box_to_image(raw_image, 0.390, 0.715, 0.003, 0.043, image)

# add_infer_box_to_image(raw_image, 0.500, 0.750, 0.200, 0.020, image)
# add_infer_box_to_image(raw_image, 0.400, 0.895, 0.200, 0.020, image)
# add_infer_box_to_image(raw_image, 0.230, 0.805, 0.016, 0.057, image)
# add_infer_box_to_image(raw_image, 0.755, 0.805, 0.016, 0.057, image)

# ==================================================================
# LA normal surprise dialogue
# add_infer_box_to_image(raw_image, 0.295, 0.722, 0.100, 0.005, image)
# add_infer_box_to_image(raw_image, 0.295, 0.765, 0.100, 0.005, image)

# add_infer_box_to_image(raw_image, 0.500, 0.760, 0.200, 0.020, image)
# add_infer_box_to_image(raw_image, 0.400, 0.900, 0.200, 0.020, image)
# add_infer_box_to_image(raw_image, 0.720, 0.855, 0.030, 0.060, image)

# ==================================================================
# Pokemon Home Box Sorting

# , m_box_sprite(console, {0.228, 0.095, 0.030, 0.049}) // ball
# ImageFloatBox GENDER_BOX{0.417, 0.097, 0.031, 0.046}; // gender

# add_infer_box_to_image(raw_image, 0.495, 0.0045, 0.01, 0.005, image) # square color to check which mode is active
# add_infer_box_to_image(raw_image, 0.447, 0.250, 0.037, 0.034, image) # pokemon national dex number pos
# add_infer_box_to_image(raw_image, 0.702, 0.09, 0.04, 0.06, image) # shiny symbol pos
# add_infer_box_to_image(raw_image, 0.463, 0.09, 0.04, 0.06, image) # gmax symbol pos
# add_infer_box_to_image(raw_image, 0.623, 0.095, 0.033, 0.05, image) # origin symbol pos
# add_infer_box_to_image(raw_image, 0.69, 0.18, 0.28, 0.46, image) # pokemon render pos
# add_infer_box_to_image(raw_image, 0.228, 0.095, 0.030, 0.049, image) # ball type pos
# add_infer_box_to_image(raw_image, 0.417, 0.097, 0.031, 0.046, image) # gender pos
# add_infer_box_to_image(raw_image, 0.546, 0.099, 0.044, 0.041, image) # Level box
# add_infer_box_to_image(raw_image, 0.782, 0.719, 0.193, 0.046, image) # OT ID box
# add_infer_box_to_image(raw_image, 0.492, 0.719, 0.165, 0.049, image) # OT box
# add_infer_box_to_image(raw_image, 0.157, 0.783, 0.212, 0.042, image) # Nature box
# add_infer_box_to_image(raw_image, 0.158, 0.838, 0.213, 0.042, image) # Ability box

# ==================================================================
# Pokemon BDSP Poffin Cooking

# add_infer_box_to_image(raw_image, 0.56, 0.724, 0.012, 0.024, image) # green & blue arrow pos


viewer = ImageViewer(image)
viewer.run()

