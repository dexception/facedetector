#include "stdafx.h"

#include <vector>
#include <iostream>
#include <map>
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

#include "FaceDetector.h"

using namespace std;
using namespace cv;
using namespace boost;
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
	else {
		incrementFoundFile();
	}

	for (unsigned int i = 0; i < faces.size(); i++)
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
			for (unsigned int j = 0; j < nose.size(); j++)
			{
				Rect n = nose[j];				
				noseCenterHeight = (n.y + n.height / 2);
				
				// In case of many noses
				break;
			}
		}

		// Detect mouth if classifier provided by the user
		double mouthCenterHeight = 0.0;
		if (!mouthCascade.empty())
		{
			vector<Rect_<int> > mouth;
			detectMouth(ROI, mouth, mouthCascade);

			for (unsigned int j = 0; j < mouth.size(); j++)
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
				
				facialCoordinates.push_back(
					(boost::format("%s;upper left x:%d;upper left y:%d;width:%d;height:%d") % "mouth" % m.x % m.y % m.width % m.height).str()
					);

				// In case of many mouths
				break;
			}
		}

		// Adding all facial coordinates into faceData map
		faceData[faceFileName] = facialCoordinates;		
	}

	BOOST_LOG_TRIVIAL(info) << "File " << imageFile << " processing is complete. Found " << faces.size() << " faces";

	//Need to save faceData content into a separate tmp file
	//Each file should be created where the file is located
	// Saving the face's elements details into a json file
	getFacesDetailsIntoFile(imageFile, faceData);

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


// Outputs parameters of found face into json format
// ThE JSON's structure is represented below:
//{
//	fullPathName: {
//	  fullPathFace1: {
//		 face : {
//            "upper left x": number,
//				"upper left y": number,
//				"width": number,
//				"height": number
//            },
//		  mouth : {
//				the same parameters: figures
//			}
//   },
//   fullPathFace2 {
//   
//   	}
//   
//   }
// 
//}
void ImageProcessing::getFacesDetailsIntoFile(const string fullPath, map< string, vector<string> > params){
	//Extract a parent from fullPath
	boost::filesystem::path pathFile(fullPath);	
	boost::filesystem::path pathTemporaryFile(pathFile.parent_path());
	
	//Defines a file where coordinates should be saved
	pathTemporaryFile /= "result.json";
	
	std::ofstream ofs(pathTemporaryFile.c_str(), ios_base::app);
	
	string sourcePath = fullPath;
	
	#if defined (_WIN32) || (_WIN64)
		replace_all(sourcePath, "\\", "\\\\");
	#endif
	
	ofs << "\"" << sourcePath << "\":{" << endl;

	int countFace = 1;
	for (pair<string, vector< string >> component : params) {
			string imageFileName = component.first;
			
			#if defined (_WIN32) || (_WIN64)
				replace_all(imageFileName, "\\", "\\\\");
			#endif
			
			ofs << "    \"" << imageFileName << "\":{" << endl;
			
			int countComponent = 1;
			for (auto val : component.second) {
				vector<string> wholefaceSubstrings;
				boost::split(wholefaceSubstrings, val, boost::is_any_of(";"));				
				
				int countElement = 1;				
				for (auto sub : wholefaceSubstrings) {
					// Name of facial element
					if (countElement == 1) {
						ofs << "            \"" << sub << "\":{" << endl;
					}
					else {
						// Subelements
						vector <string> elements;
						boost::split(elements, sub, boost::is_any_of(":"));
						
						// Defines whether the last element
						if (countElement != wholefaceSubstrings.size())
							ofs << "                \"" << elements.at(0) << "\" : " << elements.at(1) << "," << endl;
						else
							ofs << "                \"" << elements.at(0) << "\" : " << elements.at(1) << "" << endl;
					}
					
					countElement++;
				}
				
				// Defines whether the last element
				if (countComponent != component.second.size())
					ofs << "            }," << endl;
				else
					ofs << "            }" << endl;
				
				countComponent++;
			}
			
			if (countFace != params.size())
				ofs << "        }," << endl;
			else
				ofs << "        }" << endl;
			
			countFace++;
	}
	
	//closing json array	
	ofs << "    }," << endl;	
	
	ofs.close();	
}

// Counts and returns prcessed correect file
// Correct means a file where a face is found
//void ImageProcessing::incrementFoundFile(atomic_int value) {
void ImageProcessing::incrementFoundFile() {	
	++foundFile;
	
}

int ImageProcessing::getFoundFiles() {
	return foundFile.load();
}
