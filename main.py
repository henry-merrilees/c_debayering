import numpy as np
from PIL import Image
import math

# Redefine the circle function and dimensions as per the original script
WIDTH = 40
HEIGHT = 30


def circle(x, y, r):
    x -= WIDTH / 2
    y -= HEIGHT / 2
    return int(math.sqrt(x**2 + y**2) <= r) * 255


def gen_image(output_path):
    image_array = np.zeros((HEIGHT, WIDTH, 4), dtype=np.uint8)
    bw_image_array = np.zeros((HEIGHT, WIDTH, 4), dtype=np.uint8)

    # create if doesn't exist
    with open(output_path, "w") as f:
        # Generate the image as per the logic in the script
        for y in range(HEIGHT):
            for x in range(WIDTH):
                x_even = x % 2 == 0
                y_even = y % 2 == 0

                if x_even and y_even:  # Red
                    value = circle(x, y + 5, 10)

                    color = (value, 0, 0, 255)
                    bw = (value, value, value, 255)

                elif x_even ^ y_even:  # Green
                    value = circle(x - 5, y - 3, 10)

                    color = (0, value, 0, 255)
                    bw = (value, value, value, 255)
                else:  # Blue
                    value = circle(x + 5, y - 3, 10)

                    color = (value, 0, 0, 255)
                    bw = (value, value, value, 255)

                image_array[y, x] = color
                bw_image_array[y, x] = bw

                f.write(hex(value)[2:] + "\n")

    # Create an image from the array
    image = Image.fromarray(image_array)
    image.show()

    bw_image = Image.fromarray(bw_image_array)
    bw_image.show()


def load_image(input_path, output_path):
    # Open the image file
    with Image.open(input_path) as img:
        with open(output_path, "w") as f:
            # Ensure the image is in RGBA format (8-bit per channel)
            img = img.convert("RGBA")

            # Getting image dimensions
            width, height = img.size

            # Processing the image row by row, pixel by pixel
            for y in range(height):
                for x in range(width):
                    # Get the RGBA value of the pixel

                    r, g, b, a = img.getpixel((x, y))

                    x_even = x % 2 == 0
                    y_even = y % 2 == 0

                    if x_even and y_even:  # Red
                        value = r
                    elif x_even ^ y_even:  # Green
                        value = g
                    else:  # Blue
                        value = b

                    f.write(hex(value)[2:] + "\n")


def view_image_color(input_path):
    data = bytearray()
    with open(input_path, "r") as file:
        for x in file.read().split():
            four_bytes = bytes.fromhex(x)
            data += four_bytes

    image = Image.frombytes("RGBA", (WIDTH, HEIGHT), bytes(data))
    image.show()


def view_image_bw(input_path):
    pixels = []
    with open(input_path, "r") as file:
        for value in [int(x, 16) for x in file.read().split()]:
            pixels += 3 * [value]
            pixels.append(255)

    image_array = np.array(pixels, dtype=np.uint8).reshape((HEIGHT, WIDTH, 4))
    image = Image.fromarray(image_array)
    image.show()


def to_qoi(raw_bytes_file, qoi_file):
    with open(qoi_file, "wb") as q:
        # first 4 bytes are qoif
        # next 4, uint32 height (30)
        # next 4, uint32 width (40)
        # next byte, 4 for rgba
        # last byte, 1 for linear colorspace

        # header = bytearray(b"qoif") +

        header = bytes([0, 0, 0, 40, 0, 0, 0, 30, 4, 1])
        footer = bytearray([0, 0, 0, 0, 0, 0, 0, 1])
        q.write(b"qoif")
        q.write(header)
        print(header)
        with open(raw_bytes_file) as f:
            for line in f:
                print(line)
                data = bytearray([int(line.strip(), 16)])
                print(data)
                q.write(data)
        q.write(footer)


if __name__ == "__main__":
    import sys

    match sys.argv[1]:
        case "gen":
            if len(sys.argv) > 2:
                gen_image(sys.argv[2])
            else:
                print("Usage: python3 main.py gen <output_file>")
                exit()
        case "load":
            if len(sys.argv) > 3:
                load_image(sys.argv[2], sys.argv[3])
            else:
                print("Usage: python3 main.py load <input_image> <output_file>")
                exit()
        case "view_color":
            if len(sys.argv) > 2:
                view_image_color(sys.argv[2])
            else:
                print("Usage: python3 main.py view_color <color_bytes_file>")
                exit()
        case "view_bw":
            if len(sys.argv) > 2:
                view_image_bw(sys.argv[2])
            else:
                print("Usage: python3 main.py view_bw <bw_bytes_file>")
                exit()
        case "to_qoi":
            if len(sys.argv) > 3:
                to_qoi(sys.argv[2], sys.argv[3])
            else:
                print("Usage: python3 main.py to_qoi <raw_bytes_file> <qoi_file>")
                exit()
        case _:
            print("Usage: python3 main.py <gen|load|view_color|view_bw|to_qoi>")
