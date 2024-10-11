#include <boost/program_options.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define DEFAULT_STRENGTH 3
#define DEFAULT_SIGMA_RANGE 50.0
#define DEFAULT_SIGMA_SPACE 2.0
#define DEFAULT_ALGORITHM "gaussian"

#include "stb_image.h"
#include "stb_image_write.h"

std::vector<std::vector<std::vector<float>>>
pad_image(const std::vector<std::vector<std::vector<float>>> &image, int pad_h, int pad_w) {
  int Hi = image.size();
  int Wi = image[0].size();
  int channels = image[0][0].size();
  int padded_Hi = Hi + 2 * pad_h;
  int padded_Wi = Wi + 2 * pad_w;

  std::vector<std::vector<std::vector<float>>> padded(
      padded_Hi,
      std::vector<std::vector<float>>(padded_Wi, std::vector<float>(channels)));

  for (int i = 0; i < padded_Hi; ++i) {
    for (int j = 0; j < padded_Wi; ++j) {
      for (int c = 0; c < channels; ++c) {
        int src_i = std::clamp(i - pad_h, 0, Hi - 1);
        int src_j = std::clamp(j - pad_w, 0, Wi - 1);
        padded[i][j][c] = image[src_i][src_j][c];
      }
    }
  }

  return padded;
}

std::vector<std::vector<std::vector<float>>>
conv(const std::vector<std::vector<std::vector<float>>> &image,
     const std::vector<std::vector<std::vector<float>>> &kernel) {
  int Hi = image.size();
  int Wi = image[0].size();
  int channels = image[0][0].size();
  int Hk = kernel.size();
  int Wk = kernel[0].size();

  std::vector<std::vector<std::vector<float>>> out(
      Hi, std::vector<std::vector<float>>(Wi, std::vector<float>(channels, 0)));

  int pad_h = Hk / 2;
  int pad_w = Wk / 2;
  std::vector<std::vector<std::vector<float>>> padded =
      pad_image(image, pad_h, pad_w);

  for (int image_h = 0; image_h < Hi; ++image_h) {
    for (int image_w = 0; image_w < Wi; ++image_w) {
      for (int c = 0; c < channels; ++c) {
        float sum = 0;
        for (int kh = 0; kh < Hk; ++kh) {
          for (int kw = 0; kw < Wk; ++kw) {
            sum += kernel[kh][kw][c] * padded[image_h + kh][image_w + kw][c];
          }
        }
        out[image_h][image_w][c] = sum;
      }
    }
  }

  return out;
}

std::vector<std::vector<std::vector<float>>>
bilateral_conv(const std::vector<std::vector<std::vector<float>>> &image,
               const std::vector<std::vector<std::vector<float>>> &kernel, float sigma_range) {
  int Hi = image.size();
  int Wi = image[0].size();
  int channels = image[0][0].size();
  int Hk = kernel.size();
  int Wk = kernel[0].size();

  std::vector<std::vector<std::vector<float>>> out(
      Hi, std::vector<std::vector<float>>(Wi, std::vector<float>(channels, 0)));

  int pad_h = Hk / 2;
  int pad_w = Wk / 2;
  std::vector<std::vector<std::vector<float>>> padded =
      pad_image(image, pad_h, pad_w);

  for (int image_h = 0; image_h < Hi; ++image_h) {
    for (int image_w = 0; image_w < Wi; ++image_w) {
      for (int c = 0; c < channels; ++c) {
        float sum = 0;
        float weight_sum = 0;

        for (int kh = 0; kh < Hk; ++kh) {
          for (int kw = 0; kw < Wk; ++kw) {
            float intensity_diff = image[image_h][image_w][c] - padded[image_h + kh][image_w + kw][c];
            float range_gaussian = std::exp(-(intensity_diff * intensity_diff) / (2 * sigma_range * sigma_range));

            float weight = kernel[kh][kw][c] * range_gaussian;
            sum += padded[image_h + kh][image_w + kw][c] * weight;
            weight_sum += weight;
          }
        }
        out[image_h][image_w][c] = sum / weight_sum;
      }
    }
  }

  return out;
}

std::vector<std::vector<std::vector<float>>> box_kernel(int size, int channels) {
  float value = 1.0f / (size * size);

  std::vector<std::vector<std::vector<float>>> kernel(
      size,
      std::vector<std::vector<float>>(size, std::vector<float>(channels, value))
  );

  return kernel;
}

std::vector<std::vector<std::vector<float>>> gaussian_kernel(int size, int channels) {
  std::vector<std::vector<std::vector<float>>> kernel(
      size,
      std::vector<std::vector<float>>(size, std::vector<float>(channels, 0))
  );

  double sigma = ((double)size / 2 > 1) ? (double)size / 2 : 1;
  int k = (size - 1) / 2;

  double sum = 0.0;

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      double exponent = -((std::pow(i - k, 2) + std::pow(j - k, 2)) / (2 * sigma * sigma));
      float value = (1 / (2 * M_PI * sigma * sigma)) * std::exp(exponent);

      for (int c = 0; c < channels; ++c) {
        kernel[i][j][c] = value;
      }
      sum += value;
    }
  }

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      for (int c = 0; c < channels; ++c) {
        kernel[i][j][c] /= sum;
      }
    }
  }

  return kernel;
}

std::vector<std::vector<std::vector<float>>> bilateral_kernel(
  const std::vector<std::vector<std::vector<float>>> &image, int size, float sigma_space, float sigma_range, int channels) {

  std::vector<std::vector<std::vector<float>>> kernel(
      size,
      std::vector<std::vector<float>>(size, std::vector<float>(channels, 0)));

  int k = (size - 1) / 2;

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      float spatial_gaussian = std::exp(-((i - k) * (i - k) + (j - k) * (j - k)) / (2 * sigma_space * sigma_space));
      for (int c = 0; c < channels; ++c) {
        kernel[i][j][c] = spatial_gaussian;
      }
    }
  }

  return kernel;
}

std::vector<std::vector<std::vector<float>>>
median_filter(const std::vector<std::vector<std::vector<float>>> &image, int kernel_size) {
    int Hi = image.size();
    int Wi = image[0].size();
    int channels = image[0][0].size();
    int pad_h = kernel_size / 2;
    int pad_w = kernel_size / 2;
    std::vector<std::vector<std::vector<float>>> out(
        Hi, std::vector<std::vector<float>>(Wi, std::vector<float>(channels, 0)));

    std::vector<std::vector<std::vector<float>>> padded = pad_image(image, pad_h, pad_w);

    for (int image_h = 0; image_h < Hi; ++image_h) {
        for (int image_w = 0; image_w < Wi; ++image_w) {
            for (int c = 0; c < channels; ++c) {
                std::vector<float> neighborhood;
                for (int kh = 0; kh < kernel_size; ++kh) {
                    for (int kw = 0; kw < kernel_size; ++kw) {
                        neighborhood.push_back(padded[image_h + kh][image_w + kw][c]);
                    }
                }
                std::sort(neighborhood.begin(), neighborhood.end());
                out[image_h][image_w][c] = neighborhood[neighborhood.size() / 2];
            }
        }
    }

    return out;
}

std::vector<unsigned char>
flatten_image(const std::vector<std::vector<std::vector<float>>> &image,
              int width, int height, int channels) {
  std::vector<unsigned char> flat_image(width * height * channels);

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      for (int c = 0; c < channels; ++c) {
        flat_image[(i * width + j) * channels + c] =
            static_cast<unsigned char>(image[i][j][c]);
      }
    }
  }

  return flat_image;
}

std::vector<std::vector<std::vector<float>>> motion_kernel(int size, const std::string &direction, int channels) {
    std::vector<std::vector<std::vector<float>>> kernel(size,
        std::vector<std::vector<float>>(size, std::vector<float>(channels, 0)));

    float value = 1.0f / size;

    if (direction == "vertical") {
        for (int i = 0; i < size; ++i) {
            for (int c = 0; c < channels; ++c) {
                kernel[i][0][c] = value;
            }
        }
    } else if (direction == "horizontal") {
        for (int j = 0; j < size; ++j) {
            for (int c = 0; c < channels; ++c) {
                kernel[0][j][c] = value;
            }
        }
    } else if (direction == "diagonal") {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                for (int c = 0; c < channels; ++c) {
                    if (i == j) {
                        kernel[i][j][c] = value;
                    }
                }
            }
        }
    }

    return kernel;
}

int main(int argc, char *argv[]) {
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
      ("input,i", boost::program_options::value<std::string>(), "set input file name")
      ("output,o", boost::program_options::value<std::string>(), "set output file name")
      ("algo,a", boost::program_options::value<std::string>(), "set algorithm (default: gaussian)")
      ("strength,s", boost::program_options::value<int>(), "set blur strength (default: 3)")
      ("sigma_range,sr", boost::program_options::value<float>(), "set sigma range for bilateral blur (default: 50.0)")
      ("sigma_space,sp", boost::program_options::value<float>(), "set sigma space for bilateral blur (default: 2.0)")
      ("motion_direction,m", boost::program_options::value<std::string>(), "set direction for motion blur")
      ("help,h", "display usage message");

  if (argc == 1) {
    std::cout << desc << "\n";
    return 0;
  }

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  int strength = DEFAULT_STRENGTH;
  float sigma_space = DEFAULT_SIGMA_SPACE;
  float sigma_range = DEFAULT_SIGMA_RANGE;
  std::string algorithm = DEFAULT_ALGORITHM;
  std::string motion_direction;

  int width, height, channels;
  unsigned char *image_data;
  std::string image_name;
  std::string output_name;

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  if (vm.count("input")) {
    image_name = vm["input"].as<std::string>();
    image_data = stbi_load(image_name.c_str(), &width, &height, &channels, 0);
  } else {
    std::cerr << "Error: please specify input image" << std::endl;
    return 1;
  }

  if (image_data == nullptr) {
    std::cerr << "Error: could not load image: " << image_name << std::endl;
    return 1;
  }

  if (vm.count("output")) {
    output_name = vm["output"].as<std::string>();
  } else {
    std::cerr << "Error: please specify output path: " << image_name
              << std::endl;
    return 1;
  }

  if (vm.count("strength"))
    strength = vm["strength"].as<int>();

  if (vm.count("sigma_range"))
    sigma_range = vm["sigma_range"].as<float>();

  if (vm.count("sigma_space"))
    sigma_space = vm["sigma_space"].as<float>();

  if (vm.count("algo"))
    algorithm = vm["algo"].as<std::string>();

  if (algorithm == "motion") {
    if (vm.count("motion_direction")) {
      motion_direction = vm["motion_direction"].as<std::string>();

      if (motion_direction != "vertical" && motion_direction != "horizontal" && motion_direction != "diagonal") {
        std::cerr << "Error: Invalid motion direction (valid: horizontal, vertical, diagonal)." << std::endl;
        return 1;
      }
    } else {
      std::cerr << "Error: Please specify motion direction (horizontal, vertical, diagonal)." << std::endl;
      return 1;
    }
  }

  std::vector<std::vector<std::vector<float>>> image(
      height,
      std::vector<std::vector<float>>(width, std::vector<float>(channels)));

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      for (int c = 0; c < channels; ++c) {
        image[i][j][c] =
            static_cast<float>(image_data[(i * width + j) * channels + c]);
      }
    }
  }

  std::vector<std::vector<std::vector<float>>> blurred_image;

  if (algorithm == "gaussian")
    blurred_image = conv(image, gaussian_kernel(strength, channels));
  else if (algorithm == "box")
    blurred_image = conv(image, box_kernel(strength, channels));
  else if (algorithm == "bilateral")
    blurred_image = bilateral_conv(image, bilateral_kernel(image, strength, sigma_space, sigma_range, channels), sigma_range);
  else if (algorithm == "median")
    blurred_image = median_filter(image, strength);
  else if (algorithm == "motion")
        blurred_image = conv(image, motion_kernel(strength, motion_direction, channels));

  auto output_image = flatten_image(blurred_image, width, height, channels);
  std::string extension = output_name.substr(output_name.find_last_of('.') + 1);

  if (extension == "png") {
    stbi_write_png(output_name.c_str(), width, height, channels,
                   output_image.data(), width * channels);
  } else if (extension == "jpg" || extension == "jpeg") {
    stbi_write_jpg(output_name.c_str(), width, height, channels,
                   output_image.data(), 100);
  } else {
    std::cerr << "Error: Unsupported output file format. Please use .png or "
                 ".jpg/.jpeg"
              << std::endl;
    return 1;
  }

  stbi_image_free(image_data);

  return 0;
}
