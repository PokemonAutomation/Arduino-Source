"""
Image icon processor that filters pixels by color range.
Sets pixels within specified color bounds to black with alpha = 0.
"""

import argparse
from PIL import Image
import sys


def parse_color(color_str: str) -> tuple[int, int , int]:
    """Parse color string in format 'R,G,B' to tuple (R, G, B)."""
    try:
        parts = color_str.split(',')
        if len(parts) != 3:
            raise ValueError(f"Color must have 3 components, got {len(parts)}")

        r, g, b = [int(x.strip()) for x in parts]

        if not all(0 <= val <= 255 for val in [r, g, b]):
            raise ValueError("Color values must be between 0 and 255")

        return (r, g, b)
    except Exception as e:
        raise ValueError(f"Invalid color format '{color_str}': {e}")


def is_within_bounds(pixel, lower_bound, upper_bound):
    """Check if pixel color is within the specified bounds."""
    r, g, b = pixel[:3]  # Only check RGB, ignore alpha if present
    lr, lg, lb = lower_bound
    ur, ug, ub = upper_bound

    return (lr <= r <= ur and
            lg <= g <= ug and
            lb <= b <= ub)


def process_image(
    input_path: str,
    lower_color: tuple[int, int, int],
    upper_color: tuple[int, int, int],
    inverse: bool,
    output_path: str
):
    """
    Process image by setting background pixels within color bounds to black with alpha = 0.

    Args:
        input_path: Path to input image
        lower_color: Lower bound color tuple (R, G, B)
        upper_color: Upper bound color tuple (R, G, B)
        output_path: Path to save processed image
    """
    try:
        # Load image
        img = Image.open(input_path)

        # Convert to RGBA if not already (to support alpha channel)
        if img.mode != 'RGBA':
            img = img.convert('RGBA')

        # Get pixel data
        pixels = img.load()
        width, height = img.size

        # Process each pixel
        count = 0
        for y in range(height):
            for x in range(width):
                pixel = pixels[x, y]
                within_bounds = is_within_bounds(pixel, lower_color, upper_color)
                if (not inverse and within_bounds) or (inverse and not within_bounds):
                    pixels[x, y] = (0, 0, 0, 0)  # Black with alpha = 0
                    count += 1

        # Save processed image
        img.save(output_path, 'PNG')
        print(f"Processed image saved to {output_path}")
        print(f"Modified {count} pixels (out of {width * height} total)")

    except FileNotFoundError:
        print(f"Error: Input file '{input_path}' not found", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error processing image: {e}", file=sys.stderr)
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description='Process image icon by removing background pixels within color bounds.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s input.png "255,0,0" "255,100,100" -o output.png
  %(prog)s icon.png "0,0,0" "50,50,50"
        '''
    )

    parser.add_argument('input', help='Path to input image')
    parser.add_argument('lower_color', help='Lower bound background color in format "R,G,B"')
    parser.add_argument('upper_color', help='Upper bound background color in format "R,G,B"')
    parser.add_argument('-i', '--inverse', action='store_true', help='inverse color range, defining the color outside of the bounds as background')
    parser.add_argument('-o', '--output', default='output.png',
                        help='Output path (default: output.png)')

    args = parser.parse_args()

    # Parse color values
    try:
        lower = parse_color(args.lower_color)
        upper = parse_color(args.upper_color)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    # Process the image
    process_image(args.input, lower, upper,  args.inverse, args.output)


if __name__ == '__main__':
    main()
