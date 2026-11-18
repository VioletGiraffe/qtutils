# Renders a square SVG into a Windows .ico, a macOS .icns or a Linux .png, each size from the SVG itself so no frame is resampled from another.
# Requires Pillow and resvg-py. Run: python svg_to_icons.py <input.svg> <output.ico|output.icns|output.png>

import base64
import io
import struct
import sys
from pathlib import Path

import resvg_py
from PIL import Image

USAGE = 'Usage: python svg_to_icons.py <input.svg> <output.ico|output.icns|output.png>'

# Windows icon sizes by scaling, per Microsoft's "Construct your Windows app's icon":
#   context                        100%  125%  150%  200%  250%  300%  400%
#   title bar, context menu, tray    16    20    24    32    40    48    64
#   taskbar, search, Start list      24    30    36    48    60    72    96
#   Start pins                       32    40    48    64    80    96   256
# Windows uses an exact size when present, else downscales the next larger one; 256 is the largest, so it never upscales.
# 72, 80 and 96 are left to the 256 frame. 60 has a frame: Windows would otherwise downscale the 64 one for it.
ICO_SIZES = [16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 256]
ICO_PNG_MIN_SIZE = 256  # smaller frames are stored as uncompressed BMP

# ICNS entry types by pixel size, one per slot of a macOS icon set: an @2x type holds the same image as the 1x type of twice its point size.
ICNS_TYPES = {
	16: [b'ic04'],
	32: [b'ic05', b'ic11'],
	64: [b'ic12'],
	128: [b'ic07'],
	256: [b'ic08', b'ic13'],
	512: [b'ic09', b'ic14'],
	1024: [b'ic10'],
}
# These hold ARGB data, the rest PNG. macOS draws a PNG in a 16 or 32 px type garbled when the file is an application's icon.
ICNS_ARGB_TYPES = {b'ic04', b'ic05'}
# Apple's icon grid: the tile is 824 px of a 1024 px canvas, centred, so a full-bleed SVG is scaled into it.
MACOS_CANVAS = 1024
MACOS_TILE = 824

# The size the AppImage specification recommends for a PNG icon, and the size of a hicolor 256x256 icon
PNG_SIZE = 256


def render(svg: str, size: int) -> Image.Image:
	png = resvg_py.svg_to_bytes(svg_string=svg, width=size, height=size)
	image = Image.open(io.BytesIO(png)).convert('RGBA')
	if image.size != (size, size):
		sys.exit(f'The SVG must be square: it rendered at {image.width}x{image.height} for a {size} px frame')
	return image


def on_macos_grid(svg: str) -> str:
	"""A wrapper SVG that draws the given one, as an embedded image, in the tile of Apple's icon grid."""
	margin = (MACOS_CANVAS - MACOS_TILE) // 2
	data = base64.b64encode(svg.encode('utf-8')).decode('ascii')
	return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {MACOS_CANVAS} {MACOS_CANVAS}">'
		f'<image x="{margin}" y="{margin}" width="{MACOS_TILE}" height="{MACOS_TILE}" href="data:image/svg+xml;base64,{data}"/></svg>')


def encode_png(image: Image.Image) -> bytes:
	buffer = io.BytesIO()
	image.save(buffer, format='PNG', optimize=True)
	return buffer.getvalue()


def encode_bmp(image: Image.Image) -> bytes:
	"""ICO-style DIB: BITMAPINFOHEADER with doubled height, bottom-up BGRA pixels, then a 1 bpp AND mask."""
	width, height = image.size
	header = struct.pack('<IiiHHIIiiII', 40, width, 2 * height, 1, 32, 0, 0, 0, 0, 0, 0)
	pixels = image.transpose(Image.Transpose.FLIP_TOP_BOTTOM).tobytes('raw', 'BGRA')

	# Mask rows are padded to 32 bits; a set bit marks a transparent pixel.
	alpha = image.getchannel('A')
	stride = (width + 31) // 32 * 4
	mask = bytearray()
	for y in reversed(range(height)):
		row = bytearray(stride)
		for x in range(width):
			if alpha.getpixel((x, y)) == 0:
				row[x // 8] |= 0x80 >> (x % 8)
		mask += row
	return header + pixels + bytes(mask)


def write_ico(path: Path, frames: list[Image.Image]) -> None:
	blobs = [encode_png(f) if f.width >= ICO_PNG_MIN_SIZE else encode_bmp(f) for f in frames]
	directory = bytearray(struct.pack('<HHH', 0, 1, len(frames)))
	offset = len(directory) + 16 * len(frames)
	for frame, blob in zip(frames, blobs):
		# A width or height of 256 is stored as 0.
		directory += struct.pack('<BBBBHHII', frame.width % 256, frame.height % 256, 0, 0, 1, 32, len(blob), offset)
		offset += len(blob)
	path.write_bytes(bytes(directory) + b''.join(blobs))


def encode_icns_rle(plane: bytes) -> bytes:
	"""A control byte below 128 precedes control + 1 literal bytes; any other, one byte to repeat control - 125 times."""
	encoded = bytearray()
	position = 0
	while position < len(plane):
		run = 1
		while run < 130 and position + run < len(plane) and plane[position + run] == plane[position]:
			run += 1
		if run >= 3:
			encoded += bytes((run + 125, plane[position]))
			position += run
			continue

		# Literals end where a run of three starts
		start = position
		while position < len(plane) and position - start < 128 and plane[position:position + 3] != plane[position:position + 1] * 3:
			position += 1
		encoded += bytes((position - start - 1,)) + plane[start:position]
	return bytes(encoded)


def encode_argb(image: Image.Image) -> bytes:
	"""The tag 'ARGB', then the alpha, red, green and blue planes, each run-length encoded on its own."""
	red, green, blue, alpha = image.split()
	return b'ARGB' + b''.join(encode_icns_rle(plane.tobytes()) for plane in (alpha, red, green, blue))


def write_icns(path: Path, frames: dict[int, Image.Image]) -> None:
	"""The file has no table of contents, which is optional."""
	body = bytearray()
	for size, types in ICNS_TYPES.items():
		png = encode_png(frames[size])
		for icns_type in types:
			data = encode_argb(frames[size]) if icns_type in ICNS_ARGB_TYPES else png
			body += icns_type + struct.pack('>I', 8 + len(data)) + data
	path.write_bytes(b'icns' + struct.pack('>I', 8 + len(body)) + bytes(body))


def main() -> None:
	if len(sys.argv) != 3:
		sys.exit(USAGE)
	output = Path(sys.argv[2])
	svg = Path(sys.argv[1]).read_text(encoding='utf-8')
	match output.suffix.lower():
		case '.ico':
			write_ico(output, [render(svg, size) for size in ICO_SIZES])
		case '.icns':
			render(svg, 16)  # exits on a non-square SVG, which the grid's embedded image would letterbox
			tile = on_macos_grid(svg)
			write_icns(output, {size: render(tile, size) for size in ICNS_TYPES})
		case '.png':
			output.write_bytes(encode_png(render(svg, PNG_SIZE)))
		case _:
			sys.exit(USAGE)
	print(f'Wrote {output}')


if __name__ == '__main__':
	main()
