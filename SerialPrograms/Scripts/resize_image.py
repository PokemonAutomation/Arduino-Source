#!/usr/bin/env python3
"""
Image resizing script using OpenCV.
Resizes an input image to specified width and height.
"""

import argparse
import sys
import cv2
import os


def main():
    parser = argparse.ArgumentParser(
        description='Resize an image using OpenCV'
    )

    parser.add_argument(
        'input',
        help='Input image path'
    )

    parser.add_argument(
        '-o', '--output',
        default='output.png',
        help='Output image path (default: output.png)'
    )

    parser.add_argument(
        '-r', '--resolution',
        type=int,
        nargs=2,
        required=True,
        metavar=('WIDTH', 'HEIGHT'),
        help='New resolution as width and height in pixels'
    )

    args = parser.parse_args()

    # Extract width and height from resolution
    width, height = args.resolution

    # Validate input file exists
    if not os.path.isfile(args.input):
        print(f"Error: Input file '{args.input}' not found.", file=sys.stderr)
        sys.exit(1)

    # Validate dimensions
    if width <= 0 or height <= 0:
        print(f"Error: Width and height must be positive integers.", file=sys.stderr)
        sys.exit(1)

    # Read the image
    img = cv2.imread(args.input)

    if img is None:
        print(f"Error: Failed to load image '{args.input}'. Please check if it's a valid image file.", file=sys.stderr)
        sys.exit(1)

    # Get original dimensions
    original_height, original_width = img.shape[:2]
    print(f"Original size: {original_width}x{original_height}")

    # Resize the image
    resized_img = cv2.resize(img, (width, height), interpolation=cv2.INTER_LANCZOS4)

    print(f"Resized to: {width}x{height}")

    # Save the resized image
    success = cv2.imwrite(args.output, resized_img)

    if success:
        print(f"Image saved to: {args.output}")
    else:
        print(f"Error: Failed to save image to '{args.output}'", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
