#include <boost/program_options.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

std::vector<std::vector<float>>
pad_image(const std::vector<std::vector<float>> &image, int pad_h, int pad_w) {
  int Hi = image.size();
  int Wi = image[0].size();
  int padded_Hi = Hi + 2 * pad_h;
  int padded_Wi = Wi + 2 * pad_w;

  std::vector<std::vector<float>> padded(padded_Hi,
                                         std::vector<float>(padded_Wi));

  for (int i = 0; i < padded_Hi; ++i) {
    for (int j = 0; j < padded_Wi; ++j) {
      int src_i = std::clamp(i - pad_h, 0, Hi - 1);
      int src_j = std::clamp(j - pad_w, 0, Wi - 1);
      padded[i][j] = image[src_i][src_j];
    }
  }

  return padded;
}

std::vector<std::vector<float>>
conv(const std::vector<std::vector<float>> &image,
     const std::vector<std::vector<float>> &kernel) {
  int Hi = image.size();
  int Wi = image[0].size();
  int Hk = kernel.size();
  int Wk = kernel[0].size();

  std::vector<std::vector<float>> out(Hi, std::vector<float>(Wi, 0));

  int pad_h = Hk / 2;
  int pad_w = Wk / 2;
  std::vector<std::vector<float>> padded = pad_image(image, pad_h, pad_w);

  for (int image_h = 0; image_h < Hi; ++image_h) {
    for (int image_w = 0; image_w < Wi; ++image_w) {
      float sum = 0;
      for (int kh = 0; kh < Hk; ++kh) {
        for (int kw = 0; kw < Wk; ++kw) {
          sum += kernel[kh][kw] * padded[image_h + kh][image_w + kw];
        }
      }
      out[image_h][image_w] = sum;
    }
  }

  return out;
}

std::vector<std::vector<float>> gaussian_kernel(int size, int strength) {
  std::vector<std::vector<float>> kernel(size, std::vector<float>(size, 0));

  double sigma = ((double)size / 2 > 1) ? (double)size / 2 : 1;
  int k = (size - 1) / 2;

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      double exponent =
          -((std::pow(i - k, 2) + std::pow(j - k, 2)) / (2 * sigma * sigma));
      kernel[i][j] = (1 / (2 * M_PI * sigma * sigma)) * std::exp(exponent);
    }
  }

  return kernel;
}

int main(int argc, char *argv[]) {
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()("input,i", boost::program_options::value<std::string>(),
                     "set input file name")(
      "output,o", boost::program_options::value<std::string>(),
      "set output file name")(
      "algo,a", boost::program_options::value<std::string>(), "set algorithm")(
      "strength,s", boost::program_options::value<std::string>(),
      "set blur strength")("help,h", "display usage message");

  if (argc == 1) {
    std::cout << desc << "\n";
    return 0;
  }

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  int width, height, channels;
  int strength = 5;
  unsigned char *image_data;
  std::string image_name;

  if (vm.count("input")) {
    image_name = vm["input"].as<std::string>();
    image_data = stbi_load(image_name.c_str(), &width, &height, &channels, 0);
  }

  if (vm.count("strength"))
    strength = vm["strength"].as<int>();

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  if (image_data == nullptr) {
    std::cerr << "Error: could not load image: " << image_name << std::endl;
    return 1;
  }

  std::vector<std::vector<float>> image(height, std::vector<float>(width));
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      image[i][j] = static_cast<float>(image_data[(i * width + j) * channels]);
    }
  }

  stbi_image_free(image_data);

  return 0;
}
