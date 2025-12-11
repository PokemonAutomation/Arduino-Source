#!/usr/bin/env python3
"""
Optical flow computation and visualization for motion detection between two images.
Supports both dense (Farneback) and sparse (Lucas-Kanade) optical flow methods.
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


def compute_dense_optical_flow(img1, img2, pyr_scale=0.5, levels=3, winsize=15,
                                iterations=3, poly_n=5, poly_sigma=1.2):
    """
    Compute dense optical flow using Farneback method.

    Args:
        img1, img2: Input images (grayscale)
        pyr_scale: Image scale for pyramid
        levels: Number of pyramid levels
        winsize: Averaging window size
        iterations: Number of iterations
        poly_n: Size of pixel neighborhood
        poly_sigma: Standard deviation of Gaussian for polynomial expansion

    Returns:
        flow: Optical flow array (height x width x 2), where flow[y,x] = [dx, dy]
    """
    # Convert to grayscale if needed
    if len(img1.shape) == 3:
        gray1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY)
    else:
        gray1 = img1.copy()

    if len(img2.shape) == 3:
        gray2 = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)
    else:
        gray2 = img2.copy()

    # Compute dense optical flow using Farneback method
    flow = cv2.calcOpticalFlowFarneback(
        gray1, gray2, None,
        pyr_scale=pyr_scale,
        levels=levels,
        winsize=winsize,
        iterations=iterations,
        poly_n=poly_n,
        poly_sigma=poly_sigma,
        flags=0
    )

    return flow


def compute_sparse_optical_flow(img1, img2, max_corners=200, quality_level=0.01,
                                 min_distance=10, block_size=7):
    """
    Compute sparse optical flow using Lucas-Kanade method.

    Args:
        img1, img2: Input images (grayscale)
        max_corners: Maximum number of corners to detect
        quality_level: Quality level for corner detection
        min_distance: Minimum distance between corners
        block_size: Block size for corner detection

    Returns:
        pts1: Points in first image (N x 2 array)
        pts2: Corresponding points in second image (N x 2 array)
        status: Status array indicating successful tracking
    """
    # Convert to grayscale if needed
    if len(img1.shape) == 3:
        gray1 = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY)
    else:
        gray1 = img1.copy()

    if len(img2.shape) == 3:
        gray2 = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)
    else:
        gray2 = img2.copy()

    # Detect good features to track in first image
    pts1 = cv2.goodFeaturesToTrack(
        gray1,
        maxCorners=max_corners,
        qualityLevel=quality_level,
        minDistance=min_distance,
        blockSize=block_size
    )

    if pts1 is None or len(pts1) == 0:
        return None, None, None

    # Calculate optical flow (track points from img1 to img2)
    pts2, status, err = cv2.calcOpticalFlowPyrLK(
        gray1, gray2, pts1, None,
        winSize=(15, 15),
        maxLevel=2,
        criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03)
    )

    # Select good points
    if pts2 is not None:
        pts1 = pts1[status.ravel() == 1]
        pts2 = pts2[status.ravel() == 1]

    return pts1, pts2, status


def visualize_dense_flow(img, flow, step=16, scale=1.0, threshold=0.5):
    """
    Visualize dense optical flow as arrows on the image.

    Args:
        img: Original image to draw on
        flow: Optical flow array
        step: Step size for drawing arrows (to avoid clutter)
        scale: Scale factor for arrow length
        threshold: Minimum flow magnitude to draw

    Returns:
        vis: Visualization image
    """
    h, w = img.shape[:2]

    # Convert to color if grayscale
    if len(img.shape) == 2:
        vis = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    else:
        vis = img.copy()

    # Create grid of points
    y, x = np.mgrid[step//2:h:step, step//2:w:step].reshape(2, -1).astype(int)

    # Get flow at these points
    fx, fy = flow[y, x].T

    # Compute magnitude
    mag = np.sqrt(fx*fx + fy*fy)

    # Draw arrows for flow vectors above threshold
    for i in range(len(x)):
        if mag[i] > threshold:
            pt1 = (x[i], y[i])
            pt2 = (int(x[i] + fx[i] * scale), int(y[i] + fy[i] * scale))
            cv2.arrowedLine(vis, pt1, pt2, (0, 255, 0), 1, tipLength=0.3)

    return vis


def visualize_flow_hsv(flow):
    """
    Visualize optical flow using HSV color coding.
    Hue represents direction, Saturation represents magnitude.

    Args:
        flow: Optical flow array

    Returns:
        vis: HSV visualization image (BGR)
    """
    h, w = flow.shape[:2]

    # Compute magnitude and angle
    mag, ang = cv2.cartToPolar(flow[..., 0], flow[..., 1])

    # Create HSV image
    hsv = np.zeros((h, w, 3), dtype=np.uint8)
    hsv[..., 0] = ang * 180 / np.pi / 2  # Hue: direction
    hsv[..., 1] = 255  # Saturation: full
    hsv[..., 2] = cv2.normalize(mag, None, 0, 255, cv2.NORM_MINMAX)  # Value: magnitude

    # Convert to BGR
    vis = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)

    return vis


def visualize_sparse_flow(img, pts1, pts2):
    """
    Visualize sparse optical flow as arrows between tracked points.

    Args:
        img: Original image to draw on
        pts1: Points in first image
        pts2: Corresponding points in second image

    Returns:
        vis: Visualization image
    """
    # Convert to color if grayscale
    if len(img.shape) == 2:
        vis = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    else:
        vis = img.copy()

    if pts1 is None or pts2 is None:
        return vis

    # Draw arrows from pts1 to pts2
    for p1, p2 in zip(pts1, pts2):
        pt1 = tuple(p1.ravel().astype(int))
        pt2 = tuple(p2.ravel().astype(int))

        # Draw the tracked point
        cv2.circle(vis, pt1, 3, (0, 255, 0), -1)

        # Draw the flow arrow
        cv2.arrowedLine(vis, pt1, pt2, (0, 0, 255), 2, tipLength=0.3)

    return vis


def compute_flow_statistics(flow):
    """
    Compute statistics about the optical flow.

    Args:
        flow: Optical flow array

    Returns:
        stats: Dictionary containing flow statistics
    """
    # Compute magnitude
    mag = np.sqrt(flow[..., 0]**2 + flow[..., 1]**2)

    # Compute statistics
    stats = {
        'mean_magnitude': float(np.mean(mag)),
        'median_magnitude': float(np.median(mag)),
        'max_magnitude': float(np.max(mag)),
        'mean_dx': float(np.mean(flow[..., 0])),
        'mean_dy': float(np.mean(flow[..., 1])),
        'std_dx': float(np.std(flow[..., 0])),
        'std_dy': float(np.std(flow[..., 1]))
    }

    return stats


def create_flow_visualization_panel(img1, img2, flow, pts1=None, pts2=None,
                                     step=16, scale=1.0, threshold=0.5, mask1=None, mask2=None):
    """
    Create a comprehensive visualization panel showing:
    - Original images side by side
    - Dense flow visualization (arrows)
    - HSV color-coded flow
    - Sparse flow (if available)

    Args:
        img1, img2: Input images
        flow: Dense optical flow
        pts1, pts2: Sparse flow points (optional)
        step, scale, threshold: Parameters for dense flow visualization
        mask1, mask2: Optional masks to visualize masked regions

    Returns:
        panel: Combined visualization image
    """
    h1, w1 = img1.shape[:2]
    h2, w2 = img2.shape[:2]

    # Ensure images are in color
    if len(img1.shape) == 2:
        img1_color = cv2.cvtColor(img1, cv2.COLOR_GRAY2BGR)
    else:
        img1_color = img1.copy()

    if len(img2.shape) == 2:
        img2_color = cv2.cvtColor(img2, cv2.COLOR_GRAY2BGR)
    else:
        img2_color = img2.copy()

    # Overlay masked regions (show in red tint where mask is 0)
    if mask1 is not None:
        masked_region = mask1 == 0
        img1_color[masked_region] = (img1_color[masked_region].astype(float) * 0.5 + np.array([0, 0, 50])).astype(np.uint8)

    if mask2 is not None:
        masked_region = mask2 == 0
        img2_color[masked_region] = (img2_color[masked_region].astype(float) * 0.5 + np.array([0, 0, 50])).astype(np.uint8)

    # Create visualizations
    dense_arrows = visualize_dense_flow(img2_color, flow, step, scale, threshold)
    hsv_vis = visualize_flow_hsv(flow)

    # Add labels
    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(img1_color, "Image 1 (Reference)", (10, 30), font, 0.7, (255, 255, 255), 2)
    cv2.putText(img2_color, "Image 2 (Target)", (10, 30), font, 0.7, (255, 255, 255), 2)
    cv2.putText(dense_arrows, "Dense Flow (Arrows)", (10, 30), font, 0.7, (255, 255, 255), 2)
    cv2.putText(hsv_vis, "Flow Direction (HSV)", (10, 30), font, 0.7, (255, 255, 255), 2)

    # Create top row: original images
    max_h_top = max(h1, h2)
    top_row = np.zeros((max_h_top, w1 + w2, 3), dtype=np.uint8)
    top_row[:h1, :w1] = img1_color
    top_row[:h2, w1:w1+w2] = img2_color

    # Create bottom row: flow visualizations
    max_h_bottom = max(dense_arrows.shape[0], hsv_vis.shape[0])
    bottom_row = np.zeros((max_h_bottom, w1 + w2, 3), dtype=np.uint8)
    bottom_row[:dense_arrows.shape[0], :dense_arrows.shape[1]] = dense_arrows
    bottom_row[:hsv_vis.shape[0], w1:w1+hsv_vis.shape[1]] = hsv_vis

    # Combine rows
    panel = np.vstack([top_row, bottom_row])

    # Add statistics
    stats = compute_flow_statistics(flow)
    info_y = panel.shape[0] - 100

    cv2.putText(panel, f"Mean magnitude: {stats['mean_magnitude']:.2f} px",
                (10, info_y), font, 0.6, (0, 255, 255), 2)
    cv2.putText(panel, f"Max magnitude: {stats['max_magnitude']:.2f} px",
                (10, info_y + 25), font, 0.6, (0, 255, 255), 2)
    cv2.putText(panel, f"Mean motion: ({stats['mean_dx']:.2f}, {stats['mean_dy']:.2f}) px",
                (10, info_y + 50), font, 0.6, (0, 255, 255), 2)

    # Add sparse flow count if available
    if pts1 is not None and pts2 is not None:
        cv2.putText(panel, f"Tracked points: {len(pts1)}",
                    (w1 + 10, info_y), font, 0.6, (0, 255, 255), 2)

    return panel


def create_sparse_flow_visualization(img1, img2, pts1, pts2, mask1=None, mask2=None):
    """
    Create visualization for sparse optical flow.

    Args:
        img1, img2: Input images
        pts1, pts2: Tracked points
        mask1, mask2: Optional masks to visualize masked regions

    Returns:
        vis: Sparse flow visualization
    """
    h1, w1 = img1.shape[:2]
    h2, w2 = img2.shape[:2]

    # Convert to color if needed and apply mask overlays
    if len(img1.shape) == 2:
        img1_vis = cv2.cvtColor(img1, cv2.COLOR_GRAY2BGR)
    else:
        img1_vis = img1.copy()

    if len(img2.shape) == 2:
        img2_vis = cv2.cvtColor(img2, cv2.COLOR_GRAY2BGR)
    else:
        img2_vis = img2.copy()

    # Overlay masked regions (show in red tint where mask is 0)
    if mask1 is not None:
        masked_region = mask1 == 0
        img1_vis[masked_region] = (img1_vis[masked_region].astype(float) * 0.5 + np.array([0, 0, 50])).astype(np.uint8)

    if mask2 is not None:
        masked_region = mask2 == 0
        img2_vis[masked_region] = (img2_vis[masked_region].astype(float) * 0.5 + np.array([0, 0, 50])).astype(np.uint8)

    # Visualize on both images
    sparse_vis1 = visualize_sparse_flow(img1_vis, pts1, pts2)
    sparse_vis2 = visualize_sparse_flow(img2_vis, pts1, pts2)

    # Add labels
    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(sparse_vis1, "Sparse Flow (Image 1)", (10, 30), font, 0.7, (255, 255, 255), 2)
    cv2.putText(sparse_vis2, "Sparse Flow (Image 2)", (10, 30), font, 0.7, (255, 255, 255), 2)

    # Combine side by side
    max_h = max(h1, h2)
    vis = np.zeros((max_h, w1 + w2, 3), dtype=np.uint8)
    vis[:h1, :w1] = sparse_vis1
    vis[:h2, w1:w1+w2] = sparse_vis2

    # Add statistics
    if pts1 is not None and pts2 is not None:
        flow_vectors = (pts2 - pts1).reshape(-1, 2)
        mean_flow = np.mean(flow_vectors, axis=0)
        max_flow = np.max(np.linalg.norm(flow_vectors, axis=1))

        info_y = max_h - 80
        cv2.putText(vis, f"Points tracked: {len(pts1)}",
                    (10, info_y), font, 0.6, (0, 255, 255), 2)
        cv2.putText(vis, f"Mean flow: ({mean_flow[0]:.2f}, {mean_flow[1]:.2f}) px",
                    (10, info_y + 25), font, 0.6, (0, 255, 255), 2)
        cv2.putText(vis, f"Max flow: {max_flow:.2f} px",
                    (10, info_y + 50), font, 0.6, (0, 255, 255), 2)

    return vis


def main():
    parser = argparse.ArgumentParser(
        description='Optical flow computation and visualization',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Optical flow detects motion between two images:
  - Dense flow (Farneback): Computes flow for every pixel, good for understanding overall motion
  - Sparse flow (Lucas-Kanade): Tracks specific feature points, faster and good for feature-based tracking

Dense flow is better for:
  - Understanding full motion field
  - Visualizing complex motion patterns
  - Working with textured images

Sparse flow is better for:
  - Tracking specific features/corners
  - Real-time applications (faster)
  - Working with images that have distinct features

Examples:
  # Compute dense flow
  python optical_flow.py img1.png img2.png --method dense

  # Compute sparse flow
  python optical_flow.py img1.png img2.png --method sparse

  # Compute both and show all visualizations
  python optical_flow.py img1.png img2.png --method both --show-all

  # Compute flow on circular region only
  python optical_flow.py img1.png img2.png --center-x 300 --center-y 250 --radius 200

  # Mask out cursor at center
  python optical_flow.py img1.png img2.png --radius 200 --cursor-radius 20
        '''
    )
    parser.add_argument('image1', help='Path to first image (reference)')
    parser.add_argument('image2', help='Path to second image (target)')
    parser.add_argument('--method', choices=['dense', 'sparse', 'both'], default='both',
                        help='Optical flow method (default: both)')
    parser.add_argument('--center-x', type=int, default=180,
                        help='X coordinate of circle center (default: 180)')
    parser.add_argument('--center-y', type=int, default=180,
                        help='Y coordinate of circle center (default: 180)')
    parser.add_argument('--radius', type=int, default=120,
                        help='Radius of circular region (default: 120)')
    parser.add_argument('--cursor-radius', type=int, default=0,
                        help='Radius of cursor at center to mask out (default: 0, no masking)')
    parser.add_argument('--output', default='optical_flow_result.png',
                        help='Output image path (default: optical_flow_result.png)')
    parser.add_argument('--step', type=int, default=16,
                        help='Step size for dense flow arrows (default: 16)')
    parser.add_argument('--scale', type=float, default=2.0,
                        help='Scale factor for arrow length (default: 2.0)')
    parser.add_argument('--threshold', type=float, default=0.5,
                        help='Minimum flow magnitude to visualize (default: 0.5)')
    parser.add_argument('--max-corners', type=int, default=200,
                        help='Maximum corners for sparse flow (default: 200)')
    parser.add_argument('--show-all', action='store_true',
                        help='Show all visualizations in separate windows')

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
    print(f"Method: {args.method}")
    print(f"Circle center: ({args.center_x}, {args.center_y}), radius: {args.radius}")
    if args.cursor_radius > 0:
        print(f"Cursor masking: radius {args.cursor_radius} pixels at center")
    print()

    # Extract circular regions
    img1, mask1 = extract_circular_region(
        img1, args.center_x, args.center_y, args.radius, args.cursor_radius
    )
    img2, mask2 = extract_circular_region(
        img2, args.center_x, args.center_y, args.radius, args.cursor_radius
    )

    print(f"Cropped region 1 size: {img1.shape[1]}x{img1.shape[0]}")
    print(f"Cropped region 2 size: {img2.shape[1]}x{img2.shape[0]}")
    print()

    # Compute optical flow based on method
    flow = None
    pts1, pts2 = None, None

    if args.method in ['dense', 'both']:
        print("Computing dense optical flow (Farneback)...")
        flow = compute_dense_optical_flow(img1, img2)

        # Compute and print statistics
        stats = compute_flow_statistics(flow)
        print("\n" + "="*50)
        print("=== Dense Optical Flow Statistics ===")
        print("="*50)
        print(f"Mean magnitude: {stats['mean_magnitude']:.2f} pixels")
        print(f"Median magnitude: {stats['median_magnitude']:.2f} pixels")
        print(f"Max magnitude: {stats['max_magnitude']:.2f} pixels")
        print(f"Mean motion: ({stats['mean_dx']:.2f}, {stats['mean_dy']:.2f}) pixels")
        print(f"Std deviation: ({stats['std_dx']:.2f}, {stats['std_dy']:.2f}) pixels")
        print("="*50)
        print()

    if args.method in ['sparse', 'both']:
        print("Computing sparse optical flow (Lucas-Kanade)...")
        pts1, pts2, status = compute_sparse_optical_flow(
            img1, img2,
            max_corners=args.max_corners
        )

        if pts1 is not None and pts2 is not None:
            flow_vectors = (pts2 - pts1).reshape(-1, 2)
            mean_flow = np.mean(flow_vectors, axis=0)
            max_flow = np.max(np.linalg.norm(flow_vectors, axis=1))

            print("\n" + "="*50)
            print("=== Sparse Optical Flow Statistics ===")
            print("="*50)
            print(f"Points tracked: {len(pts1)}")
            print(f"Mean flow: ({mean_flow[0]:.2f}, {mean_flow[1]:.2f}) pixels")
            print(f"Max flow: {max_flow:.2f} pixels")
            print("="*50)
            print()
        else:
            print("Warning: No features could be tracked")

    # Create visualizations
    if args.method == 'dense' and flow is not None:
        panel = create_flow_visualization_panel(
            img1, img2, flow,
            step=args.step, scale=args.scale, threshold=args.threshold,
            mask1=mask1, mask2=mask2
        )
        cv2.imwrite(args.output, panel)
        print(f"Dense flow visualization saved to: {args.output}")
        if args.cursor_radius > 0:
            print(f"Note: Masked regions (cursor) shown with red tint in visualization")

        # Always show the visualization
        cv2.namedWindow('Dense Optical Flow', cv2.WINDOW_NORMAL)
        cv2.imshow('Dense Optical Flow', panel)
        cv2.waitKey(1)  # Small delay to ensure window renders

    elif args.method == 'sparse' and pts1 is not None:
        sparse_vis = create_sparse_flow_visualization(img1, img2, pts1, pts2, mask1, mask2)
        cv2.imwrite(args.output, sparse_vis)
        print(f"Sparse flow visualization saved to: {args.output}")
        if args.cursor_radius > 0:
            print(f"Note: Masked regions (cursor) shown with red tint in visualization")

        # Always show the visualization
        cv2.namedWindow('Sparse Optical Flow', cv2.WINDOW_NORMAL)
        cv2.imshow('Sparse Optical Flow', sparse_vis)
        cv2.waitKey(1)  # Small delay to ensure window renders

    elif args.method == 'both':
        # Create main panel with dense flow
        if flow is not None:
            panel = create_flow_visualization_panel(
                img1, img2, flow, pts1, pts2,
                step=args.step, scale=args.scale, threshold=args.threshold,
                mask1=mask1, mask2=mask2
            )
            cv2.imwrite(args.output, panel)
            print(f"Dense flow visualization saved to: {args.output}")
            if args.cursor_radius > 0:
                print(f"Note: Masked regions (cursor) shown with red tint in visualization")

        # Create sparse flow visualization
        if pts1 is not None:
            sparse_vis = create_sparse_flow_visualization(img1, img2, pts1, pts2, mask1, mask2)
            sparse_output = args.output.rsplit('.', 1)[0] + '_sparse.png'
            cv2.imwrite(sparse_output, sparse_vis)
            print(f"Sparse flow visualization saved to: {sparse_output}")
            if args.cursor_radius > 0:
                print(f"Note: Masked regions (cursor) shown with red tint in sparse flow visualization")

        # Show visualizations
        if args.show_all:
            # Show both in separate windows
            if flow is not None:
                cv2.namedWindow('Dense Optical Flow', cv2.WINDOW_NORMAL)
                cv2.imshow('Dense Optical Flow', panel)
                cv2.waitKey(1)  # Small delay to ensure window renders
            if pts1 is not None:
                cv2.namedWindow('Sparse Optical Flow', cv2.WINDOW_NORMAL)
                cv2.imshow('Sparse Optical Flow', sparse_vis)
                cv2.waitKey(1)  # Small delay to ensure window renders
        else:
            # Show combined panel by default
            if flow is not None:
                cv2.namedWindow('Optical Flow', cv2.WINDOW_NORMAL)
                cv2.imshow('Optical Flow', panel)
                cv2.waitKey(1)  # Small delay to ensure window renders

    print("\nPress any key to close the visualization window...")
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
