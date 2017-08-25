#include <iostream>
#include <future>
#include <algorithm>
#include "utils.h"

static const std::string BASE_SOURCE_PATH = "Source\\";
static const std::string BASE_SUB_PATH = "Subs\\";

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

std::string make_avs(std::string episode, std::string resolution, std::string file_format, std::vector<std::string> subs = {}) {
    static const std::string ffms2_dll_path = "LoadPlugin(\"D:\\FinalDevil\\EncodeCLI\\dll\\ffms2.dll\")";
    static const std::string vsfilter_mod_dll_path = "LoadPlugin(\"D:\\FinalDevil\\EncodeCLI\\dll\\VSFilterMod.dll\")";
    std::string output_name = episode + "-[" + resolution + "]";
    std::ofstream avs(output_name + ".avs");
    avs << ffms2_dll_path + "\n";
    avs << vsfilter_mod_dll_path + "\n";
    std::string command_load_ep = "FFVideoSource(\"" + BASE_SOURCE_PATH + episode + "." + file_format + "\", fpsnum=24000, fpsden=1001, threads=24)\n";
    avs << command_load_ep;
    if (is_file_exist(BASE_SUB_PATH + episode + ".avs")) {
        avs << "TextSubMod(" + BASE_SUB_PATH + episode + ".ass, 1)\n";
    } else {
        std::cout << "Can not find " << BASE_SUB_PATH + episode + ".ass => ignore" << std::endl;
    }
    for (auto i = 0; i < subs.size(); i++) {
        avs << "TextSubMod(\"" + BASE_SUB_PATH + subs[i] + ".ass\", 1)\n";
    }
    return output_name;
}

unsigned encode_one_episode(std::string episode, std::string file_format, std::vector<std::string> subs = {}){
    std::string resolution = "480p";
    std::string profile = choose_profile(resolution);
    std::cout << "Encoding " << resolution << " episode " << episode << std::endl;
    std::string output_name = make_avs(episode, resolution, file_format, subs);
    std::string stats_file = episode + "-" + resolution + ".stats";
    std::string binary = get_binary(Binary::x264_8bit);
    for (unsigned i = 1; i <= 2; i++) {
        std::string profile_i_th = profile;
        str_replace_inplace("{pass}", std::to_string(i), profile_i_th);
        str_replace_inplace("{stats}", stats_file, profile_i_th);
        str_replace_inplace("{output}", i == 1 ? "NUL" : output_name + ".mp4", profile_i_th);
        str_replace_inplace("{avs}", output_name + ".avs", profile_i_th);
        std::cout << profile_i_th << std::endl;
        // Call here
    }
    return 0;
}

void batch_encode(std::string source_type) {
    std::cout << "Batch encode!" << std::endl;
    std::vector<std::string> episodes;
    std::string file_lists;
    std::cout << "Please enter episode number list, separate by ,: ";
    std::cin >> file_lists;
    split(file_lists, episodes, ',');
    std::vector<std::future<unsigned>> futures;
    for (auto ep : episodes) {
        futures.push_back(std::async(std::launch::async,
            [&ep, &source_type](){
                return encode_one_episode(ep, source_type);
            }
        ));
    }
    for (auto& future: futures) {
        future.get();
    }
}

int main(int argc, char const *argv[]) {
    if (argc >= 2) {
        std::string file_name = argv[1];
        std::vector<std::string> subs = [&argc, &argv]() {
            std::vector<std::string> subs;
            if (argc >= 3) {
                for (unsigned i = 2; i < argc; i++) {
                    subs.push_back(std::string(argv[i]));
                }
            }
            return subs;
        }();
        if (is_file_exist(file_name + ".mp4")) {
            encode_one_episode(std::string(argv[1]), "mp4", subs);
        } else if (is_file_exist(file_name + ".mkv")) {
            encode_one_episode(std::string(argv[1]), "mkv", subs);
        } else {
            std::cout << "Can not find source " << std::endl;
            exit(0);
        }
    } else {
        std::cout << "Batch encode? Need to specify format first!" << std::endl;
        std::cout << "0: mp4" << std::endl;
        std::cout << "1: mkv" << std::endl;
        unsigned source_type;
        while(true) {
            std::cout << "Choose one of above: ";
            std::cin >> source_type;
            if (source_type == 0 || source_type == 1) {
                break;
            }
        }
        batch_encode(get_source_type(source_type));
    }
    return 0;
}
