from PIL import Image
import numpy as np
import argparse
import sys

# Parse command line arguments
parser = argparse.ArgumentParser(
    description='Transform image colors with linear scaling: [0,0,0] -> [255,255,255] and [255,255,255] -> target color'
)
parser.add_argument('input_image', type=str, help='Path to input image file')
parser.add_argument('--target-color', '-c', type=str, default='180,240,60',
                    help='Target RGB color for white pixels (format: R,G,B). Default: 180,240,60')
parser.add_argument('--output', '-o', type=str, default='output.png',
                    help='Output image path. Default: output.png')

args = parser.parse_args()

# Parse target color
try:
    target_rgb = [int(x.strip()) for x in args.target_color.split(',')]
    if len(target_rgb) != 3:
        raise ValueError("Target color must have exactly 3 values")
    if not all(0 <= v <= 255 for v in target_rgb):
        raise ValueError("RGB values must be between 0 and 255")
    target_r, target_g, target_b = target_rgb
except Exception as e:
    print(f"Error parsing target color: {e}", file=sys.stderr)
    print("Format should be: R,G,B (e.g., 180,240,60)", file=sys.stderr)
    sys.exit(1)

print(f"Input image: {args.input_image}")
print(f"Target color: RGB({target_r}, {target_g}, {target_b})")
print(f"Output image: {args.output}")

# Load the image
image_path = args.input_image
try:
    img = Image.open(image_path)
except Exception as e:
    print(f"Error loading image: {e}", file=sys.stderr)
    sys.exit(1)

# Convert to RGBA if not already
img = img.convert('RGBA')

# Convert to numpy array for easier manipulation
img_array = np.array(img, dtype=np.float32)

height, width = img_array.shape[0:2]
print(f"height {height} x width {width}")

# Extract RGBA channels
r, g, b, a = img_array[:, :, 0], img_array[:, :, 1], img_array[:, :, 2], img_array[:, :, 3]

# For pixels with alpha = 0, set RGB to 0
mask = (a == 0)
r[mask] = 0
g[mask] = 0
b[mask] = 0

# Linear scaling: [0, 0, 0] -> [255, 255, 255] and [255, 255, 255] -> [target_r, target_g, target_b]
# This creates an inverted color scheme with specific endpoints
# Formula: output = 255 - input * (255 - target) / 255

# Calculate scale factors
scale_r = (255.0 - target_r) / 255.0
scale_g = (255.0 - target_g) / 255.0
scale_b = (255.0 - target_b) / 255.0

print(f"Scale factors: R={scale_r:.4f}, G={scale_g:.4f}, B={scale_b:.4f}")

# Apply linear transformation
r = 255 - r * scale_r
g = 255 - g * scale_g
b = 255 - b * scale_b

# Clip values to valid range [0, 255]
r = np.clip(r, 0, 255)
g = np.clip(g, 0, 255)
b = np.clip(b, 0, 255)

# Reconstruct the image array
img_array[:, :, 0] = r
img_array[:, :, 1] = g
img_array[:, :, 2] = b
# Alpha channel remains unchanged

# Convert back to uint8
img_array = img_array.astype(np.uint8)

# Create new image from array
output_img = Image.fromarray(img_array, 'RGBA')

# Save the result
output_img.save(args.output)
print(f"Image processed and saved as {args.output}")
