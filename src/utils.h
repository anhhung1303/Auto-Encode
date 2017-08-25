#pragma once
#include <map>
#include <fstream>

enum class Binary {
    x264_8bit = 0,
    x264_10bit,
    x265_8bit,
    x265_10bit
};

std::string get_profile(const std::string need_profile) {
    static const std::map<std::string, std::string> profiles = {
        {"480p", "--profile baseline --tune animation --bframes 3 --level 3.0 --bitrate 430 --threads 24"},
        {"720p", "--profile baseline --tune animation --bframes 3 --level 3.0 --bitrate 130 --threads 24"},
        {"1080p", "--profile baseline --tune animation --bframes 3 --level 3.0 --bitrate 2300 --threads 24"}
    };
    return profiles.at(need_profile);
}

std::string get_binary(Binary binary) {
    switch (binary) {
        case Binary::x264_8bit:
            return "x264_64_8";
        case Binary::x264_10bit:
            return "x264_64_10";
        case Binary::x265_8bit:
            return "x265_64_8";
        case Binary::x265_10bit:
            return "x265_64_10";
        default:
            return "x264_64_8";
    }
}

bool is_file_exist(const std::string file_name) {
    std::ifstream infile(file_name);
    return infile.good();
}
