#include <iostream>
#include <future>
#include <thread>
#include <algorithm>
#include <string>
#include <windows.h>
#include <Lmcons.h>
#include <stdio.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

using std::string;

static const std::string null_redirect = " >nul 2>&1";

ResultStatus create_process(std::string& command) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		const_cast<LPSTR>(command.c_str()),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		) {
		std::cout << "Error: Can not start x264!" << std::endl;
		return ResultStatus::E_CREATE_PROCESS_FAILED;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return ResultStatus::SUCCESS;
}

std::string choose_profile(std::string& resolution){
    std::cout << "1: 480" << std::endl;
    std::cout << "2: 720" << std::endl;
    std::cout << "3: 1080" << std::endl;
	std::cout << "4: 720p download" << std::endl;
    static const std::map<unsigned, std::string> map_options = {
        {1, "480p"},
        {2, "720p"},
        {3, "1080p"},
		{4, "720pdl"}
    };
    unsigned option = 1;
    std::cout << "Please choose profile [1,2,3,4]? ";
    std::cin >> option;
    resolution = map_options.at(option);
    return get_profile(resolution);
}

std::string make_avs(std::string episode, std::string resolution, std::string file_format, std::vector<std::string> subs = {}) {
    static const std::string ffms2_dll_path = "LoadPlugin(\"D:\\FinalDevil\\EncodeCLI\\ffms2.dll\")";
    static const std::string vsfilter_mod_dll_path = "LoadPlugin(\"D:\\FinalDevil\\EncodeCLI\\VSFilterMod.dll\")";
	std::string output_name = episode + "-[" + resolution + "]";
	std::ofstream avs(output_name + ".avs");
	avs << ffms2_dll_path + "\n";
	avs << vsfilter_mod_dll_path + "\n";
	std::string command_load_ep = "FFVideoSource(\"" + BASE_SOURCE_PATH + episode + "." + file_format + "\", fpsnum=24000, fpsden=1001, threads=24)\n";
	avs << command_load_ep;
	if (is_file_exist(BASE_SUB_PATH + episode + ".ass")) {
		avs << "TextSubMod(\"" + BASE_SUB_PATH + episode + ".ass\", 1)\n";
	}
	else {
		std::cout << "Can not find " << BASE_SUB_PATH + episode + ".ass => assume hardsub" << std::endl;
	}
	if (is_file_exist(BASE_SUB_PATH + "logo.ass")) {
		avs << "TextSubMod(\"" + BASE_SUB_PATH + "logo.ass\", 1)\n";
	}
	for (size_t i = 0; i < subs.size(); i++) {
		avs << "TextSubMod(\"" + BASE_SUB_PATH + subs[i] + ".ass\", 1)\n";
	}
	if (resolution == "480p") {
		avs << "LanczosResize(848, 480)\n";
	}
    return output_name;
}

ResultStatus encode_video(std::string encode_command) {
	//std::cout << "Encoding video" << std::endl;
	return create_process(encode_command);
	//return ResultStatus::SUCCESS;
}

std::string encode_audio(std::string episode, std::string resolution, std::string file_format) {
	//std::cout << "Encoding audio " << episode << std::endl;
	std::string audio_output = episode + "-" + resolution + "-audio.mp4";
	std::string wav_file = episode + "-" + resolution + ".wav";
	system(std::string("ffmpeg -i " + BASE_SOURCE_PATH + episode + "." + file_format + " -vn -f wav " + wav_file + null_redirect).c_str());
	system(std::string("neroAacEnc -if " + wav_file + " -ignorelength -he -br 96000 -of " + audio_output + null_redirect).c_str());
	system(std::string("del " + wav_file + null_redirect).c_str());
	return audio_output;
}

ResultStatus encode_one_episode(std::string episode, std::string resolution, std::string profile, std::vector<std::string> subs = {}) {
	std::cout << "Encoding episode " << episode << " with profile " << resolution << std::endl;
	std::string file_format;
	const bool exist = any_source_format_exist(episode, file_format);
	if (!exist) {
		std::cout << "Can not find source for episode " << episode << std::endl;
		return ResultStatus::E_FILE_NOT_FOUND;
	}
    std::string output_name = make_avs(episode, resolution, file_format, subs);
	Sleep(1000); // Safety wait until avs creation 100% completed
	std::string encoded_video = output_name + ".264";
    std::string stats_file = episode + "-" + resolution + ".stats";
	std::string avs_file = output_name + ".avs";
    static std::string binary = get_binary(Binary::x264_8bit);

	std::future<std::string> audio_task = std::async(std::launch::async,
		[episode, resolution, file_format]() {
			return encode_audio(episode, resolution, file_format);
		});

    for (unsigned i = 1; i <= 2; i++) {
        std::string profile_i_th = profile;
        str_replace_inplace("{pass}", std::to_string(i), profile_i_th);
        str_replace_inplace("{stats}", stats_file, profile_i_th);
        str_replace_inplace("{output}", i == 1 ? "NUL" : encoded_video, profile_i_th);
        str_replace_inplace("{avs}", avs_file, profile_i_th);
		std::string encode_command("avs4x26x --x26x-binary --seek-mode fast " + binary + profile_i_th + null_redirect);
		ResultStatus status = encode_video(encode_command);
		/*std::future<ResultStatus> pass_task = std::async(std::launch::async,
			[encode_command]() {
			return encode_video(encode_command);
		});
		status = pass_task.get();
		if (i == 1) {
			audio_output = encode_audio(episode, resolution, file_format);
		}*/
		if (status != ResultStatus::SUCCESS) {
			return status;
		}
    }

	std::string audio_output = episode + "-" + resolution + "-audio.mp4";
	const std::string final_video = output_name + ".mp4";
	std::string mux_cmd = "mp4box -add " + audio_output + " -add " + encoded_video + " " + final_video + null_redirect;
	system(mux_cmd.c_str());

	system(std::string("del " + stats_file + null_redirect).c_str());
	system(std::string("del " + stats_file + ".mbtree" + null_redirect).c_str());
	system(std::string("del " + encoded_video + null_redirect).c_str());
	system(std::string("del " + audio_output + null_redirect).c_str());
	system(std::string("del " + avs_file + null_redirect).c_str());
	system(std::string("del " + BASE_SOURCE_PATH + episode + "." + file_format + ".ffindex" + null_redirect).c_str());
	std::cout << "Done encode episode " << episode << std::endl;
    return ResultStatus::SUCCESS;
}

void batch_encode(std::string file_format, std::string resolution, std::string profile) {
    std::cout << "Batch encode!" << std::endl;
    std::vector<std::string> episodes;
    std::string file_lists;
    std::cout << "Please enter episode number list, separate by ,: ";
    std::cin >> file_lists;
    split(file_lists, episodes, ',');
    std::vector<std::future<ResultStatus>> futures;
    for (auto ep : episodes) {
        futures.push_back(std::async(std::launch::async,
            [ep, &file_format, &resolution, &profile](){
                return encode_one_episode(ep, resolution, profile);
            }
        ));
    }
    for (auto& future: futures) {
        future.get();
    }
}

int main(int argc, char const *argv[]) {
	std::cout << "----------------------------------" << std::endl;
	std::cout << "| VuiGhe encode tool version 2.0 |" << std::endl;
	std::cout << "| @Author: FinalDevil            |" << std::endl;
	std::cout << "----------------------------------" << std::endl;
	/*char username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username, &username_len);
	std::cout << username << std::endl;*/
	std::string resolution = "480p";
	std::string profile = choose_profile(resolution);
	std::time_t s_time;
    if (argc >= 2) {
        std::string file_name = argv[1];
        std::vector<std::string> subs = [&argc, &argv]() {
            std::vector<std::string> subs;
            if (argc >= 3) {
                for (auto i = 2; i < argc; i++) {
                    subs.push_back(std::string(argv[i]));
                }
            }
            return subs;
        }();
		s_time = real_time_now();
		encode_one_episode(std::string(argv[1]), resolution, profile, subs);
    } else {
        std::cout << "Batch encode need to choose file format first!" << std::endl;
        std::cout << "0: mp4" << std::endl;
        std::cout << "1: mkv" << std::endl;
        unsigned source_type;
        while(true) {
            std::cout << "Please choose source format [0,1]? ";
            std::cin >> source_type;
            if (source_type == 0 || source_type == 1) {
                break;
            }
        }
		s_time = real_time_now();
        batch_encode(get_source_type(source_type), resolution, profile);
    }
	auto duration = real_time_now() - s_time;
	auto minutes = static_cast<int>((duration / 60) % 60);
	auto hours = static_cast<int>(minutes / 60);
	auto seconds = static_cast<int>(duration % 60);
	std::cout << "Total encoded time: " << hours << ":" << minutes << ":" << seconds << std::endl;
    return 0;
}
