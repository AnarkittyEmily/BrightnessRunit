#include <fstream>
#include <iostream>
#include <filesystem>
#include <getopt.h>
#include <sys/stat.h>

// getting the device driver, then accessing said file, then modifying it 

struct config { 
    std::string configFileContents; 
    std::string configFileName = "/etc/brightness_runit/brightness_runit.conf"; 
    std::ifstream configFile; 
};


int checkifConfigExists() { 
    config config; 
    if (!std::filesystem::exists(config.configFileName)) 
    { 
        return 1; 
    }
    
    return 0;
}

std::string getDeviceDriver() {
    config driver; 
    if (checkifConfigExists() == 1) { 
       std::cout << "Config file doesn't exist\n";
       return "1"; 
    } 
    driver.configFile.open(driver.configFileName); 
    if (!driver.configFile.is_open()) 
    {
        return "2"; 
    }

    while (getline(driver.configFile, driver.configFileContents, '=')) { 
        if (driver.configFileContents.find("driver") != std::string::npos) { 
            driver.configFile >> driver.configFileContents; 
            driver.configFile.close(); 
            return driver.configFileContents; 
        } else {
            continue; 
        }
    }
    driver.configFile.close(); 
    std::cout << "Device driver doesn't exist in config";
    return "1";  
}


int getBrightnessSetting() { 
    config brightness; 
    brightness.configFile.open(brightness.configFileName);

    if (checkifConfigExists() == 1) { 
        return 1;  
    } 
    if (!brightness.configFile.is_open()) 
    {
        return 2; 
    }
    int brightnessValue = 0;  
    while (getline(brightness.configFile, brightness.configFileContents, '=')) {
        if (brightness.configFileContents.find("brightness") != std::string::npos) {
            brightness.configFile >> brightnessValue; 

            return brightnessValue; 
        }
    }

    return 0; 
}


// if old brightness value should be stored in the file
bool getOptionForSave() { 
    config option; 

    option.configFile.open(option.configFileName); 

    if (checkifConfigExists() == 1) { 
        return 1; 
    }

    while (getline(option.configFile, option.configFileContents)) { 
        if (option.configFileContents.find("save_on_shutdown") != std::string::npos) { 
            option.configFile >> option.configFileContents;
            if (option.configFileContents.find("true") != std::string::npos
                or option.configFileContents.find("1") != std::string::npos) { 
                return true; 
            } else { 
                return false; 
            }
        }
    }

    return false; 
}

const std::string getBacklightFile() { 
    std::string backlightFile = "/sys/class/backlight/_/brightness"; 
    std::string driver; 
    if ((driver = getDeviceDriver()) != "1") { 
        backlightFile.replace(backlightFile.find("_"), 1, driver); 
    }
    return backlightFile; 
}


int getBacklightValue() { 
    std::ifstream backlightFile;
    backlightFile.open(getBacklightFile());
    std::string backlightFileContents; 


    while (getline(backlightFile, backlightFileContents)) { 
        return std::stoi(backlightFileContents); 
    }
    return 0; 
}

int getBacklightValuePrevious() { 
    std::ifstream previousBacklight;
    std::string previousBackLightContents; 
    previousBacklight.open("/etc/brightness_runit/previous_brightness.conf");

    if (!previousBacklight.is_open()) { 
        std::cout << "Could not open file\n"; 
        return 0; 
    }

    while (getline(previousBacklight, previousBackLightContents)) {
        return std::stoi(previousBackLightContents); 
    }
}

void onShutdown() { 
    std::fstream save_previous_brightness; 

    if (!std::filesystem::exists("/etc/brightness_runit/previous_brightness.conf")) { 
        save_previous_brightness.open("/etc/brightness_runit/previous_brightness.conf", std::fstream::out);
    } else {
        save_previous_brightness.open("/etc/brightness_runit/previous_brightness.conf"); 
    }
    save_previous_brightness << getBacklightValue(); 
}

void onTurnOn(int &value) { 
    std::string backlightFileName = getBacklightFile();
    std::fstream backlightFile;
    if (chmod(backlightFileName.c_str(), 644) == -1) {
        std::cout << "Could not change mode" << std::endl;
        return;  
    }

    backlightFile.open(backlightFileName); 

    backlightFile << value;  
}

int main(int argc, char **argv) { 
    bool shutdown; 
    bool turnOn; 
    struct option long_options[] = { 
        {"--start", no_argument, 0, 's'},
        {"--stop", no_argument, 0, 'o'}
    }; 
    int option_index; 
    int args;
    while ((args = getopt_long(argc, argv, "so", long_options, &option_index)) != -1) { 
        switch (args) { 
            case 's': { 
                turnOn = true; 
                break; 
            }

            case 'o': {
                shutdown = true; 
                break;
            }

            case '?': { 
                break; 
            }
        }
    }

    if (shutdown) { 
        if (getOptionForSave() == true){ 
            if (getDeviceDriver() == "1") { 
                return 1; 
            }
            onShutdown(); 
        } else {
            return 0; 
        }
    } else if (turnOn) { 
        if (getOptionForSave() == true) { 
            if (getDeviceDriver() == "1") { 
                return 1; 
            }
            int old_brightness = getBacklightValuePrevious(); 
            onTurnOn(old_brightness); 
        } else {
            if (getDeviceDriver() == "1") { 
                return 1; 
            } 
            int brightness = getBrightnessSetting(); 
            onTurnOn(brightness); 
        } 
    }

}
