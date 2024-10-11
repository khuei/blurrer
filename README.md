# Image Blurring Program

This program can apply various blur methods to an input image using
convolution.

## Features

- Load images in PNG or JPEG format.
- Various blur methods
- Customizable blur strength
- Save the processed image in PNG or JPEG format.

## Supported Blur Methods

- [x] Gaussian
- [x] Box
- [x] Bilateral
- [x] Median
- [x] Motion

## Blur Method Comparison

| Blurring Method | Original Image | Blurred Image |
|----------------|---------------|------------------|
| Gaussian | ![Original Image](images/sample.jpg) | ![Blurred Image](images/gaussian.jpg) |
| Box | ![Original Image](images/sample.jpg) | ![Blurred Image](images/box.jpg) |
| Bilateral | ![Original Image](images/sample.jpg) | ![Blurred Image](images/bilateral.jpg) |
| Median | ![Original Image](images/sample.jpg) | ![Blurred Image](images/median.jpg) |
| Motion (Horizontal) | ![Original Image](images/sample.jpg) | ![Blurred Image](images/horizontal.jpg) |
| Motion (Vertical) | ![Original Image](images/sample.jpg) | ![Blurred Image](images/vertical.jpg) |
| Motion (Diagonal) | ![Original Image](images/sample.jpg) | ![Blurred Image](images/diagonal.jpg) |

## Dependencies

- [Boost](https://www.boost.org/) (specifically Boost.ProgramOptions)
- [stb_image](https://github.com/nothings/stb) for image loading and saving

## Installation

```
$ git clone https://github.com/khuei/blurrer.git
$ cd blurrer
$ make
$ make install
```

## Usage

```
$ blurrer --input <input_image> --output <output_image> [options]
```

Options:
- `-i`, `--input <input_image>`: Path to the input image file (required).
- `-o`, `--output <output_image>`: Path to save the output image file (required).
- `-a`, `--algo <algorithm>`: Set the algorithm for blurring (default: "gaussian").
- `-s`, `--strength <strength>`: Set the blur strength (default: 3).
- `-h`, `--help`: Display usage message.
