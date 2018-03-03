// Detector.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"

#include <iostream>
#include <sstream>      
#include <vector>
#include <map>
#include <exception>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#if defined (_WIN32) || (_WIN64)
  #include <windows.h>
#else
  #include <dlfcn.h>
#endif

#include "GenericDetector.h"

using namespace std;
using namespace boost;

namespace po = boost::program_options;
namespace fs = boost::filesystem;
#if defined (_WIN32) || (_WIN64)
    typedef GenericPreparation* (*PREP) (string, string, string, string, string, vector<string>, int);
#else
    typedef GenericPreparation* PREP(string, string, string, string, string, vector<string>, int);
#endif

// Parses command line arguments
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
	copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
	return os;
}

map<string, string> clParse(int argc, char* argv[]) {

	map<string, string> argument;

	try {
		string configFile;

		string faceDataset;
		string noseDataset;
		string mouthDataset;
		string eyesDataset;

		string directory;
		int threads;

		// Declare a group of options that will be 
		// allowed only on command line
		po::options_description generic("Generic options");
		generic.add_options()
			("help,h", "produce help message")
			("config,c", po::value<string>(&configFile),				
				"name of a file of a configuration.")
			;

		// Declare a group of options that will be 
		// allowed both on command line and in
		// config file
		po::options_description config("Configuration");
		config.add_options()
			("faces,f", po::value<string>(&faceDataset)->default_value("datasets/haarcascade_frontalface_alt.xml"), "path to fases detection file")
			("eyes,e", po::value<string>(&eyesDataset)->default_value("datasets/haarcascade_eye.xml"), "path to eyes detection file")
			("mouths,m", po::value<string>(&mouthDataset)->default_value("datasets/haarcascade_mcs_mouth.xml"), "path to mouths detection file")
			("noses,n", po::value<string>(&noseDataset)->default_value("datasets/haarcascade_mcs_nose.xml"), "path to noses detection file")
			;

		// The options will be allowed both on command line and
		// in config file, but will not be shown to the user
		po::options_description non_param("can be passed without keys");
		non_param.add_options()
			("directory,d", po::value<string>(&directory), "input directory")
			("threads,t", po::value<int>(&threads)->default_value(2), "number of threads")
			;

		po::options_description cmdline_options;
		cmdline_options.add(generic).add(config).add(non_param);

		po::options_description config_file_options;
		config_file_options.add(config).add(non_param);

		po::options_description visible("Allowed options");
		visible.add(generic).add(config).add(non_param);

		po::positional_options_description p;
		p.add("directory", 1).add("threads", -1);

		po::variables_map vm;
		store(po::command_line_parser(argc, argv).
			options(cmdline_options).positional(p).run(), vm);
		notify(vm);

		// The command line doesn't contain any argument
		if (vm.count("help") || argc == 1) {
			cout << visible << "\n";
			argument.empty();
			return argument;
		}

		if (vm.count("config")) {

			// Whether the config exists
			ifstream ifs(configFile.c_str());

			if (!ifs)
			{
				cout << "can not open config file: " << configFile << "\n";
			}
			else
			{	
				store(parse_config_file(ifs, config_file_options), vm);
				notify(vm);
			}
		}

		// Filling result map
		if (vm.count("faces"))
		{
			argument["faces"] = vm["faces"].as< string >();
		}

		if (vm.count("eyes"))
		{
			argument["eyes"] = vm["eyes"].as< string >();
		}

		if (vm.count("mouths"))
		{
			argument["mouths"] = vm["mouths"].as< string >();
		}

		if (vm.count("noses"))
		{
			argument["noses"] = vm["noses"].as< string >();
		}

		if (vm.count("directory"))
		{
			argument["directory"] = vm["directory"].as< string >();
		}

		if (vm.count("threads"))
		{
			argument["threads"] = to_string(vm["threads"].as< int >());
		}

	}
	catch (std::exception& e)
	{
		cout << e.what() << "\n";
	}

	return argument;
}

int main(int ac, char* av[])
{
	// Banner
	cout << "  /$$$$$$$ /$$$$$$$$/$$$$$$$$/$$$$$$$$ /$$$$$$ /$$$$$$$$/$$$$$$ /$$$$$$$  " << endl;
	cout << " | $$__  $| $$_____|__  $$__| $$_____//$$__  $|__  $$__/$$__  $| $$__  $$ " << endl;
	cout << " | $$  | $| $$        | $$  | $$     | $$  |__/  | $$ | $$    $| $$    $$ " << endl;
	cout << " | $$  | $| $$$$$     | $$  | $$$$$  | $$        | $$ | $$  | $| $$$$$$$  " << endl;
	cout << " | $$  | $| $$__/     | $$  | $$__/  | $$        | $$ | $$  | $| $$__  $$ " << endl;
	cout << " | $$  | $| $$        | $$  | $$     | $$    $$  | $$ | $$  | $| $$    $$ " << endl;
	cout << " | $$$$$$$| $$$$$$$$  | $$  | $$$$$$$|  $$$$$$/  | $$ |  $$$$$$| $$  | $$ " << endl;
	cout << " |_______/|________/  |__/  |________/ ______/   |__/    _____/|__/  |__/ " << endl;

	// For visual purposes an empty line is being printed. 
	cout << endl;
	cout << endl;

    #if defined (_WIN32) || (_WIN64)
		TCHAR currentDir[MAX_PATH];
		GetCurrentDirectoryW(sizeof(currentDir), currentDir);
			
		SetCurrentDirectoryW(currentDir);
		
		// Loads functions from imported dll	
		HMODULE dllFile = LoadLibraryW(L"FaceDetectorLib.dll");
			
		if (dllFile == NULL) {		
			cout << "Can't find FaceDetectorLib.dll file. Exit!" << endl;
			return 1;
		}
    #else
        void* handle = dlopen("./libFaceDetectorLib.so", RTLD_LAZY); 
	
        if (!handle) {		
			cout << "Can't find libFaceDetectorLib.so file. Exit!" << endl;
			return 1;
		}
    #endif
			
	//Parses command line arguments and pushes it to map
	map<string, string> params(clParse(ac, av));
	
	// If none of paramas are passed - the program shows help and exits
	if (params.size() == 0)
		return 0;

	try {
		// Creates object of ImagePrepartion class 
		// 4 paths to xml datasets are being passed as arguments for the class constructor
	        #if defined(_WIN32) || (_WIN64)	
                PREP imprepPtr = (PREP)GetProcAddress(dllFile, "Preparation");
               
		// Initializes imported class from dll
		GenericPreparation* genPrep = imprepPtr(
                        params.at("faces"),
			params.at("eyes"),
			params.at("noses"),
			params.at("mouths"),
			params.at("directory"),
			{ ".jpeg", ".jpg", ".png" },
			atoi(params.at("threads").c_str())
                );

		// The main process has been launched. It executes images analyzing and processing of them.
		// In the end of its work it saves faces in their own files and outputs statistics into json file
		genPrep->run();

		delete genPrep;
                #else
                PREP* imprepPtr = (PREP*)dlsym(handle, "create");                      
                if (!imprepPtr)
                {
                    cout<<"The error is %s"<<dlerror();
                }
               
                GenericPreparation* genPrep = imprepPtr(
                        params.at("faces"),
			params.at("eyes"),
			params.at("noses"),
			params.at("mouths"),
			params.at("directory"),
			{ ".jpeg", ".jpg", ".png" },
			atoi(params.at("threads").c_str())
                );

                genPrep->run();

                dlclose(handle); 
                #endif
	}
	catch (std::exception& e) {
		cerr << "Error. The details are " << e.what() << endl;
	}

    return 0;
}

