#include <iostream>
#include <future>
#include <algorithm>
#include "utils.h"

std::string choose_profile(std::string& resolution){
    std::cout << "1. 480" << std::endl;
    std::cout << "2. 720" << std::endl;
    std::cout << "3. 1080" << std::endl;
    static const std::map<unsigned, std::string> map_options = {
        {1, "480p"},
        {2, "720p"},
        {3, "1080p"}
    };
    unsigned option = 1;
    std::cout << "Please choose profile: ";
    std::cin >> option;
    resolution = map_options.at(option);
    return get_profile(resolution);
}

std::string make_avs(std::string episode, std::string resolution, std::string file_format) {
    std::string output_name = episode + "-[" + resolution + "]";
    std::ofstream avs(output_name + ".avs");
    avs << "LoadPlugin(\"D:\\FinalDevil\\EncodeCLI\\dll\\ffms2.dll\")";
    avs << "LoadPlugin(\"D:\\FinalDevil\\EncodeCLI\\dll\\VSFilterMod.dll\")";
    std::string command_load_ep = "FFVideoSource(" + "Source\\" + episode + "." + file_format + "fpsnum=24000, fpsden=1001, threads=24)";
    return output_name;
}

void encode_one_episode(std::string episode, std::string file_format){
    std::string resolution = "480p";
    std::string profile = choose_profile(resolution);
    std::cout << "Encoding " << resolution << " episode " << episode << std::endl;
    std::string output_name = make_avs(episode, resolution, file_format);
    std::cout << output_name << std::endl;
}

void batch_encode() {
    std::cout << "Batch encode" << std::endl;
}

int main(int argc, char const *argv[]) {
    if (argc == 2) {
        std::string file_name = argv[1];
        if (is_file_exist("Source\\" + file_name + ".mp4")) {
            encode_one_episode(std::string(argv[1]), "mp4");
        } else if (is_file_exist("Source\\" + file_name + ".mkv")) {
            encode_one_episode(std::string(argv[1]), "mkv");
        } else {
            std::cout << "Can not file source" << std::endl;
            exit(0);
        }
    } else {
        batch_encode();
    }
    return 0;
}
