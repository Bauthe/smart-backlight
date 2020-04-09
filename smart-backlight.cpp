# include <iostream>
# include <fstream>
# include <list>
# include <vector>
# include <string>
# include <sstream>
# include <sys/stat.h>

//HOME environment variable
std::string HOME = std::getenv("HOME");

//function to check the success of the permission setter
int checkPermission();

//struct and function to get the config
struct config {
	std::string device;
	std::vector<float> levels;
};

config readConfig(std::string path);

//main process
int main(int argc, char *argv[])
{
	std::string help = "smart-backlight COMMAND\nConfigures brightness according to predefined levels.\n-> COMMAND: 'up' or 'down'.\n";

    //checking if a command was given
    if (argc < 2) {
        std::cout << help;
        return 1;
    }

    //checking if the command is allowed
    std::string command = argv[1];
    if (command != "up" && command != "down") {
        std::cout << help;
        return 1;
    }
	
	//ensuring that permissions are set
	int permissionResult = checkPermission();
	if (permissionResult == 1) {return 1;}
	
	//reading the config file
	config c = readConfig(HOME + "/.config/smart-backlight");

    //reading the system info to get the current percentage
	std::string location("/sys/class/backlight/" + c.device);
    
	int currentLevel;
    std::ifstream currentFile(location + "/brightness");
    currentFile >> currentLevel;
	
	int maxLevel;
	std::ifstream maxFile(location + "/max_brightness");
	maxFile >> maxLevel;

    float currentPercent = 100 * currentLevel / maxLevel;

    //finding the levels we are between
    int lower = 0, higher;
    for (int const& level: c.levels)
    {
        higher = level;
        if (higher > currentPercent) {break;}
        if (higher != currentPercent) {lower = higher;}
    }

    //executing the command
	std::ofstream f(location + "/brightness");
	if (command == "down" && lower != 0)
    {f << maxLevel * lower / 100;}
    if (command == "up" && higher != currentPercent)
    {f << maxLevel * higher / 100;}

    return 0;
}

//function to ensure that the systemd service has set the right permissions
int checkPermission() {

	//ensuring that permissions are set
	struct stat pipe;
	std::string pipePath("/tmp/smart-backlight");
	stat(pipePath.c_str(), &pipe);

	if (S_ISFIFO(pipe.st_mode)) {

		//if permissions are not set, asking smart-backlight service to set it
		if (! (std::ofstream(pipePath) << "setpermissions")) {
			std::cout << "smart-backlight ERROR: cannot ask for permissions on backlight" << std::endl;
			throw 1;
		}
		
		//awaiting the response and printing error if there was an issue
		std::string response;
		std::ifstream(pipePath) >> response;
		if (response != "complete") {
			std::cout << "smart-backlight ERROR: smart-backlight-service did not report success" << std::endl;
			throw 1;
		}
	}

	return 0;
}

//function to read the config
config readConfig(std::string path) {
	
	config result;
	
	//file reading variables
	std::string line;
	std::ifstream f(path);

	//iterating over the lines
	while (std::getline(f, line)) {
	
		//iterating over the words
		std::stringstream streamline(line);
		std::string word;

		streamline >> word;

		//acquire device name
		if (word == "device") {
			streamline >> result.device;
		}

		//acquire levels
		else if (word == "levels") {

			float level;
			while (streamline >> level) {
				result.levels.push_back(level);
			}
		}

		//discard irrelevant lines
		else if (word[0] != '#' and word != "") {
			std::cout << "ERROR: unexpected input \"" << line << "\" in configuration file" << std::endl;
			throw 1;
		}
	}

	return result;
}
