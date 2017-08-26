#pragma once
#include <map>
#include <fstream>
#include <ctime>
#include <cassert>

static const std::string BASE_SOURCE_PATH = "Source\\";
static const std::string BASE_SUB_PATH = "Subs\\";

enum class Binary {
    x264_8bit = 0,
    x264_10bit,
    x265_8bit,
    x265_10bit
};

enum class ResultStatus : unsigned {
	SUCCESS = 0,
	E_FILE_NOT_FOUND,
	E_CREATE_PROCESS_FAILED
};

std::string get_source_type(unsigned index) {
    static const std::map<unsigned, std::string> source_types = {
        {0, "mp4"},
        {1, "mkv"}
    };
    return source_types.at(index);
}

std::string get_profile(const std::string need_profile) {
    static const std::map<std::string, std::string> profiles = {
        {"480p", " --profile baseline --tune animation --bframes 3 --level 3.0 --bitrate 430 --no-progress --quiet --colorprim bt709 --pass {pass} --stats {stats} --output {output} {avs} --threads 16"},
        {"720p", " --profile baseline --tune animation --bframes 3 --level 3.0 --bitrate 1300 --no-progress --quiet --colorprim bt709 --pass {pass} --stats {stats} --output {output} {avs} --threads 16"},
        {"1080p", " --profile baseline --tune animation --bframes 3 --level 3.0 --bitrate 2300 --no-progress --quiet --colorprim bt709 --pass {pass} --stats {stats} --output {output} {avs} --threads 16"}
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

void str_replace_inplace(const std::string& search, const std::string& replace, std::string& source) {
    auto pos = source.find(search);
    size_t search_sz = search.size();
    size_t replace_sz = replace.size();
    while (pos != std::string::npos) {
        source.replace(pos, search_sz, replace);
        pos = source.find(search, pos+replace_sz);
    }
}

void split(const std::string& s, std::vector<std::string>& v, char delimiter) {
    size_t start = 0;
    size_t end = s.find_first_of(delimiter, start);
    bool last = false;
    while (!last) {
        assert(end >= start);
        if (end == std::string::npos) {
            last = true;
        }
        v.push_back(s.substr(start, end-start));
        if (!last) {
            start = end + 1;
            end = s.find_first_of(delimiter, start);
        }
    }
}

bool any_source_format_exist(std::string episode, std::string& format) {
	if (is_file_exist(BASE_SOURCE_PATH + episode + ".mp4")) {
		format = "mp4";
		return true;
	}
	if (is_file_exist(BASE_SOURCE_PATH + episode + ".mkv")) {
		format = "mkv";
		return true;
	}
	return false;
}

std::time_t real_time_now() {
	return std::time(nullptr);
}
