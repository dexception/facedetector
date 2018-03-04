#include "stdafx.h"

#include <vector>
#include <iostream>

#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/atomic.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>

#include "FaceDetector.h"

using namespace std;
using namespace boost::filesystem;
using namespace boost::asio;
using namespace FaceDetector;

ImagePreparation::ImagePreparation(
	string faceXMLDatasetFullPath,
	string eyesXMLDataSetFullPath,
	string noseXMLDataSetFullPath,
	string mouthXMLDataSetFullPath, 	
	string faceDirPath,
	vector<string> extensions,
	int countThreads): GenericPreparation() {
		
	// Datasets
	this->faceXMLDatasetPath = faceXMLDatasetFullPath;
	this->eyesXMLDatasetPath = eyesXMLDataSetFullPath;
	this->mouthXMLDatasetPath = mouthXMLDataSetFullPath;
	this->noseXMLDatasetPath = noseXMLDataSetFullPath;

	// Directory where pictures are located
	this->faceDirPath = faceDirPath;

	// Extensions for processed pictures
	this->extensions = extensions;

	// Count of threads
	this->countThreads = countThreads;

	initLog();
}

ImagePreparation::~ImagePreparation() {}

string ImagePreparation::getFaceDataset() {
	return this->faceXMLDatasetPath;
}

string ImagePreparation::getEyesdataset() {
	return this->eyesXMLDatasetPath;
}

string ImagePreparation::getMouthdataset() {
	return this->mouthXMLDatasetPath;
}

string ImagePreparation::getNoseDataset() {
	return this->noseXMLDatasetPath;
}

// Launch threads for image processing
void ImagePreparation::run() {
	// Ready, steady, go! The process has begun
	BOOST_LOG_TRIVIAL(info) << "The analysis has been started";

	// Getting fullpaths where images can be found	
	vector<string> filePaths (this->getRecursiveFilepaths(this->faceDirPath, this->extensions));

	// Shows how many pictures have been discovered
	BOOST_LOG_TRIVIAL(info) << filePaths.size() << " files found";

	io_service ioService;
	boost::thread_group threadpool; // threadpool	
	shared_ptr<io_service::work> work(new io_service::work(ioService)); // workers
		 
	ImageProcessing proc;

	for (int i = 0; i < this->countThreads; i++)
	{
		threadpool.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));		
	}

	BOOST_LOG_TRIVIAL(debug) << this->countThreads << " threads are created ";

	// json output. result.json should be located together with the souce image in the same catalog
	vector<string> imagesDirName = getImagesDirName(filePaths);
	
	// JSON file initialization. An opening curly bracket is inserted
	for (auto imageDir : imagesDirName) {		
		boost::filesystem::path pathTemporaryFile(imageDir);			
		pathTemporaryFile /= "result.json";
		
		std::ofstream ofs(pathTemporaryFile.c_str(), ios_base::app);
		ofs << "{" << endl;

		ofs.close();
	}

	boost::mutex mutex;
		    
    for (auto it : filePaths)     
	{
		ioService.post(boost::bind(&ImageProcessing::detectAllElements,			
			it,
			this->getFaceDataset(),
			this->getEyesdataset(),
			this->getMouthdataset(),
			this->getNoseDataset()
			));
		
		boost::lock_guard<boost::mutex> guard(mutex);
	}
	
	ioService.poll();
	ioService.stop();

	work.reset();
	threadpool.join_all();

	// Closing curly bracket is inserted
	for (auto imageDir : imagesDirName) {		
		boost::filesystem::path pathTemporaryFile(imageDir);			
		pathTemporaryFile /= "result.json";
		
		std::ifstream ifs(pathTemporaryFile.c_str(), std::ifstream::in);
		
		vector<string> lines;
		
		for (string line; getline(ifs, line);) {
			lines.push_back(line);
		}
		
		string lastString = lines.at(lines.size()-1);
				
		boost::replace_last(lastString, "},", "}");
		
		lines.at(lines.size()-1) = lastString;		
		
		std::ofstream ofs(pathTemporaryFile.c_str());
		
		for (auto it : lines) {
			ofs << it << endl;			
		}
		
		ofs << "}" << endl;

		ofs.close();
	}
	
	BOOST_LOG_TRIVIAL(info) << proc.getFoundFiles() << " files have been analyzed and processed";
	BOOST_LOG_TRIVIAL(info) << "The analysis has been completed";
	return;
}

// Returns unique foldernames and paths to them where images are located
vector<string> ImagePreparation::getImagesDirName(vector<string>& images)
{
	vector<string> result;

	for (auto& image : images) {
		path p(image);
		path parentPath = p.parent_path();
		result.push_back(parentPath.string());
	}

	// Leaves only unique paths to catalogs
	result.erase(
		unique(result.begin(), result.end()),
		result.end()
		);

	return result;
}

// Preparation of logging settings
void ImagePreparation::initLog()
{
	/* init boost log
	* 1. Add common attributes
	* 2. set log filter to trace
	*/
	boost::log::add_common_attributes();
	boost::log::core::get()->add_global_attribute("Scope",
		boost::log::attributes::named_scope());
	boost::log::core::get()->set_filter(
		boost::log::trivial::severity >= boost::log::trivial::trace
		);

	/* log formatter:
	* [TimeStamp] [ThreadId] [Severity Level] [Scope] Log message
	*/
	auto fmtTimeStamp = boost::log::expressions::
		format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
	auto fmtSeverity = boost::log::expressions::
		attr<boost::log::trivial::severity_level>("Severity");
	boost::log::formatter logFmt =
		boost::log::expressions::format("[%1%] [%2%] %3%")
		% fmtTimeStamp % fmtSeverity % boost::log::expressions::smessage;

	/* console sink */
	auto consoleSink = boost::log::add_console_log(std::clog);
	consoleSink->set_formatter(logFmt);

	/* fs sink */
	auto fsSink = boost::log::add_file_log(
		boost::log::keywords::file_name = "TestFaceDetection_%Y-%m-%d_%H-%M-%S.%N.log",
		boost::log::keywords::open_mode = std::ios_base::app);

	fsSink->set_formatter(logFmt);
	fsSink->locked_backend()->auto_flush(true);
}


// The search is non-recursive
// Returns a vector of nested paths to image files (extensions are shown in exts vector)
// The paths are absolute ones
// Executes recursive search
vector<string> ImagePreparation::getRecursiveFilepaths(string dirPath, vector<string> exts) {
	vector<string> dirs;

	try {
		for (recursive_directory_iterator i = recursive_directory_iterator(dirPath), end_iter; i != end_iter; ++i) {
			path p = i->path();
			string fileExtension = p.extension().string();

			for (auto& extIt : exts) {

				if (fileExtension.compare(extIt) == 0) {
					// Photos have been found
					string filename = p.filename().string();

					dirs.push_back(p.string());
				}
			}
		}
	}
	catch (std::exception &e) {
		cout << e.what() << "\n";
	}

	return dirs;
}

#if defined (_WIN32) || (_WIN64)
// Binding declaration beween an execution file and the dll's classes and methods
FACE_DETECTOR GenericPreparation* _cdecl Preparation(
	string faceXMLDatasetFullPath,
	string eyesXMLDataSetFullPath,
	string noseXMLDataSetFullPath,
	string mouthXMLDataSetFullPath,
	string imageDirPath,
	vector<string> exts,
	int threads
	)
{
	return new ImagePreparation(faceXMLDatasetFullPath, 
		eyesXMLDataSetFullPath,
		noseXMLDataSetFullPath,
		mouthXMLDataSetFullPath,
		imageDirPath,
		exts,
		threads
		);
}
#else
    extern "C" ImagePreparation* create(
	    string faceXMLDatasetFullPath,
	    string eyesXMLDataSetFullPath,
	    string noseXMLDataSetFullPath,
	    string mouthXMLDataSetFullPath,
	    string imageDirPath,
	    vector<string> exts,
	    int threads
    )
{
	return new ImagePreparation(faceXMLDatasetFullPath, 
		eyesXMLDataSetFullPath,
		noseXMLDataSetFullPath,
		mouthXMLDataSetFullPath,
		imageDirPath,
		exts,
		threads
		);
}
#endif