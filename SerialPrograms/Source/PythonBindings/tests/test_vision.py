"""Tests of the pure-Python vision helpers (opencv-python / pytesseract), no hardware."""

import io

import numpy as np
import pytest

pytest.importorskip("cv2")

from pokemon_automation import Frame, encode_image, ocr_image  # noqa: E402
from pokemon_automation.video import ocr_available  # noqa: E402


def make_image(width=64, height=48):
    image = np.zeros((height, width, 3), dtype=np.uint8)
    image[:, : width // 2] = (255, 0, 0)   # left half red
    image[:, width // 2:] = (0, 0, 255)    # right half blue
    return image


def test_encode_png_and_jpeg():
    image = make_image()
    assert encode_image(image, "png").startswith(b"\x89PNG")
    assert encode_image(image, "jpg").startswith(b"\xff\xd8")


def test_encode_rejects_bad_input():
    with pytest.raises(ValueError):
        encode_image(np.zeros((10, 10), dtype=np.uint8))
    with pytest.raises(ValueError, match="Unknown image format"):
        encode_image(make_image(), "gif")
    with pytest.raises(ValueError, match="fractions"):
        encode_image(make_image(), box=(0, 0, 2, 1))


def test_crop_and_scale():
    image = make_image()
    frame = Frame(image, 0, 1)
    assert (frame.crop((0.5, 0.0, 0.5, 1.0)) == (0, 0, 255)).all()
    # The PNG header (IHDR) holds the output width and height.
    small = encode_image(image, "png", box=(0.5, 0.0, 0.5, 1.0), max_width=16)
    assert (int.from_bytes(small[16:20], "big"), int.from_bytes(small[20:24], "big")) == (16, 24)


def test_encoded_colors_are_rgb():
    """Round-trip through the encoder must not swap red and blue."""
    pil = pytest.importorskip("PIL.Image")
    decoded = np.asarray(pil.open(io.BytesIO(encode_image(make_image(), "png"))).convert("RGB"))
    assert tuple(decoded[0, 0]) == (255, 0, 0)
    assert tuple(decoded[0, -1]) == (0, 0, 255)


def test_ocr_reads_rendered_text():
    if not ocr_available():
        pytest.skip("pytesseract or tesseract not installed")
    pil = pytest.importorskip("PIL.Image")
    from PIL import ImageDraw, ImageFont

    canvas = pil.new("RGB", (640, 120), (20, 20, 20))
    draw = ImageDraw.Draw(canvas)
    try:
        font = ImageFont.truetype("Arial.ttf", 48)
    except OSError:
        font = ImageFont.load_default(size=48)
    draw.text((20, 30), "Hello Switch 123", fill=(250, 250, 250), font=font)
    text = ocr_image(np.asarray(canvas), mode="line")
    assert "Switch" in text and "123" in text
