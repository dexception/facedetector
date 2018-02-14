#pragma once

#include "stdafx.h"

#if defined (_WIN32) || defined (_WIN64)
	#ifdef SYS_TIME_DLL_EXPORTS
	#define FACE_DETECTOR __declspec(dllexport)
	#else
	#define FACE_DETECTOR __declspec(dllimport)
	#endif
#elif defined(__GNUC__)
#define FACE_DETECTOR __attribute__((visibility("default")))
#define FACE_DETECTOR
#else
//  need to analyze deeper the case building the solution by different complers
#define FACE_DETECTOR
#define FACE_DETECTOR
#pragma warning Unknown dynamic link import/export semantics.
#endif

#include "opencv2/highgui.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"

#include "boost/filesystem.hpp"
#include "boost/format.hpp"
#include <boost/exception/all.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <algorithm>
#include <atomic>

#include "GenericDetector.h"

using namespace std;
using namespace cv;

namespace FaceDetector {
	class ImagePreparation : public GenericPreparation {
	public:
		/* Into the class constructor paths of xml datasets are being passed for:
		*       face
		*       eyes
		*       mouth
		*       nose
		*/
		ImagePreparation(string, string, string, string, string, vector<string>, int);

		virtual ~ImagePreparation();

		// Getters and setters for datasets
		string getFaceDataset();
		string getEyesdataset();
		string getMouthdataset();
		string getNoseDataset();

		/*  Runs image processing with such values as
		*  vector<string> - list of image files for processing
		*  int - quantity of being created threadpools
		*/
		void run();
		vector<string> getRecursiveFilepaths(string dir_path, vector<string> exts); //returns list of files inside of parameter's directory including nested catalogs 

		static vector<string> getNonRecursiveFilepaths(string dir_path, vector<string> exts); //returns list of files inside of parameter's directory (exts is a filemask)
	private:
		// datasets
		string faceXMLDatasetPath;
		string eyesXMLDatasetPath;
		string noseXMLDatasetPath;
		string mouthXMLDatasetPath;
		string faceDirPath;

		vector<string> extensions;
		int countThreads;
				
		// Returns unique names of catalogs where the images are
		static vector<string> getUniqueFolders(vector<string>&);

		// Initialization of logging
		static void initLog();
	};

	// image processing class executes analyze and next it outputs as separate image file with individual faces
	// and json files storing coordiantes of an individual face
	class ImageProcessing {
	public:
		/* Uniting method. Executes detection of all facial elements
		* The function parameters are:
		*   path to a being detected image
		*   xmls datasets in next order:
		*       face
		*       eye
		*       mouth
		*       nose
		*/

		static void detectAllElements(const string&,
			string, string, string, string
			);

		// Unites all jsons into 1 file "result.json"
		// The file is located in a root directory for pictures
		static void concatenateJsons(vector<string>&);

		// Returns quantity of found files
		static atomic_int getFoundFiles();

	private:
		static string imageFile;
		// Face detection	
		static void detectFaces(Mat&, vector<Rect_<int> >&, string);

		// Common function for facial features detection
		static void detectFacialFeatures(Mat&, const string, const vector<Rect_<int> >, string,
			string, string);

		// Nose detection
		static void detectNose(Mat&, vector<Rect_<int> >&, string);

		// Mouth detection
		static void detectMouth(Mat&, vector<Rect_<int> >&, string);

		// Eyes detection
		static void detectEyes(Mat&, vector<Rect_<int> >&, string);

		//Filename for the cropped face 
		static string getFaceFileName(const string, int);

		//Flipping and saving found face into a separate file
		static void flipSaveFace(string, Mat);

		// Makes a name for json output (for each face)
		static string getJSONFilename(const string);

		// Writes into a json file (for each face) details
		static void getFacesDetailsInJSON(const string, map<string, vector<string> >);

		// Increments counter of found correct files
		static void incrementFoundFile(atomic_int);

		// Removes temporary json files
		static void removeTempJSONFiles(vector<string>);
	};	
}

extern "C" {
	FACE_DETECTOR GenericPreparation* _cdecl Preparation(string, string, string, string, string, vector<string>, int);
};