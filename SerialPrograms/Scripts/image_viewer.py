#!python3

"""
A simple image viewer (using OpenCV) with basic ability to check pixel location and color.
It can also draw boxes on the image and print their locations. Useful for writing 
PokemonAutomation visual inference methods.

Single left clicking on a pixel shows you the info of that pixel. Note it may also print the
alpha channel value.

- Press 'w', 's', 'a', 'd' to move the selected pixels around.

To draw a box: left click and drag the rectangle.
You can draw multiple boxes on the screen.

- Press 'i' to dump the information of those boxes so you can copy them into the code,
  or into check_detector_regions.py to fine tune the boxes.

- Press 'backspace/delete' to delete the current selected box. If no box is selected,
  delete the last added box. Select an existing box by right clicking.

- Press 'ESC' to exit the program.
"""


import argparse
import cv2
import numpy as np

class ImageViewer:
	def __init__(self, image: np.ndarray, highlight_list = []):
		if image.ndim == 2:
			# convert gray scale image to RGB image
			image = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)
		self.image = image  # bgr or bgra channel order
		self.nc = image.shape[2]  # num_channel
		if self.nc == 4:
			alpha_free_image = cv2.cvtColor(image, cv2.COLOR_BGRA2BGR)
		else:
			alpha_free_image = image
		self.hsv_image = cv2.cvtColor(alpha_free_image, cv2.COLOR_BGR2HSV)
		self.selected_pixel = (-1, -1)
		self.buffer = image.copy()
		self.window_name = 'image'
		self.height = image.shape[0]
		self.width = image.shape[1]
		self.highlight_list = highlight_list
		self.cur_highlight_index = -1
		self.rects = [] # List of Tuple[int,int,int,int]: each tuple: start_x, start_y, end_x, end_y
		self.cur_rect_index = -1
		self.mouse_down = False
		self.mouse_move_counter = 0
		# The size of the cross used to highlight a selected pixel
		self.cross_size = max(1, min(self.width, self.height) // 200)

	def _solid_color(self, color):
		return color if self.image.shape[2] == 3 else color + [255]

	def _set_color_to_buffer(self, coord, color):
		x, y = coord
		if x >= 0 and x < self.width and y >= 0 and y < self.height:
			self.buffer[y, x] = self._solid_color(color)

	def update_buffer(self) -> None:
		self.buffer = self.image.copy()
		if self.selected_pixel[0] >= 0 and self.selected_pixel[1] >= 0:
			p = self.selected_pixel
			# Draw a red cross
			self._set_color_to_buffer(p, color=[0, 0, 255])
			for i in range(1, self.cross_size):
				self._set_color_to_buffer((p[0]-i, p[1]), color=[0, 0, 255])
				self._set_color_to_buffer((p[0]+i, p[1]), color=[0, 0, 255])
				self._set_color_to_buffer((p[0], p[1]-i), color=[0, 0, 255])
				self._set_color_to_buffer((p[0], p[1]+i), color=[0, 0, 255])

		if self.cur_highlight_index >= 0 and len(self.highlight_list) > 0:
			for pixel in self.highlight_list[self.cur_highlight_index]:
				self.buffer[pixel[1], pixel[0]] = self._solid_color([255, 0, 0])

		for i, rect in enumerate(self.rects):
			# print(f"In render(): {rect}")
			# rect: [start_x start_y end_x end_y]
			width = 2
			color = (0, 0, 255) if i == self.cur_rect_index else (255, 0, 0)
			self.buffer = cv2.rectangle(self.buffer, (rect[0], rect[1]), (rect[2], rect[3]), color, width)

	def _render(self) -> None:
		self.update_buffer();
		cv2.imshow(self.window_name, self.buffer)
		# self.fullscreen = False

	def _change_selected_rect(self, x, y):
		if len(self.rects) == 0:
			return

		min_dist = 0
		for i, rect in enumerate(self.rects):
			min_x = min(rect[0], rect[2])
			max_x = max(rect[0], rect[2])
			min_y = min(rect[1], rect[3])
			max_y = max(rect[1], rect[3])
			dist_x = min_x - x if x <= min_x else (x - max_x if x >= max_x else 0)
			dist_y = min_y - y if y <= min_y else (y - max_y if y >= max_y else 0)
			dist = dist_x ** 2 + dist_y ** 2
			if i == 0 or dist < min_dist:
				min_dist = dist
				self.cur_rect_index = i
		print(f"Selected rect No.{self.cur_rect_index}/{len(self.rects)}: {self.rects[self.cur_rect_index]}.")

	def _print_pixel(self, coord):
		p = self.image[coord[1], coord[0]]
		msg = f"Pixel (x,y) = ({coord[0]}, {coord[1]}), ({coord[0]/self.width:0.3}, {coord[1]/self.height:0.3}), "
		if self.nc == 4:
			msg += f"argb=[{p[3]}, {p[2]}, {p[1]}, {p[0]}], {hex(((int(p[3])*256+int(p[2]))*256+int(p[1]))*256+int(p[0]))}"
		else:
			msg += f"rgb=[{p[2]}, {p[1]}, {p[0]}]"
		p = self.hsv_image[coord[1], coord[0]]
		msg += f", hsv=[{p[0]}, {p[1]}, {p[2]}]"
		print(msg)

	def _print_rect(self, i, rect):
		crop = self.image[rect[1]:rect[3], rect[0]:rect[2]].astype(float) / 255.0
		num_pixels = crop.shape[0] * crop.shape[1]
		# crop_sum shape: (4), 4 is channel count
		crop_sum = np.sum(crop, (0, 1))
		# crop_avg shape: (4), 4 is channel count
		crop_avg = crop_sum / num_pixels
		# crop_sqr_sum shape: (4), 4 is channel count
		crop_sqr_sum = np.sum(np.square(crop), (0, 1))
		
		crop_stddev = np.sqrt(  np.clip(crop_sqr_sum - (np.square(crop_sum) / num_pixels), 0, None) / (num_pixels-1))

		avg_sum = np.sum(crop_avg)
		avg_ratio = [1/3., 1/3., 1/3.] if avg_sum == 0. else crop_avg / avg_sum
		avg_ratio = np.array([avg_ratio[2], avg_ratio[1], avg_ratio[0]])
		x = rect[0] / self.width
		y = rect[1] / self.height
		w = (rect[2] - rect[0]) / self.width
		h = (rect[3] - rect[1]) / self.height

		stddev_sum = np.sum(crop_stddev)
		print(f"Rect No.{i}, ({x:.3f}, {y:.3f}, {w:.3f}, {h:.3f}) rgb ratio: {avg_ratio[0]:.3f}:{avg_ratio[1]:.3f}:{avg_ratio[2]:.3f}, stddev sum: {stddev_sum:.3g}")

	def _mouse_callback(self, event, x, y, flags, param):
		redraw = False

		# Dragging selected_pixel rectangle
		if event == cv2.EVENT_LBUTTONDOWN:
			self.mouse_down = True
			self.cur_rect_index = len(self.rects)
			self.rects.append([x, y, x, y])
			self.mouse_move_counter = 0
		elif event == cv2.EVENT_LBUTTONUP:
			self.mouse_down = False
			assert len(self.rects) > 0
			if self.rects[-1][0] == self.rects[-1][2] and self.rects[-1][1] == self.rects[-1][3]:
				self.cur_rect_index -= 1
				self.rects.pop()
				self._change_selected_rect(x, y)
				self._print_pixel((x, y))
				if self.selected_pixel != (x, y):
					self.selected_pixel = (x, y)
					redraw = True
			else:
				# sanitize the rect:
				if self.cur_rect_index >= 0 and self.cur_rect_index < len(self.rects):
					rect = self.rects[self.cur_rect_index]
					min_x = min(rect[0], rect[2])
					max_x = max(rect[0], rect[2])
					min_y = min(rect[1], rect[3])
					max_y = max(rect[1], rect[3])
					rect[0] = min_x
					rect[1] = min_y
					rect[2] = max_x
					rect[3] = max_y
			redraw = True
			mouse_move_counter = 0
		elif event == cv2.EVENT_RBUTTONUP: # right click to select existing rect to edit
			if len(self.rects) > 0:
				self._change_selected_rect(x, y)
				redraw = True
		elif event == cv2.EVENT_MOUSEMOVE and self.mouse_down:
			if self.cur_rect_index >= 0 and self.cur_rect_index < len(self.rects):
				old_loc = (self.rects[self.cur_rect_index][2], self.rects[self.cur_rect_index][3])
				if old_loc != (x, y):
					self.rects[self.cur_rect_index][2] = x
					self.rects[self.cur_rect_index][3] = y
					# if mouse_move_counter % 2 == 0:
					redraw = True
			self.mouse_move_counter += 1

		if redraw:
			self._render()

	def run(self):
		cv2.imshow(self.window_name, self.buffer)
		cv2.setWindowProperty(self.window_name, cv2.WND_PROP_TOPMOST, 1)
		cv2.setMouseCallback(self.window_name, self._mouse_callback)
		
		while True:
			key = cv2.waitKey(0)

			if key == 27: # esc
				cv2.destroyAllWindows()
				break
			elif key == 119: # w
				if self.selected_pixel[0] >= 0 and self.selected_pixel[1] >= 1:
					self.selected_pixel = (self.selected_pixel[0], self.selected_pixel[1]-1)
					self._print_pixel(self.selected_pixel)
			elif key == 115: # s
				if self.selected_pixel[0] >= 0 and self.selected_pixel[1] >= 0 and self.selected_pixel[1] + 1 < self.height:
					self.selected_pixel = (self.selected_pixel[0], self.selected_pixel[1]+1)
					self._print_pixel(self.selected_pixel)
			elif key == 97: # a
				if self.selected_pixel[0] >= 1 and self.selected_pixel[1] >= 0:
					self.selected_pixel = (self.selected_pixel[0]-1, self.selected_pixel[1])
					self._print_pixel(self.selected_pixel)
			elif key == 100: # d
				if self.selected_pixel[0] >= 0 and self.selected_pixel[1] >= 0 and self.selected_pixel[0] + 1 < self.width:
					self.selected_pixel = (self.selected_pixel[0]+1, self.selected_pixel[1])
					self._print_pixel(self.selected_pixel)
			elif key == 44: # ,
				if len(self.highlight_list) > 0:
					self.cur_highlight_index = len(self.highlight_list) - 1 if self.cur_highlight_index < 0 else self.cur_highlight_index-1
					print(f"Change highlight index to {self.cur_highlight_index}/{len(self.highlight_list)}")
			elif key == 46: # .
				if len(self.highlight_list) > 0:
					self.cur_highlight_index = -1 if self.cur_highlight_index == len(self.highlight_list)-1 else self.cur_highlight_index+1
					print(f"Change highlight index to {self.cur_highlight_index}/{len(self.highlight_list)}")
			elif key == 127 or key == 8: # DEL or backspace (BS), remove selected rectangle
				mouse_down = False
				mouse_move_counter = 0
				if len(self.rects) > 0:
					if self.cur_rect_index < 0:
						self.cur_rect_index = len(self.rects) - 1
					del self.rects[self.cur_rect_index]
					if len(self.rects) == 0:
						self.cur_rect_index = -1
					elif self.cur_rect_index >= len(self.rects):
						self.cur_rect_index = len(self.rects) - 1
			elif key == 105: # i
				for i, rect in enumerate(self.rects):
					self._print_rect(i, rect)
			else:
				print(f"Pressed key {key}")
			self._render()


def parse_box(box_str: str):
	"""
	Parse a box string in the format "start_x,start_y,width,height".
	All values should be floats between 0.0 and 1.0 (inclusive).
	Returns a tuple of (start_x, start_y, width, height).
	"""
	try:
		parts = box_str.split(',')
		if len(parts) != 4:
			raise ValueError(f"Box must have exactly 4 values, got {len(parts)}")

		values = [float(v) for v in parts]
		for i, v in enumerate(values):
			if not (0.0 <= v <= 1.0):
				raise ValueError(f"Value at position {i} ({v}) is not between 0.0 and 1.0")

		return tuple(values)
	except ValueError as e:
		raise argparse.ArgumentTypeError(f"Invalid box format '{box_str}': {e}")


if __name__ == '__main__':
	parser = argparse.ArgumentParser(
		description='Interactive image viewer with pixel inspection and box drawing capabilities.',
		epilog="""
Controls:
  Left click: Select pixel and show info
  Left drag: Draw a box
  Right click: Select existing box
  w/s/a/d: Move selected pixel
  i: Print info for all boxes
  Backspace/Delete: Delete selected box (or last box if none selected)
  ESC: Exit

Box format:
  Each box is specified as "start_x,start_y,width,height" where all values
  are floats between 0.0 and 1.0 (inclusive), representing normalized coordinates.
		""",
		formatter_class=argparse.RawDescriptionHelpFormatter
	)

	parser.add_argument(
		'image',
		help='Path to the image file to view'
	)

	parser.add_argument(
		'--box',
		action='append',
		type=parse_box,
		metavar='START_X,START_Y,WIDTH,HEIGHT',
		help='Add a box to render on the image. Format: "start_x,start_y,width,height" '
		     'where each value is a float between 0.0 and 1.0. Can be specified multiple times.'
	)

	args = parser.parse_args()

	# bgr or bgra channel order
	image = cv2.imread(args.image, cv2.IMREAD_UNCHANGED)
	if image is None:
		parser.error(f"Could not load image from '{args.image}'")

	height = image.shape[0]
	width = image.shape[1]
	print(f"Load image from {args.image}, size: {width} x {height}")

	viewer = ImageViewer(image)

	# Add boxes from command line arguments
	if args.box:
		for box in args.box:
			start_x, start_y, box_width, box_height = box
			# Convert normalized coordinates to absolute pixel coordinates
			abs_start_x = int(start_x * width)
			abs_start_y = int(start_y * height)
			abs_end_x = int((start_x + box_width) * width)
			abs_end_y = int((start_y + box_height) * height)

			# Add to viewer's rect list in the format [start_x, start_y, end_x, end_y]
			viewer.rects.append([abs_start_x, abs_start_y, abs_end_x, abs_end_y])
			print(f"Added box: ({start_x:.3f}, {start_y:.3f}, {box_width:.3f}, {box_height:.3f}) -> "
			      f"pixels ({abs_start_x}, {abs_start_y}, {abs_end_x}, {abs_end_y})")
		viewer.update_buffer()

	viewer.run()
