#include <iostream>
#include <string>
#include <boost/program_options.hpp>

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

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 0;
	}

	return 0;
}
