from PIL import Image, ImageDraw, ImageFont

from dom_color import RGB


def _hex(color: RGB) -> str:
    return f"#{color.r:02X}{color.g:02X}{color.b:02X}"


def _text_color(background: RGB) -> tuple[int, int, int]:
    luminance = 0.299 * background.r + 0.587 * background.g + 0.114 * background.b
    return (0, 0, 0) if luminance > 140 else (255, 255, 255)


def comparison_tile(cover: Image.Image, expected: RGB, predicted: RGB,
                    size: int = 256, caption: str | None = None) -> Image.Image:
    """Cover on the left; expected (top) and predicted (bottom) swatches on the right.

    Tiles made with the same size and caption presence share dimensions, so they
    can be saved on their own or passed to collage().
    """
    font = ImageFont.load_default(size=max(10, size // 16))
    swatch_w, swatch_h = size // 2, size // 2
    caption_h = 2 * font.size if caption else 0

    tile = Image.new("RGB", (size + swatch_w, size + caption_h), (255, 255, 255))
    tile.paste(cover.convert("RGB").resize((size, size)), (0, 0))

    draw = ImageDraw.Draw(tile)
    pad = font.size // 2
    for row, (label, color) in enumerate([("spotify", expected), ("dom_color", predicted)]):
        top = row * swatch_h
        draw.rectangle([size, top, size + swatch_w - 1, top + swatch_h - 1],
                       fill=(color.r, color.g, color.b))
        draw.text((size + pad, top + pad), f"{label}\n{_hex(color)}",
                  fill=_text_color(color), font=font)

    if caption:
        draw.text((pad, size + font.size // 2), caption, fill=(0, 0, 0), font=font)
    return tile


def collage(tiles: list[Image.Image], columns: int = 4, gap: int = 8,
            background: tuple[int, int, int] = (255, 255, 255)) -> Image.Image:
    """Grid of tiles; each cell is as large as the largest tile."""
    if not tiles:
        raise ValueError("collage needs at least one tile")
    cell_w = max(t.width for t in tiles)
    cell_h = max(t.height for t in tiles)
    columns = min(columns, len(tiles))
    rows = -(-len(tiles) // columns)

    image = Image.new("RGB", (columns * cell_w + (columns + 1) * gap,
                              rows * cell_h + (rows + 1) * gap), background)
    for i, tile in enumerate(tiles):
        row, col = divmod(i, columns)
        image.paste(tile, (gap + col * (cell_w + gap), gap + row * (cell_h + gap)))
    return image
