
### A couple of introductory words
The English langualge isn't a native one for me. Hence, the text below might contain some gramatical or lexical mistakes.

# Description
## About
The program makes the following:
1. Searches pictures (jpg, png, jpeg extensions) in a given directory including all subdirectories. 
2. Saves detected faces on the found pictures in their own personal files. The files are located in the same directory where the 
original file is.
3. Details about detected faces are being saved in result.json file. The file is located in the same directory where the files 
of faces are.

## How to launch
You may launch the solution in a few modes:
1. Not to pass any parameters. Just enter a program's executuble file name. Upon the launch you will see help message. The message shows 
all possible parameters:

```C:\Users\Ruslan\Source\Repos\facedetector\LaunchTheProgram\x64>Detector.exe
  /$$$$$$$ /$$$$$$$$/$$$$$$$$/$$$$$$$$ /$$$$$$ /$$$$$$$$/$$$$$$ /$$$$$$$
 | $$__  $| $$_____|__  $$__| $$_____//$$__  $|__  $$__/$$__  $| $$__  $$
 | $$  | $| $$        | $$  | $$     | $$  |__/  | $$ | $$    $| $$    $$
 | $$  | $| $$$$$     | $$  | $$$$$  | $$        | $$ | $$  | $| $$$$$$$
 | $$  | $| $$__/     | $$  | $$__/  | $$        | $$ | $$  | $| $$__  $$
 | $$  | $| $$        | $$  | $$     | $$    $$  | $$ | $$  | $| $$    $$
 | $$$$$$$| $$$$$$$$  | $$  | $$$$$$$|  $$$$$$/  | $$ |  $$$$$$| $$  | $$
 |_______/|________/  |__/  |________/ ______/   |__/    _____/|__/  |__/



***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****

Allowed options:

Generic options:
  -h [ --help ]                         produce help message
  -c [ --config ] arg                   name of a file of a configuration.

Configuration:
  -f [ --faces ] arg (=datasets/haarcascade_frontalface_alt.xml)
                                        path to fases detection file
  -e [ --eyes ] arg (=datasets/haarcascade_eye.xml)
                                        path to eyes detection file
  -m [ --mouths ] arg (=datasets/haarcascade_mcs_mouth.xml)
                                        path to mouths detection file
  -n [ --noses ] arg (=datasets/haarcascade_mcs_nose.xml)
                                        path to noses detection file

can be passed without keys:
  -d [ --directory ] arg                input directory
  -t [ --threads ] arg (=2)            number of threads
  ```
2. Pass minimum parameters. It is a path to a catalogue where pictures are located. Other paremeters have got default values.
3. Pass as many allowed parameters as you want. But the minimum one is a path to pictures.
4. Use a config file. To switch on usage of the file is possible via `-c` or `--config` parameter.
In the file you may enumerate all or just a few parameters. The rest ones will be taken by default.

_Important_: arguments for input directory or threads can be passed without any parameters' key.
For instance, you need to make some survey about pictures in `C:\pictures` by leveraging 4 threads:
`Detector.exe c:\pictures 4`

# Technical information
## Libraries and tools
During the implementation development I used the next tools and frameworks:
* __OS__: Microsoft Windows 8.1
* __IDE__/__compiler__: Microsoft Visual Studio 2015
* __Additional libraries__: 
  boost 1.60.0b
  opencv 3.2
  rapidjson 1.1.0
* __WinAPI__: a couple of WinApi functions were leveraged during call functions from DLL file.

## Main components of the solution
The solution is represented by a library file FaceDetectionLib.dll and laucher Detector.exe.

## Short description of the source codes and methods
### Launcher
It comprises 2 files:
1. Detector.cpp. Contains a method of parsing line arguments `clParse` and `main` method
2. GenericDetector.h. Represents an interface which helps to bind Detector and the DLL-file

### Library
It saves main logic for the program execution. The library includes a few files:
1. FaceDetectorLib.h. An interface shaping a structure for 2 classes in the library
2. GenericDetector.h. The same one like for the launcher
3. ImagePreparation.cpp. A class which takes command-line parsed parameters. It prepares the solution for further processing:
  * Collects paths to pictures including their own filenames in one catalogue or including the nested ones;
  * Makes a list of unique directories;
  * Launches logging mechanism;
  * Makes getters for datasets file
 The file also the analysis process by method `run()`. Main parameters for the detection are quantity of threads (default value is 2) and list of paths to pictures for faces detection. 
 4. ImageProcessing.cpp. It executes function of facial elements detection. The "found faces" are being saved into their own personal files and some data about faces is being stored inside result.json file. The unit also contains some auxiliary methods making some file operations (removal files management)
 
 ## Directories
 1. Detector. Saves the laucnher's source codes
 2. FaceDescriptionLib. Contains the library program codes
 3. LaunchTheProgram. Binary files of the library and the laucner are in the directory. The binaries are compiled for x86 and x64 architecture and located in their own "architectural" directories x86 and x64 respectively. Datasets and about 10 image files are represented in those directories too.
