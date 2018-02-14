
# Description
## About
The program makes the following:
1. Searches pictures (jpg, png, jpeg extensions) in a given directory including all subdirectories. 
2. Saves detected faces on the found pictures in their own personal file. The file is located in the same directory where the original file is.
3. Details about detected faces are being saved in result.json file. The file is located in the same directory where the files of faces are.

## How to launch
You may launch the solution in a few modes:
1. Not to pass any parameters. Just enter a program's executuble file name. Upon the launch you will see help message. The message shows all possible parameters:

`C:\Users\Ruslan\Source\Repos\facedetector\LaunchTheProgram\x64>Detector.exe
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
  -t [ --threads ] arg (=2)             number of threads`
