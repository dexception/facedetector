#include "stdafx.h"

#include <vector>
#include <iostream>
#include <map>
#include <cstdio>
#include <atomic>

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include "FaceDetector.h"

using namespace std;
using namespace cv;
using namespace boost;
using namespace rapidjson;
using namespace FaceDetector;

// Stores found files
static atomic_int foundFile(0);

// The way of solution is taken from https://github.com/samyak-268/facial-feature-detection/blob/master/facial_features.cpp

void ImageProcessing::detectAllElements(const string& imagePath,
	string faceDataset,
	string eyesDataset,
	string noseDataset,
	string mouthDataset) {
	// Loads images and cascade classifier files
	Mat image(imread(imagePath));
		
	// Detects faces and facial features
	vector<Rect_<int>> faces;

	detectFaces(image, faces, faceDataset);
		
	string path = imagePath;
	detectFacialFeatures(image, path, faces, eyesDataset, noseDataset, mouthDataset);

	return;
};

void ImageProcessing::detectFaces(Mat& img, vector<Rect_<int> >& faces, string faceDataset)
{
	CascadeClassifier faceCascade;
	faceCascade.load(faceDataset);

	faceCascade.detectMultiScale(img, faces, 1.15, 3, 0 | CASCADE_SCALE_IMAGE, Size(10, 10));
	return;
}

// Common function detecting facial features and drawing rectangles around pointed objects
void ImageProcessing::detectFacialFeatures(Mat& img, const string imageFile, const vector<Rect_<int> > faces, string eyeCascade,
	string noseCascade, string mouthCascade)
{
	/* Stores coordinates about a found face. The format is
	/path/to/found_face_filename.jpg - where detected face is being stored
	face - "upper left point (x,y);width;height"
	eyes - "eye1 left point (x,y);width;height"
	"eye2 left point (x,y);width;height"
	mouth - "mouth left point (x,y);width;height"
	*/
	map<string, vector<string>> faceData;
	BOOST_LOG_TRIVIAL(info) << "File " << imageFile << " is being processed";

	// No face is detected
	if (faces.size() == 0) {
		BOOST_LOG_TRIVIAL(info) << "No face is detected!!!!";
		return;
	}
	else
		incrementFoundFile(1);

	for (unsigned int i = 0; i < faces.size(); ++i)
	{
		vector<string> facialCoordinates;

		// Mark the bounding box enclosing the face
		Rect face = faces[i];
		rectangle(img, Point(face.x, face.y), Point(face.x + face.width, face.y + face.height),
			Scalar(255, 0, 0), 1, 4);

		// Eyes, nose and mouth will be detected inside the face (region of interest)
		Mat ROI = img(Rect(face.x, face.y, face.width, face.height));

		facialCoordinates.push_back(
			(boost::format("%s;upper left x:%d;upper left y:%d;width:%d;height:%d") % "face" % face.x % face.y % face.width % face.height).str()
			);


		// Found face need to save into a separate file. 
		string faceFileName = getFaceFileName(imageFile, i);
				
		flipSaveFace(faceFileName, ROI);

		// Check if all features (eyes, nose and mouth) are being detected
		bool isFullDetection = false;
		if ((!eyeCascade.empty()) && (!noseCascade.empty()) && (!mouthCascade.empty()))
			isFullDetection = true;

		// Detect eyes if classifier provided by the user
		if (!eyeCascade.empty())
		{
			vector<Rect_<int> > eyes;
			detectEyes(ROI, eyes, eyeCascade);

			// Mark points corresponding to the centre of the eyes
			for (unsigned int j = 0; j < eyes.size(); ++j)
			{
				Rect e = eyes[j];
				//circle(ROI, Point(e.x + e.width / 2, e.y + e.height / 2), 3, Scalar(0, 255, 0), -1, 8);
				rectangle(ROI, Point(e.x, e.y), Point(e.x + e.width, e.y + e.height), Scalar(0, 255, 0), 1, 4);

				// Saving eyes coordinates			
				facialCoordinates.push_back(
					(boost::format("%s%d;upper left x:%d;upper left y:%d;width:%d;height:%d") % "eye" % j % e.x % e.y % e.width % e.height).str()
					);
			}
		}

		// Detect nose if classifier provided by the user
		double noseCenterHeight = 0.0;
		if (!noseCascade.empty())
		{
			vector<Rect_<int> > nose;
			detectNose(ROI, nose, noseCascade);

			// Mark points corresponding to the centre (tip) of the nose
			for (unsigned int j = 0; j < nose.size(); ++j)
			{
				Rect n = nose[j];
				//rectangle(ROI, Point(n.x, n.y), Point(n.x + n.width, n.y + n.height), Scalar(0, 255, 0), 1, 4);
				noseCenterHeight = (n.y + n.height / 2);
			}
		}

		// Detect mouth if classifier provided by the user
		double mouthCenterHeight = 0.0;
		if (!mouthCascade.empty())
		{
			vector<Rect_<int> > mouth;
			detectMouth(ROI, mouth, mouthCascade);

			for (unsigned int j = 0; j < mouth.size(); ++j)
			{
				Rect m = mouth[j];
				mouthCenterHeight = (m.y + m.height / 2);

				// The mouth should lie below the nose
				if ((isFullDetection) && (mouthCenterHeight > noseCenterHeight))
				{
					rectangle(ROI, Point(m.x, m.y), Point(m.x + m.width, m.y + m.height), Scalar(0, 255, 0), 1, 4);
				}
				else if ((isFullDetection) && (mouthCenterHeight <= noseCenterHeight))
					continue;
				else
					rectangle(ROI, Point(m.x, m.y), Point(m.x + m.width, m.y + m.height), Scalar(0, 255, 0), 1, 4);

				// Saving mouth coordinates				
				facialCoordinates.push_back(
					(boost::format("%s;upper left x:%d;upper left y:%d;width:%d;height:%d") % "mouth" % m.x % m.y % m.width % m.height).str()
					);
			}
		}

		// Adding all facial coordinates into faceData map
		faceData[faceFileName] = facialCoordinates;
	}

	BOOST_LOG_TRIVIAL(info) << "File " << imageFile << " processing is complete. Found " << faces.size() << " faces";

	// Saving the face's elements details into a json file
	getFacesDetailsInJSON(imageFile, faceData);

	return;
}

void ImageProcessing::detectEyes(Mat& img, vector<Rect_<int> >& eyes, string cascadePath)
{
	CascadeClassifier eyesCascade;
	eyesCascade.load(cascadePath);

	eyesCascade.detectMultiScale(img, eyes, 1.20, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	return;
}

void ImageProcessing::detectNose(Mat& img, vector<Rect_<int> >& nose, string cascadePath)
{
	CascadeClassifier noseCascade;
	noseCascade.load(cascadePath);

	noseCascade.detectMultiScale(img, nose, 1.20, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	return;
}

void ImageProcessing::detectMouth(Mat& img, vector<Rect_<int> >& mouth, string cascadePath)
{
	CascadeClassifier mouthCascade;
	mouthCascade.load(cascadePath);

	mouthCascade.detectMultiScale(img, mouth, 1.20, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	return;
}

// Filename for a cropped face. The parameters are:
//   name of the source file
//   current number of the detected faces
// The output should be /sourcePath/sourceImage__N.jpg
string ImageProcessing::getFaceFileName(const string imageFile, int n) {
	boost::filesystem::path p(imageFile);

	// Extract fullpath for the file
	string file = absolute(p).string();
	string extensionSourceImage = p.extension().string();
	string extensionDestImage = ".jpg";

	// Making filename like /sourcePath/sourceImage__N.jpg
	boost::replace_last(
		file,
		extensionSourceImage,
		(boost::format("__%1%%2%") % n % extensionDestImage).str());

	return file;
}

// Flips found face and next is to save it into a separate file
// The face should be mirrored and flipped around a vertical axe before saving
void ImageProcessing::flipSaveFace(string file, Mat img) {
	Mat flipped;
	flip(img, flipped, 1);

	imwrite(file, flipped, { CV_IMWRITE_JPEG_QUALITY, 95 });
	return;
};

// Returns a json file name. The parameter is imagefilename
// Output: the same filename with JSON extension 
string ImageProcessing::getJSONFilename(const string imageFile) {
	boost::filesystem::path p(imageFile);

	// Extract fullpath for the file
	string file = absolute(p).string();
	string extensionImage = p.extension().string();

	// Making filename like /sourcePath/sourceImage__N.jpg
	boost::replace_last(
		file,
		extensionImage,
		".json"
		);

	return file;
}
// Outputs parameters of found face into json format
// ThE JSON's structure is represented below:
//{SourceFileName: fullPathName,
// Details:
// [
//	{ FaceFileName:fullPathFace1,
//    Description:
//		 [	 {Element: face,
//            upperLeft.x,  upperLeft.y, 
//			  upperRight.x, upperRight.y, 
//			  bottomLeft.x, bottomLeft.y,
//		      upperLeft.x,  upperLeft.y
//            },
//   
//			{Element: eye1,
//           upperLeft.x,  upperLeft.y, 
//           upperRight.x, upperRight.y, 
//		     bottomLeft.x, bottomLeft.y,
//		     upperLeft.x,  upperLeft.y
//          },
//		]		 
//   } ,
//   
//   { FaceFileName:fullPathFace2,
//   
//   ...
//   
//   }
// ]
//}
void ImageProcessing::getFacesDetailsInJSON(const string fullPath, map< string, vector<string> > params) {

	// Root json message definition
	Document document;

	// The document's object definition
	document.SetObject();

	// When the object neeeds to allocate memory, an allocator should be passed
	Document::AllocatorType& alloc = document.GetAllocator();

	// Initially soureFileName is being inserted
	//Value sourceFilename(full_path.c_str(), full_path.size(), alloc);
	document.AddMember("sourceFilename", fullPath, alloc);

	// Elements of faces into JSON format generation
	//map<string, vector< string >>::iterator it = params.begin();
	Value details(kArrayType);

	for (pair<string, vector< string >> component : params) {

		// Element of JSON array where parsed data is represented
		Value faceFileName(kObjectType);

		// Next face's filename is being added
		Value filename(component.first.c_str(), component.first.size(), alloc);
		faceFileName.AddMember("FaceFileName", filename, alloc);
		details.PushBack(faceFileName, alloc); // added line like "FaceFileName":"C:\\Users\\1.jpg"

		Value desription(kObjectType); // Description section in json

		Value elements(kArrayType); // array of lists. Upon its readiness it places into Description section

		for (auto val : component.second) {
			vector<string> wholefaceSubstrings; // stores values when map's value is being parsed

												 // Need to parse an array like
												 // element;number;number;number;number into
												 // Element: [number, number, number, number]
			boost::split(wholefaceSubstrings, val, boost::is_any_of(";"));

			//inside an array is responsible for 
			// {"Element":"face",
			//  "upper left x":"346",
			//	"upper left y" : "24",
			//	"width" : "159",
			//	"height" : "159"
			// }
			Value comp(kObjectType);

			// Name of element
			Value value(wholefaceSubstrings.at(0).c_str(), wholefaceSubstrings.at(0).size(), alloc);
			comp.AddMember("Element", value, alloc);

			for (unsigned int i = 1; i < wholefaceSubstrings.size(); i++) {
				vector<string> element; // a single element like "width:254" is parsed into ["width", "254"]
				boost::split(element, wholefaceSubstrings.at(i), boost::is_any_of(":"));

				Value elKey(element.at(0).c_str(), element.at(0).size(), alloc);
				Value elValue(element.at(1).c_str(), element.at(1).size(), alloc);

				comp.AddMember(elKey, elValue, alloc); // got formed into json
			}

			elements.PushBack(comp, alloc);
		}

		details.PushBack(elements, alloc);
	}

	document.AddMember("Details", details, alloc);

	// Writes the whole output into a json file
	// Makes filename like /sourcePath/sourceImage__N.jpg
	string jsonoutName = getJSONFilename(fullPath);

	ofstream fp(jsonoutName.c_str(), ios::binary);	

	OStreamWrapper osw(fp);
	Writer<OStreamWrapper> writer(osw);
	document.Accept(writer);

}

// Joins temporary json files into one result.json
// Every directory, where pictures have been found , has its own result.json
// Unique paths are passed as a param
void ImageProcessing::concatenateJsons(vector<string>& pathToJsons) {

	// Iterate through every directory where json is met
	for (auto& jsonPath : pathToJsons) {
		vector<string> files;

		Document docOutput;

		docOutput.SetObject();

		// When the object neeeds to allocate memory, an allocator should be passed
		Document::AllocatorType& alloc = docOutput.GetAllocator();

		Value arr(kArrayType); //next one is array should be passed to val(kObjectType)

		// List of temporary json files
		vector<string> foundJsonFiles = ImagePreparation::getNonRecursiveFilepaths(jsonPath, { ".json" });

		// Works non-completely
		// processing found jsons
		for (auto& itJson : foundJsonFiles) {
			ifstream ifs(itJson.c_str());
			IStreamWrapper isw(ifs);
			Document docInput;
			docInput.ParseStream(isw);

			Value val(docInput, alloc);

			arr.PushBack(val, alloc);
		}

		docOutput.AddMember("Result", arr, alloc);

		// Save everything into result.json
		boost::filesystem::path outputJson(jsonPath);
		outputJson /= "result.json";
		ofstream fp(outputJson.c_str(), ios::binary);		

		OStreamWrapper osw(fp);
		Writer<OStreamWrapper> writer(osw);
		docOutput.Accept(writer);

		removeTempJSONFiles(foundJsonFiles);
	}
};

// Removes temporary JSON files
void ImageProcessing::removeTempJSONFiles(vector<string> temporaryFiles) {
	for (auto& tempFile : temporaryFiles) {
		// Removes found file from fs except for result.json
		if (!tempFile.find("result.json")) {
			filesystem::wpath foundFile(tempFile.c_str());
			if (filesystem::exists(foundFile)) {
				filesystem::remove(foundFile);
			}
		}
	}
}

// Counts and returns prcessed correect file
// Correct means a file where a face is found
void ImageProcessing::incrementFoundFile(atomic_int value) {
	foundFile += value;
}

atomic_int ImageProcessing::getFoundFiles() {
	return foundFile.load();
}
