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


import cv2
import numpy as np

class ImageViewer:
	def __init__(self, image, highlight_list = []):
		self.image = image
		self.hsv_image = cv2.cvtColor(image, cv2.COLOR_RGB2HSV)
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
		self.nc = image.shape[2]  # num_channel
		# The size of the cross used to highlight a selected pixel
		self.cross_size = max(1, min(self.width, self.height) // 200)

	def _solid_color(self, color):
		return color if self.image.shape[2] == 3 else color + [255]

	def _set_color_to_buffer(self, coord, color):
		x, y = coord
		if x >= 0 and x < self.width and y >= 0 and y < self.height:
			self.buffer[y, x] = self._solid_color(color)

	def _render(self):
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
		print(f"Selected rect No.{self.cur_rect_index}/{len(self.rects)}: {rect}.")

	def _print_pixel(self, coord):
		p = self.image[coord[1], coord[0]]
		print(f"Pixel (x,y) = ({coord[0]}, {coord[1]}), ({coord[0]/self.width:0.3}, {coord[1]/self.height:0.3}), "
		+ f"rgb=[{str(p[3]) + ',' if self.nc == 4 else ''}{p[2]},{p[1]},{p[0]}] hsv={self.hsv_image[coord[1], coord[0]]}")

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

	def _move_crop(self, o0, o1, o2, o3):
		if self.cur_crop_index >= 0 and self.cur_crop_index < len(self.crops):
			self.crops[self.cur_crop_index][0] += o0
			self.crops[self.cur_crop_index][1] += o1
			self.crops[self.cur_crop_index][2] += o2
			self.crops[self.cur_crop_index][3] += o3
			self._render()

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
						self.cur_rect_index = len(self.crops) - 1
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

if __name__ == '__main__':
	import sys
	assert len(sys.argv) == 2

	filename = sys.argv[1]

	image = cv2.imread(filename, cv2.IMREAD_UNCHANGED)

	height = image.shape[0]
	width = image.shape[1]
	print(f"Load image from {filename}, size: {width} x {height}")
	viewer = ImageViewer(image)
	viewer.run()
