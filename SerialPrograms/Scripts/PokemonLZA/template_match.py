#!/usr/bin/env python3
"""
Template matching-based translation estimation for circular regions.
Good for textured images with distinct features.
"""

import argparse
import cv2
import numpy as np
import sys
from typing import Tuple, Optional


def create_circular_mask(image: np.ndarray, center_x: int, center_y: int, radius: int) -> np.ndarray:
    """Create a circular mask for the image."""
    mask = np.zeros(image.shape[:2], dtype=np.uint8)
    cv2.circle(mask, (center_x, center_y), radius, 255, -1)
    return mask


def extract_circular_region(
    image: np.ndarray,
    center_x: int,
    center_y: int,
    radius: int,
    cursor_radius: int = 0
) -> Tuple[np.ndarray, np.ndarray]:
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


def compute_template_match(
    img1: np.ndarray,
    mask1: Optional[np.ndarray],
    img2: np.ndarray,
    mask2: Optional[np.ndarray],
    method: int = cv2.TM_CCOEFF_NORMED
) -> Tuple[Tuple[float, float], float, np.ndarray]:
    """
    Compute translation between two images using template matching.

    Args:
        img1: Template image (grayscale)
        img2: Source image to search in (grayscale)
        mask1: Mask for template (0 = masked out, 255 = valid)
        mask2: Mask for source image (0 = masked out, 255 = valid)
        method: Template matching method (default: cv2.TM_CCOEFF_NORMED)

    Returns:
        (dx, dy): Translation in pixels (from img1 center to matched location in img2)
        confidence: Confidence of the match (0-1, higher is better)
        result_map: Full matching result map for visualization
    """
    # Convert to uint8 if needed
    assert img1.dtype == np.uint8
    assert img2.dtype == np.uint8

    # Template matching with mask
    # XXX
    sub_zone_width = sub_zone_height = 30
    sub_zone_x = 10
    sub_zone_y = 120
    result = cv2.matchTemplate(img2, img1[sub_zone_y:sub_zone_y+sub_zone_height, sub_zone_x:sub_zone_x+sub_zone_width], cv2.TM_SQDIFF_NORMED)

    # Find the best match location
    min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(result)

    # For TM_CCOEFF_NORMED and TM_CCORR_NORMED, higher values are better
    # For TM_SQDIFF_NORMED, lower values are better
    match_loc = min_loc
    match_loc = (match_loc[0] - sub_zone_x, match_loc[1] - sub_zone_y)
    confidence = 1.0 - min_val
    print(f"Matching location: {match_loc}")
    
    # Calculate translation
    # The matched location is the top-left corner where template best matches
    # We want translation from center of img1 to center of matched region
    h1, w1 = img1.shape[:2]
    template_center_x = w1 // 2
    template_center_y = h1 // 2

    # Matched region center in img2
    matched_center_x = match_loc[0] + template_center_x
    matched_center_y = match_loc[1] + template_center_y

    # Center of img2
    h2, w2 = img2.shape[:2]
    img2_center_x = w2 // 2
    img2_center_y = h2 // 2

    # Translation from img1 center to matched location relative to img2 center
    dx = matched_center_x - img2_center_x
    dy = matched_center_y - img2_center_y

    return (dx, dy), confidence, result


def visualize_template_match(
    img1: np.ndarray,
    img2: np.ndarray,
    dx: float,
    dy: float,
    confidence: float,
    result_map: np.ndarray,
    mask1: Optional[np.ndarray] = None,
    mask2: Optional[np.ndarray] = None
) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """
    Visualize the template matching results.
    Shows both images side by side with match location overlay.

    Args:
        img1, img2: Input images
        dx, dy: Translation in pixels
        confidence: Confidence score
        result_map: Template matching result map
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

    # Draw matched region in img2
    # Calculate where the template matched
    matched_center_x = w1 + w2 // 2 + int(dx)
    matched_center_y = h2 // 2 + int(dy)
    matched_point = (matched_center_x, matched_center_y)

    # Draw the matched location
    cv2.drawMarker(vis, matched_point, (255, 0, 255), cv2.MARKER_STAR, 20, 2)

    # Draw translation vector from img2 center to matched location
    cv2.arrowedLine(vis, center2, matched_point, (0, 0, 255), 2, tipLength=0.3)

    # Add text annotations
    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(vis, "Template (Img1)", (10, 30), font, 1, (255, 255, 255), 2)
    cv2.putText(vis, "Source (Img2)", (w1 + 10, 30), font, 1, (255, 255, 255), 2)

    # Translation info
    info_y = h1 - 80 if h1 == max_h else max_h - 80
    cv2.putText(vis, f"Translation: ({dx:.2f}, {dy:.2f}) px",
                (10, info_y), font, 0.7, (0, 255, 255), 2)
    cv2.putText(vis, f"Confidence: {confidence:.4f}",
                (10, info_y + 30), font, 0.7, (0, 255, 255), 2)

    # Create overlay visualization
    overlay = create_overlay_visualization(img1_color, img2_color, dx, dy)

    # Create heatmap visualization of template matching result
    heatmap = create_heatmap_visualization(result_map, dx, dy, w2, h2)

    return vis, overlay, heatmap


def create_overlay_visualization(img1: np.ndarray, img2: np.ndarray, dx: float, dy: float) -> np.ndarray:
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


def create_heatmap_visualization(result_map: np.ndarray, dx: float, dy: float, w2: int, h2: int) -> np.ndarray:
    """
    Create a heatmap visualization of the template matching result.
    """
    # Normalize result map to 0-255
    norm_result = cv2.normalize(result_map, None, 0, 255, cv2.NORM_MINMAX, cv2.CV_8U)

    # Apply colormap
    heatmap = cv2.applyColorMap(norm_result, cv2.COLORMAP_JET)

    # Calculate matched location
    center2_x = w2 // 2
    center2_y = h2 // 2
    matched_x = int(center2_x + dx)
    matched_y = int(center2_y + dy)

    # Draw marker at best match location
    # The result map is smaller than the source image by (template_w-1, template_h-1)
    # So we need to adjust coordinates
    if 0 <= matched_y < result_map.shape[0] and 0 <= matched_x < result_map.shape[1]:
        cv2.drawMarker(heatmap, (matched_x, matched_y), (255, 255, 255),
                      cv2.MARKER_CROSS, 20, 2)

    # Add text
    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(heatmap, "Template Match Heatmap",
                (10, 30), font, 0.7, (255, 255, 255), 2)
    cv2.putText(heatmap, "Brighter = Better match",
                (10, 55), font, 0.5, (255, 255, 255), 1)

    return heatmap


def main() -> None:
    parser = argparse.ArgumentParser(
        description='Translation estimation using template matching on circular regions',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Template matching is ideal for:
  - Textured images with distinct features
  - When images have good contrast and detail
  - Finding specific patterns or objects
  - Works well with local features

The method slides the template over the source image and computes similarity.

Use --cursor-radius to mask out a cursor or marker at the center of the circular region,
preventing it from affecting the template matching.

Available matching methods:
  - ccoeff (default): Correlation coefficient (normalized, robust to brightness changes)
  - ccorr: Cross-correlation (sensitive to brightness)
  - sqdiff: Sum of squared differences (good for similar brightness)

Examples:
  python template_match.py img1.png img2.png --center-x 300 --center-y 250 --radius 200
  python template_match.py img1.png img2.png --radius 200 --cursor-radius 20
  python template_match.py img1.png img2.png --method ccorr
        '''
    )
    parser.add_argument('image1', help='Path to first image (template)')
    parser.add_argument('image2', help='Path to second image (source)')
    parser.add_argument('--center-x', type=int, default=180,
                        help='X coordinate of circle center (default: 180)')
    parser.add_argument('--center-y', type=int, default=180,
                        help='Y coordinate of circle center (default: 180)')
    parser.add_argument('--radius', type=int, default=120,
                        help='Radius of circular region (default: 120)')
    parser.add_argument('--cursor-radius', type=int, default=20,
                        help='Radius of cursor at center to mask out (default: 20)')
    parser.add_argument('--output', default='template_match_result.png',
                        help='Output image path (default: template_match_result.png)')
    parser.add_argument('--method', choices=['ccoeff', 'ccorr', 'sqdiff'], default='ccoeff',
                        help='Template matching method (default: ccoeff)')
    parser.add_argument('--show-overlay', action='store_true',
                        help='Also show overlay visualization')
    parser.add_argument('--show-heatmap', action='store_true',
                        help='Also show template matching heatmap')

    args = parser.parse_args()

    # Map method name to OpenCV constant
    method_map = {
        'ccoeff': cv2.TM_CCOEFF_NORMED,
        'ccorr': cv2.TM_CCORR_NORMED,
        'sqdiff': cv2.TM_SQDIFF_NORMED
    }
    method = method_map[args.method]

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
    print(f"Matching method: {args.method}")
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

    # Compute template matching
    print("Computing template matching...")
    (dx, dy), confidence, result_map = compute_template_match(
        crop1, mask1, crop2, mask2, method=method
    )

    print("\n" + "="*50)
    print("=== Translation Estimation (Template Matching) ===")
    print("="*50)
    print(f"Translation: ({dx:.2f}, {dy:.2f}) pixels")
    print(f"Confidence: {confidence:.6f}")
    print()
    print("Interpretation:")
    print(f"  Template (Image 1) best matches in Image 2 at offset ({dx:.2f}, {dy:.2f})")
    print(f"  To align Image 2 with Image 1, translate by ({-dx:.2f}, {-dy:.2f})")
    print()
    print(f"Confidence interpretation:")
    if confidence > 0.8:
        print(f"  Very high confidence - excellent match")
    elif confidence > 0.6:
        print(f"  Good confidence - reliable match")
    elif confidence > 0.4:
        print(f"  Moderate confidence - acceptable match")
    else:
        print(f"  Low confidence - images may be very different")
    print("="*50)
    print()

    # Visualize
    vis, overlay, heatmap = visualize_template_match(
        crop1, crop2, dx, dy, confidence, result_map, mask1, mask2
    )

    # Save result
    cv2.imwrite(args.output, vis)
    print(f"Visualization saved to: {args.output}")
    if args.cursor_radius > 0:
        print(f"Note: Masked regions (cursor) shown with red tint in visualization")

    if args.show_overlay:
        overlay_path = args.output.rsplit('.', 1)[0] + '_overlay.png'
        cv2.imwrite(overlay_path, overlay)
        print(f"Overlay visualization saved to: {overlay_path}")

    if args.show_heatmap:
        heatmap_path = args.output.rsplit('.', 1)[0] + '_heatmap.png'
        cv2.imwrite(heatmap_path, heatmap)
        print(f"Heatmap visualization saved to: {heatmap_path}")

    # Display result
    cv2.imshow('Template Matching - Translation', vis)

    if args.show_overlay:
        cv2.imshow('Overlay Visualization', overlay)

    if args.show_heatmap:
        cv2.imshow('Match Heatmap', heatmap)

    print("\nPress any key to close the visualization window...")
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
