#!/usr/bin/env python3
"""
Phase correlation-based translation estimation for circular regions.
Ideal for flat-color images like game maps.
"""

import argparse
import cv2
import numpy as np
import sys


def create_circular_mask(image, center_x, center_y, radius):
    """Create a circular mask for the image."""
    mask = np.zeros(image.shape[:2], dtype=np.uint8)
    cv2.circle(mask, (center_x, center_y), radius, 255, -1)
    return mask


def extract_circular_region(image, center_x, center_y, radius, cursor_radius=0):
    """
    Extract a circular region from the image.

    Args:
        image: Input image
        center_x, center_y: Center of the circular region
        radius: Radius of the circular region
        cursor_radius: Radius of cursor at center to mask out (0 = no cursor masking)
    """
    # Create mask for the circular region
    mask = create_circular_mask(image, center_x, center_y, radius)

    # Mask out the cursor at the center if requested
    if cursor_radius > 0:
        # Create inverse mask for cursor (black circle at center)
        cursor_mask = create_circular_mask(image, center_x, center_y, cursor_radius)
        # Subtract cursor from the main mask
        mask = cv2.subtract(mask, cursor_mask)

    # Apply mask
    masked_image = cv2.bitwise_and(image, image, mask=mask)

    # Crop to bounding box
    x_min = max(0, center_x - radius)
    x_max = min(image.shape[1], center_x + radius)
    y_min = max(0, center_y - radius)
    y_max = min(image.shape[0], center_y + radius)

    cropped = masked_image[y_min:y_max, x_min:x_max]
    cropped_mask = mask[y_min:y_max, x_min:x_max]

    return cropped, cropped_mask


def compute_phase_correlation(img1, mask1, img2, mask2, use_hanning=True):
    """
    Compute translation between two images using phase correlation.

    Args:
        img1, img2: Input images (grayscale)
        mask1, mask2: Masks for the images (0 = masked out, 255 = valid)
                      Can include cursor masking at center
        use_hanning: Apply Hanning window to reduce edge effects

    Returns:
        (dx, dy): Translation in pixels
        response: Confidence of the match (higher is better)
    """
    # Convert to float32
    gray1 = np.float32(img1)
    gray2 = np.float32(img2)

    # Apply mask (set masked regions to mean value to reduce artifacts)
    if mask1 is not None:
        mean_val = np.mean(gray1[mask1 > 0])
        gray1 = gray1.copy()
        gray1[mask1 == 0] = mean_val

    if mask2 is not None:
        mean_val = np.mean(gray2[mask2 > 0])
        gray2 = gray2.copy()
        gray2[mask2 == 0] = mean_val

    # Apply Hanning window to reduce edge effects
    if use_hanning:
        hann = cv2.createHanningWindow((gray1.shape[1], gray1.shape[0]), cv2.CV_32F)
        gray1 = gray1 * hann
        gray2 = gray2 * hann

    # Phase correlation
    (dx, dy), response = cv2.phaseCorrelate(gray1, gray2)

    return (dx, dy), response


def visualize_translation(img1, img2, dx, dy, response, mask1=None, mask2=None):
    """
    Visualize the translation between two images.
    Shows both images side by side with translation overlay.

    Args:
        img1, img2: Input images
        dx, dy: Translation in pixels
        response: Confidence score
        mask1, mask2: Optional masks to visualize masked regions
    """
    h1, w1 = img1.shape[:2]
    h2, w2 = img2.shape[:2]

    # Create side-by-side visualization
    max_h = max(h1, h2)
    vis = np.zeros((max_h, w1 + w2, 3), dtype=np.uint8)

    # Place images
    if len(img1.shape) == 2:
        img1_color = cv2.cvtColor(img1, cv2.COLOR_GRAY2BGR)
    else:
        img1_color = img1.copy()

    if len(img2.shape) == 2:
        img2_color = cv2.cvtColor(img2, cv2.COLOR_GRAY2BGR)
    else:
        img2_color = img2.copy()

    vis[:h1, :w1] = img1_color
    vis[:h2, w1:w1+w2] = img2_color

    # Overlay masked regions (show in red tint where mask is 0)
    if mask1 is not None:
        # Create a red tint overlay for masked regions
        masked_region = mask1 == 0
        overlay_img = vis[:h1, :w1].copy()
        overlay_img[masked_region] = (overlay_img[masked_region].astype(float) * 0.5 + np.array([0, 0, 50])).astype(np.uint8)
        vis[:h1, :w1] = overlay_img

    if mask2 is not None:
        # Create a red tint overlay for masked regions
        masked_region = mask2 == 0
        overlay_img = vis[:h2, w1:w1+w2].copy()
        overlay_img[masked_region] = (overlay_img[masked_region].astype(float) * 0.5 + np.array([0, 0, 50])).astype(np.uint8)
        vis[:h2, w1:w1+w2] = overlay_img

    # Draw center markers
    center1 = (w1 // 2, h1 // 2)
    center2 = (w1 + w2 // 2, h2 // 2)

    cv2.drawMarker(vis, center1, (0, 255, 0), cv2.MARKER_CROSS, 20, 2)
    cv2.drawMarker(vis, center2, (0, 255, 0), cv2.MARKER_CROSS, 20, 2)

    # Draw translation vector from img1 center to corresponding point in img2
    # The translation (dx, dy) means img2 is shifted by (dx, dy) relative to img1
    # So a point at center1 in img1 corresponds to (center2 + translation) in img2's coordinate
    translated_point = (int(center2[0] + dx), int(center2[1] + dy))

    # Draw the translation vector
    cv2.arrowedLine(vis, center2, translated_point, (0, 0, 255), 2, tipLength=0.3)

    # Add text annotations
    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(vis, "Image 1", (10, 30), font, 1, (255, 255, 255), 2)
    cv2.putText(vis, "Image 2", (w1 + 10, 30), font, 1, (255, 255, 255), 2)

    # Translation info
    info_y = h1 - 80 if h1 == max_h else max_h - 80
    cv2.putText(vis, f"Translation: ({dx:.2f}, {dy:.2f}) px",
                (10, info_y), font, 0.7, (0, 255, 255), 2)
    cv2.putText(vis, f"Confidence: {response:.4f}",
                (10, info_y + 30), font, 0.7, (0, 255, 255), 2)

    # Create overlay visualization
    overlay = create_overlay_visualization(img1_color, img2_color, dx, dy)

    return vis, overlay


def create_overlay_visualization(img1, img2, dx, dy):
    """
    Create an overlay showing how well img2 aligns with img1 after translation.
    """
    h1, w1 = img1.shape[:2]
    h2, w2 = img2.shape[:2]

    # Create translation matrix
    M = np.float32([[1, 0, -dx], [0, 1, -dy]])

    # Warp img2 to align with img1
    img2_aligned = cv2.warpAffine(img2, M, (w1, h1))

    # Create overlay: img1 in red channel, img2_aligned in cyan
    overlay = np.zeros((h1, w1, 3), dtype=np.uint8)

    # Convert to grayscale if needed
    if len(img1.shape) == 3:
        gray1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY)
    else:
        gray1 = img1

    if len(img2_aligned.shape) == 3:
        gray2 = cv2.cvtColor(img2_aligned, cv2.COLOR_BGR2GRAY)
    else:
        gray2 = img2_aligned

    # Red channel: img1
    overlay[:, :, 2] = gray1
    # Cyan (green + blue): img2_aligned
    overlay[:, :, 0] = gray2
    overlay[:, :, 1] = gray2

    # Where they overlap perfectly, it will appear white/gray
    # Where they differ, you'll see red or cyan

    # Add text
    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(overlay, "Overlay: Red=Img1, Cyan=Img2 (aligned)",
                (10, 30), font, 0.6, (255, 255, 255), 2)
    cv2.putText(overlay, "Perfect match = White/Gray",
                (10, 55), font, 0.5, (255, 255, 255), 1)

    return overlay


def main():
    parser = argparse.ArgumentParser(
        description='Translation estimation using phase correlation on circular regions',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Phase correlation is ideal for:
  - Flat-color images (game maps, UI elements)
  - Translation-only motion
  - Fast processing with sub-pixel accuracy
  - No feature detection needed

The method works in frequency domain using FFT and is robust to uniform regions.

Use --cursor-radius to mask out a cursor or marker at the center of the circular region,
preventing it from affecting the phase correlation matching.

Examples:
  python phase_correlation_match.py img1.png img2.png --center-x 300 --center-y 250 --radius 200
  python phase_correlation_match.py img1.png img2.png --radius 200 --cursor-radius 20
        '''
    )
    parser.add_argument('image1', help='Path to first image')
    parser.add_argument('image2', help='Path to second image')
    parser.add_argument('--center-x', type=int, default=180,
                        help='X coordinate of circle center (default: 180)')
    parser.add_argument('--center-y', type=int, default=180,
                        help='Y coordinate of circle center (default: 180)')
    parser.add_argument('--radius', type=int, default=120,
                        help='Radius of circular region (default: 120)')
    parser.add_argument('--cursor-radius', type=int, default=50,
                        help='Radius of cursor at center to mask out (default: 50)')
    parser.add_argument('--output', default='phase_correlation_result.png',
                        help='Output image path (default: phase_correlation_result.png)')
    parser.add_argument('--no-hanning', action='store_true',
                        help='Disable Hanning window (may reduce accuracy)')
    parser.add_argument('--show-overlay', action='store_true',
                        help='Also show overlay visualization')

    args = parser.parse_args()

    # Read images
    img1 = cv2.imread(args.image1)
    img2 = cv2.imread(args.image2)

    if img1 is None:
        print(f"Error: Could not read image: {args.image1}")
        sys.exit(1)

    if img2 is None:
        print(f"Error: Could not read image: {args.image2}")
        sys.exit(1)

    print(f"Image 1 size: {img1.shape[1]}x{img1.shape[0]}")
    print(f"Image 2 size: {img2.shape[1]}x{img2.shape[0]}")
    print(f"Circle center: ({args.center_x}, {args.center_y}), radius: {args.radius}")
    if args.cursor_radius > 0:
        print(f"Cursor masking: radius {args.cursor_radius} pixels at center")
    print()

    # Extract circular regions
    crop1, mask1 = extract_circular_region(
        img1, args.center_x, args.center_y, args.radius, args.cursor_radius
    )
    crop2, mask2 = extract_circular_region(
        img2, args.center_x, args.center_y, args.radius, args.cursor_radius
    )

    print(f"Cropped region 1 size: {crop1.shape[1]}x{crop1.shape[0]}")
    print(f"Cropped region 2 size: {crop2.shape[1]}x{crop2.shape[0]}")

    # Convert to grayscale
    if len(crop1.shape) == 3:
        gray1 = cv2.cvtColor(crop1, cv2.COLOR_BGR2GRAY)
    else:
        gray1 = crop1

    if len(crop2.shape) == 3:
        gray2 = cv2.cvtColor(crop2, cv2.COLOR_BGR2GRAY)
    else:
        gray2 = crop2

    # Compute phase correlation
    print("Computing phase correlation...")
    (dx, dy), response = compute_phase_correlation(
        gray1, mask1, gray2, mask2,
        use_hanning=not args.no_hanning
    )

    print("\n" + "="*50)
    print("=== Translation Estimation (Phase Correlation) ===")
    print("="*50)
    print(f"Translation: ({dx:.2f}, {dy:.2f}) pixels")
    print(f"Confidence: {response:.6f}")
    print()
    print("Interpretation:")
    print(f"  Image 2 is shifted by ({dx:.2f}, {dy:.2f}) relative to Image 1")
    print(f"  To align Image 2 with Image 1, translate by ({-dx:.2f}, {-dy:.2f})")
    print()
    print(f"Confidence interpretation:")
    if response > 0.8:
        print(f"  Very high confidence - excellent match")
    elif response > 0.5:
        print(f"  Good confidence - reliable match")
    elif response > 0.3:
        print(f"  Moderate confidence - may have some differences")
    else:
        print(f"  Low confidence - images may be very different")
    print("="*50)
    print()

    # Visualize
    vis, overlay = visualize_translation(crop1, crop2, dx, dy, response, mask1, mask2)

    # Save result
    cv2.imwrite(args.output, vis)
    print(f"Visualization saved to: {args.output}")
    if args.cursor_radius > 0:
        print(f"Note: Masked regions (cursor) shown with red tint in visualization")

    if args.show_overlay:
        overlay_path = args.output.rsplit('.', 1)[0] + '_overlay.png'
        cv2.imwrite(overlay_path, overlay)
        print(f"Overlay visualization saved to: {overlay_path}")

    # Display result
    cv2.imshow('Phase Correlation - Translation', vis)

    if args.show_overlay:
        cv2.imshow('Overlay Visualization', overlay)

    print("\nPress any key to close the visualization window...")
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
