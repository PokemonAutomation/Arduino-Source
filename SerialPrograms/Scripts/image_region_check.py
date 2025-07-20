"""
Functions to check stats on image regions: what's the average color of this region and the std dev of the color of this region.
These stats is useful for image inferences like whether the current screen shows a pokemon battle menu.
The code here matches the implementation in C++, so that we can quickly prototype and debug image inference code in Python.
"""
import cv2
import sys
import numpy as np
from typing import Tuple, Union, List, NamedTuple
from dataclasses import dataclass

class Stats(NamedTuple):
	# RGB order in this class
	
	avg: np.ndarray
	
	avg_sum: float
	
	stddev: np.ndarray
	
	stddev_sum: float

	color_ratio: np.ndarray

	# Crop size: width, height
	crop_size: Tuple[int, int]

	def to_str(self) -> str:
		avg = self.avg
		ratio = self.color_ratio
		stdev = self.stddev
		return (
			f"RGB avg [{int(avg[0]+0.5)}, {int(avg[1]+0.5)}, {int(avg[2]+0.5)}]"
			+ f" avg sum {int(self.avg_sum+0.5)} ratio [{ratio[0]:.3f}, {ratio[1]:.3f}, {ratio[2]:.3f}]"
			+ f" stddev [{stdev[0]:.3f}, {stdev[1]:.3f}, {stdev[2]:.3f}] sum {self.stddev_sum:.3f}"
			+ f" crop size {self.crop_size}"
		)



def _get_region(image: np.ndarray, x: float, y: float, w: float, h: float) -> Tuple[int, int, int, int]:
	"""
	Given an image and (x, y, w, h) denoting a resolution-independent region, return
	the actual region in pixel counts.
	"""
	height = image.shape[0]
	width = image.shape[1]
	start_x = int(x * width + 0.5)
	start_y = int(y * height + 0.5)
	rect_width = int(w * width + 0.5)
	rect_height = int(h * height + 0.5)
	end_x = start_x + rect_width
	end_y = start_y + rect_height
	return start_x, start_y, end_x, end_y


def add_rect(image: np.ndarray, x: float, y: float, w: float, h: float, color=(0, 0, 255)) -> np.ndarray:
	"""
	Add a red rectangle around a region, where (x, y, w, h) denoting a resolution-independent region.
	"""
	start_x, start_y, end_x, end_y = _get_region(image, x, y, w, h)
	# print(start_x, start_y, end_x, end_y)
	image = cv2.rectangle(image, (start_x, start_y), (end_x, end_y), color, 2)
	return image

def _get_stats(image: np.ndarray, x: float, y: float, w: float, h: float) -> Stats:
	"""
	Given (x, y, w, h) denoting a resolution-independent region, return the color stats of the region.
	
	Return the average color of the region and the std dev of the region.
	"""
	start_x, start_y, end_x, end_y = _get_region(image, x, y, w, h)
	crop = image[start_y:end_y, start_x:end_x].astype(float)
	num_pixels = crop.shape[0] * crop.shape[1]
	crop_sum = np.sum(crop, (0, 1))
	crop_avg = crop_sum / num_pixels
	avg_sum = np.sum(crop_avg)
	crop_color_ratio = [1/3., 1/3., 1/3.] if avg_sum <= 1e-6 else crop_avg / avg_sum
	crop_color_ratio = np.array([crop_color_ratio[2], crop_color_ratio[1], crop_color_ratio[0]])

	crop_sqr_sum = np.sum(np.square(crop), (0, 1))
	crop_stddev = np.sqrt((crop_sqr_sum - (np.square(crop_sum) / num_pixels)) / (num_pixels-1))
	crop_stddev_sum = np.sum(crop_stddev)
	return Stats(
		avg = np.flip(crop_avg), # from BGR to RGB
		avg_sum = avg_sum,
		stddev = np.flip(crop_stddev), # from BGR to RGB
		stddev_sum = crop_stddev_sum,
		color_ratio = crop_color_ratio,
		crop_size=(end_x - start_x, end_y - start_y),
	)

def add_infer_box_to_image(
	image: np.ndarray,
	x: float, y: float, w: float, h: float,
	rendered_image: np.ndarray,
	color=(0,0,255)
) -> np.ndarray:
	stats = _get_stats(image, x, y, w, h)
	ratio = stats.color_ratio
	print(f"Add infer box: ({x:0.4f}, {y:0.4f}, {w:0.4f}, {h:0.4f}), {stats.to_str()}")
	return add_rect(rendered_image, x, y, w, h, color)

def _color_matched(
	crop_color_ratio: np.ndarray,
	crop_stddev_sum: float,
	expected_color_ratio: Union[np.ndarray, Tuple[float, float, float]],
	max_euclidean_distance: float,
	max_stddev_sum: float
) -> bool:
	"""Check whether an inference box check is matched"""
	dist = crop_color_ratio - np.array(expected_color_ratio)
	dist = np.square(dist)
	euclidean_distance = np.sqrt(np.sum(dist))

	# print(f"stddev_sum: {stddev_sum} vs max {max_stddev_sum}, rgb actual: {actual}, eu_dist: {euclidean_distance} vs max {max_euclidean_distance}")
	if crop_stddev_sum > max_stddev_sum:
		return False
	return euclidean_distance <= max_euclidean_distance


@dataclass
class RegionCheck:
	# Name of the region
	name: str
	# (x, y, w, h) denoting a resolution-independent region
	region: Tuple[float, float, float, float]
	# extpected the color ratio, all three values sum to one
	expected_color_ratio: Tuple[float, float, float]
	# threshold for euclidendistance between the color aspect ratio (size-3 vecctor) of
	# the current image and the expected ratio.
	max_euclidean_distance: float
	# threshold for sum of stddev from the three color channels.
	max_stddev_sum: float

	def check(self, image: np.ndarray) -> bool:
		"""Check whether this image passes inference check."""
		stats = _get_stats(image, *self.region)
		crop_ratio = stats.color_ratio
		crop_stddev_sum = stats.stddev_sum
		print(f"Checking {self.name}")
		print(stats.to_str())
		solid = _color_matched(crop_ratio, crop_stddev_sum, self.expected_color_ratio, self.max_euclidean_distance, self.max_stddev_sum)
		print(f"_color_matched? {solid}")
		return solid


def check_image(filename: str, regions: List[RegionCheck]) -> None:
	"""Print whether an image saved in filename passes all region checks."""
	image = cv2.imread(filename, cv2.IMREAD_UNCHANGED)
	height = image.shape[0]
	width = image.shape[1]
	print(f"Load image {filename}, size: {width} x {height}")

	for region in regions:
		if region.check(image) == False:
			print("====== Failed ======")
			return
	print("====== Passed ======")


def set_black_out_of_rect(image: np.ndarray, x: float, y: float, w: float, h: float):
	"""Set regions outside of the rect on `image` to be black."""
	height = image.shape[0]
	width = image.shape[1]
	start_x = int(x * width + 0.5)
	start_y = int(y * height + 0.5)
	rect_width = int(w * width + 0.5)
	rect_height = int(h * height + 0.5)
	end_x = start_x + rect_width
	end_y = start_y + rect_height
	
	new_image = np.zeros(image.shape, dtype=image.dtype)
	new_image[start_y:end_y, start_x:end_x] = image[start_y:end_y, start_x:end_x]
	image[...] = new_image[...]