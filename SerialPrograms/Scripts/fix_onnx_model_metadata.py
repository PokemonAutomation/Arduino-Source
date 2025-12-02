#!/usr/bin/env python3
"""
Remove privacy-sensitive metadata (file paths in stack trace) from ONNX models.

Usage:
    python fix_onnx_model_metadata.py <input_onnx_file> [output_onnx_file]

If output file is not specified, it will overwrite the input file.
"""

import onnx
import sys
import os


def clean_onnx_metadata(model_path, output_path=None):
    """
    Remove privacy-sensitive metadata from an ONNX model.

    Args:
        model_path: Path to input ONNX model
        output_path: Path to save cleaned model (defaults to overwriting input)
    """
    print(f"Loading ONNX model from: {model_path}")
    model = onnx.load(model_path)

    # What we don't clean currently:
    # model.doc_string: top-level doc string
    # model.metadata_props: top-level metadata
    # model.graph.doc_string: graph doc string
    # model.graph.node[...].docstring: node's doc string
    # model.graph.initializer[...].metadata_props: intializer (tensor)'s metadata
    # model.functions[...].doc_string: function doc string

    # Track what we're cleaning
    changes_made = []
    removed_values = set()

    # Clean metadata_props from nodes (this is where PyTorch stores stack traces)
    node_metadata_cleaned = 0
    for node in model.graph.node:
        if not node.metadata_props:
            continue
        # Remove metadata containing paths or stack traces
        kept_props = []
        for prop in node.metadata_props:
            # Remove stack traces and name scopes that contain file paths
            if prop.key in ['pkg.torch.onnx.stack_trace', 'pkg.torch.onnx.name_scopes'] and ('/' in prop.value or '\\' in prop.value):
                removed_values.add(prop.value)
                node_metadata_cleaned += 1
                continue  # Skip this prop
            kept_props.append(prop)

        # Clear and re-add only kept props:
        # We can't directly assign value to node.metadat_props, otherwise will have following error:
        # "Assignment not allowed to map  or repeated field "metadata_props" in protocol message object."
        while len(node.metadata_props) > 0:
            node.metadata_props.pop()
        for prop in kept_props:
            node.metadata_props.append(prop)

    if node_metadata_cleaned > 0:
        print(f"  Removed {node_metadata_cleaned} metadata_props entries from nodes")
        changes_made.append(f"{node_metadata_cleaned} node metadata_props")

    # Verify the model is still valid
    print("  Verifying cleaned model...")
    try:
        onnx.checker.check_model(model)
        print("  ✓ Model validation passed")
    except Exception as e:
        print(f"  ✗ Model validation failed: {e}")
        print("  Aborting - model may be corrupted")
        return False

    # Save the cleaned model
    if output_path is None:
        output_path = model_path

    print(f"Saving cleaned model to: {output_path}")
    onnx.save(model, output_path)

    # Report what was cleaned
    if changes_made:
        print(f"\nCleaned metadata:")
        for change in changes_made:
            print(f"  - {change}")
        
        print(f"Removed {len(removed_values)} values")
    else:
        print("\nNo privacy-sensitive filepath metadata found")

    # Verify the paths are gone
    print("\nVerifying paths are removed...")
    with open(output_path, 'rb') as f:
        content = f.read()
        # Check for common path indicators
        if b'/Users/' in content or b'C:\\' in content or b'/home/' in content:
            print("  ⚠ Warning: Some path-like strings may still be present")
            print("    (This could be in tensor names or other non-metadata)")
        else:
            print("  ✓ No obvious file paths detected in binary")

    print(f"\n✓ Successfully cleaned ONNX model!")
    return True


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        print("\nError: No input file specified")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.exists(input_path):
        print(f"Error: Input file not found: {input_path}")
        sys.exit(1)

    success = clean_onnx_metadata(input_path, output_path)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
