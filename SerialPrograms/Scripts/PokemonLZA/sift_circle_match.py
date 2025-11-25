#!/usr/bin/env python3
"""
SIFT-based image matching on circular regions.
Reads two images, extracts circular areas from top-left, and computes SIFT matches.
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


def extract_circular_region(image, center_x, center_y, radius):
    """Extract a circular region from the image."""
    # Create mask
    mask = create_circular_mask(image, center_x, center_y, radius)

    # Apply mask
    masked_image = cv2.bitwise_and(image, image, mask=mask)

    # Crop to bounding box
    x_min = max(0, center_x - radius)
    x_max = min(image.shape[1], center_x + radius)
    y_min = max(0, center_y - radius)
    y_max = min(image.shape[0], center_y + radius)

    cropped = masked_image[y_min:y_max, x_min:x_max]
    cropped_mask = mask[y_min:y_max, x_min:x_max]

    return cropped, cropped_mask, (x_min, y_min)


def compute_sift_matches(img1, mask1, img2, mask2, contrast_threshold=0.02,
                         edge_threshold=15, ratio_threshold=0.8, use_bfmatcher=False):
    """Compute SIFT features and matches between two images."""
    # Initialize SIFT detector with relaxed parameters for more features
    sift = cv2.SIFT_create(
        contrastThreshold=contrast_threshold,  # Lower = more keypoints (default 0.04)
        edgeThreshold=edge_threshold,          # Higher = more edge features (default 10)
        nOctaveLayers=3,                       # Default, controls scale space
        sigma=1.6                              # Default
    )

    # Detect and compute keypoints and descriptors
    kp1, desc1 = sift.detectAndCompute(img1, mask1)
    kp2, desc2 = sift.detectAndCompute(img2, mask2)

    print(f"Image 1: Found {len(kp1)} keypoints")
    print(f"Image 2: Found {len(kp2)} keypoints")

    if desc1 is None or desc2 is None or len(kp1) < 2 or len(kp2) < 2:
        print("Not enough keypoints found for matching")
        return None, None, None

    # Match features
    if use_bfmatcher:
        # BFMatcher - more thorough but slower
        bf = cv2.BFMatcher(cv2.NORM_L2, crossCheck=False)
        matches = bf.knnMatch(desc1, desc2, k=2)
        print("Using BFMatcher (slower but more thorough)")
    else:
        # FLANN-based matcher - faster
        FLANN_INDEX_KDTREE = 1
        index_params = dict(algorithm=FLANN_INDEX_KDTREE, trees=5)
        search_params = dict(checks=100)  # Increased from 50 for more thorough search
        flann = cv2.FlannBasedMatcher(index_params, search_params)
        matches = flann.knnMatch(desc1, desc2, k=2)
        print("Using FLANN matcher (faster)")

    # Apply Lowe's ratio test
    good_matches = []
    for match_pair in matches:
        if len(match_pair) == 2:
            m, n = match_pair
            if m.distance < ratio_threshold * n.distance:
                good_matches.append(m)

    print(f"Good matches (ratio < {ratio_threshold}): {len(good_matches)}")

    return kp1, kp2, good_matches


def estimate_transformation(kp1, kp2, good_matches, ransac_threshold=8.0):
    """Estimate the translation-only transformation between two sets of matched keypoints."""
    if len(good_matches) < 1:
        print("Not enough matches to estimate transformation")
        return None, None

    # Extract matched point coordinates
    src_pts = np.float32([kp1[m.queryIdx].pt for m in good_matches])
    dst_pts = np.float32([kp2[m.trainIdx].pt for m in good_matches])

    # Compute translation vectors for each match
    translations = dst_pts - src_pts

    # Use RANSAC-like approach to find inliers
    # Compute median translation as initial estimate
    median_tx = np.median(translations[:, 0])
    median_ty = np.median(translations[:, 1])
    median_translation = np.array([median_tx, median_ty])

    print(f"Initial median translation: ({median_tx:.2f}, {median_ty:.2f}) pixels")

    # Find inliers: matches whose translation is within threshold of median
    distances = np.sqrt(np.sum((translations - median_translation)**2, axis=1))
    inlier_mask = distances < ransac_threshold

    num_inliers = np.sum(inlier_mask)
    print(f"Inliers: {num_inliers}/{len(good_matches)} (within {ransac_threshold:.1f} pixels)")

    if num_inliers == 0:
        print("No inliers found - all matches are inconsistent")
        return None, None

    # Compute final translation from inliers
    inlier_translations = translations[inlier_mask]
    mean_tx = np.mean(inlier_translations[:, 0])
    mean_ty = np.mean(inlier_translations[:, 1])
    std_tx = np.std(inlier_translations[:, 0])
    std_ty = np.std(inlier_translations[:, 1])

    print("\n=== Estimated Translation (Translation-Only Model) ===")
    print(f"Translation: ({mean_tx:.2f}, {mean_ty:.2f}) pixels")
    print(f"Std deviation: ({std_tx:.2f}, {std_ty:.2f}) pixels")
    print(f"Translation range X: [{np.min(inlier_translations[:, 0]):.2f}, {np.max(inlier_translations[:, 0]):.2f}]")
    print(f"Translation range Y: [{np.min(inlier_translations[:, 1]):.2f}, {np.max(inlier_translations[:, 1]):.2f}]")

    # Create transformation matrix for visualization (identity + translation)
    T = np.array([
        [1.0, 0.0, mean_tx],
        [0.0, 1.0, mean_ty],
        [0.0, 0.0, 1.0]
    ])

    print("\nTranslation Matrix:")
    print(T)

    # Convert mask to column vector for compatibility with visualization
    mask = inlier_mask.reshape(-1, 1).astype(np.uint8)

    return T, mask


def visualize_matches(img1, kp1, img2, kp2, good_matches, mask=None):
    """Create visualization of matched keypoints."""
    # Filter matches by mask if provided
    if mask is not None:
        matches_to_draw = [m for m, inlier in zip(good_matches, mask.ravel()) if inlier]
    else:
        matches_to_draw = good_matches

    # Draw matches
    draw_params = dict(
        matchColor=(0, 255, 0),
        singlePointColor=(255, 0, 0),
        matchesMask=None,
        flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS
    )

    result = cv2.drawMatches(img1, kp1, img2, kp2, matches_to_draw, None, **draw_params)

    return result


def main():
    parser = argparse.ArgumentParser(
        description='Perform SIFT matching on circular regions of two images',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
This script estimates TRANSLATION ONLY (no rotation or scale).
All matched keypoints vote for the best translation vector using RANSAC.

Tuning tips:
  - More keypoints: Lower --contrast-threshold (try 0.01) and raise --edge-threshold (try 20)
  - More matches: Increase --ratio-threshold (try 0.85-0.9)
  - More inliers: Increase --ransac-threshold (try 10-15)
  - Low contrast images: Use --equalize for histogram equalization
  - Use --bfmatcher for more thorough (but slower) matching

Example: python sift_circle_match.py img1.jpg img2.jpg --contrast-threshold 0.01 --ratio-threshold 0.85
        '''
    )
    parser.add_argument('image1', help='Path to first image')
    parser.add_argument('image2', help='Path to second image')
    parser.add_argument('--center-x', type=int, default=200,
                        help='X coordinate of circle center (default: 200)')
    parser.add_argument('--center-y', type=int, default=200,
                        help='Y coordinate of circle center (default: 200)')
    parser.add_argument('--radius', type=int, default=150,
                        help='Radius of circular region (default: 150)')
    parser.add_argument('--output', default='sift_matches.png',
                        help='Output image path (default: sift_matches.png)')

    # SIFT tuning parameters
    parser.add_argument('--contrast-threshold', type=float, default=0.02,
                        help='SIFT contrast threshold - lower = more keypoints (default: 0.02, standard: 0.04)')
    parser.add_argument('--edge-threshold', type=float, default=15,
                        help='SIFT edge threshold - higher = more edge features (default: 15, standard: 10)')
    parser.add_argument('--ratio-threshold', type=float, default=0.8,
                        help='Lowe\'s ratio test threshold - higher = more matches (default: 0.8, standard: 0.7)')
    parser.add_argument('--ransac-threshold', type=float, default=8.0,
                        help='RANSAC threshold: max distance from median translation for inliers (default: 8.0 pixels)')
    parser.add_argument('--bfmatcher', action='store_true',
                        help='Use BFMatcher instead of FLANN (slower but more thorough)')
    parser.add_argument('--equalize', action='store_true',
                        help='Apply histogram equalization to improve contrast')

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
    print(f"\nSIFT Parameters:")
    print(f"  Contrast threshold: {args.contrast_threshold}")
    print(f"  Edge threshold: {args.edge_threshold}")
    print(f"  Ratio threshold: {args.ratio_threshold}")
    print(f"  RANSAC threshold: {args.ransac_threshold}")
    print()

    # Extract circular regions
    crop1, mask1, offset1 = extract_circular_region(img1, args.center_x, args.center_y, args.radius)
    crop2, mask2, offset2 = extract_circular_region(img2, args.center_x, args.center_y, args.radius)

    # Convert to grayscale for SIFT
    if len(crop1.shape) == 3:
        gray1 = cv2.cvtColor(crop1, cv2.COLOR_BGR2GRAY)
    else:
        gray1 = crop1

    if len(crop2.shape) == 3:
        gray2 = cv2.cvtColor(crop2, cv2.COLOR_BGR2GRAY)
    else:
        gray2 = crop2

    # Apply histogram equalization if requested
    if args.equalize:
        gray1 = cv2.equalizeHist(gray1)
        gray2 = cv2.equalizeHist(gray2)
        print("Applied histogram equalization")

    # Compute SIFT matches
    kp1, kp2, good_matches = compute_sift_matches(
        gray1, mask1, gray2, mask2,
        contrast_threshold=args.contrast_threshold,
        edge_threshold=args.edge_threshold,
        ratio_threshold=args.ratio_threshold,
        use_bfmatcher=args.bfmatcher
    )

    if good_matches is None or len(good_matches) == 0:
        print("\n!!! No matches found !!!")
        print("\nTry these adjustments:")
        print("  1. Lower --contrast-threshold to 0.01 or even 0.005")
        print("  2. Increase --edge-threshold to 20")
        print("  3. Increase --ratio-threshold to 0.85 or 0.9")
        print("  4. Try --equalize for histogram equalization")
        print("  5. Try --bfmatcher for more thorough matching")
        sys.exit(1)

    # Print suggestions if matches are low
    if len(good_matches) < 10:
        print("\n*** Warning: Very few matches found ***")
        print("Consider trying:")
        print(f"  --contrast-threshold {args.contrast_threshold / 2:.4f}")
        print(f"  --ratio-threshold {min(0.9, args.ratio_threshold + 0.05):.2f}")
        if not args.equalize:
            print("  --equalize")
        print()

    # Estimate transformation
    H, inlier_mask = estimate_transformation(kp1, kp2, good_matches,
                                              ransac_threshold=args.ransac_threshold)

    # Visualize matches
    result = visualize_matches(crop1, kp1, crop2, kp2, good_matches, inlier_mask)

    # Save result
    cv2.imwrite(args.output, result)
    print(f"\nVisualization saved to: {args.output}")
    if inlier_mask is not None:
        num_visualized = np.sum(inlier_mask)
        print(f"Note: Visualization shows {num_visualized} inlier matches (green lines)")
        print(f"      ({len(good_matches) - num_visualized} outliers excluded)")

    # Display result
    cv2.imshow('SIFT Matches', result)
    print("\nPress any key to close the visualization window...")
    cv2.waitKey(0)
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
