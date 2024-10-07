#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

int
main(int argc, char *argv[])
{
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("input,i", boost::program_options::value<std::string>(), "set input file name")
		("output,o", boost::program_options::value<std::string>(), "set output file name")
		("algo,a", boost::program_options::value<std::string>(), "set algorithm")
		("strength,s", boost::program_options::value<std::string>(), "set blur strength")
		("help,h", "display usage message");

	if (argc == 1) {
		std::cout << desc << "\n";
	}

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	int width, height, channels;
	unsigned char *image;
	std::string image_name;

	if (vm.count("input")) {
		image_name = vm["input"].as<std::string>();
		image = stbi_load(image_name.c_str(), &width, &height, &channels, 0);
	}

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 0;
	}


	if (image == nullptr) {
		std::cerr << "Error: could not load image: " << image_name << std::endl;
		return 1;
	}

	stbi_image_free(image);

	return 0;
}
