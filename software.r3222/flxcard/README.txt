- What is provided by the package

The package provides a low level C++ API for the control of the FELIX PCIe card. Documentation for the API van be found at: https://espace.cern.ch/ATLAS-FELIX/Shared%20Documents/SW_Documentation/FlxCard_API.docx?Web=1

- Author information

The S/W in this package is maintained by Markus Joos, CERN
Most of the code has been developed by other artists.

- How to build the package
Please have a look at: https://espace.cern.ch/ATLAS-FELIX/Shared%20Documents/SW_Documentation/flxcard_package.docx?Web=1

- Any necessary environment configuration (libraries that required etc.)
The package uses a device driver and some libraries from the ATLAS TDAQ project. For details please read: https://espace.cern.ch/ATLAS-FELIX/Shared%20Documents/SW_Documentation/FLX%20support%20SW.docx?Web=1

- Usage examples
Please have a look at: https://espace.cern.ch/ATLAS-FELIX/Shared%20Documents/SW_Documentation/flxcard_package.docx?Web=1




To compile the flxcard tools from source:

mkdir build
cd build
cmake ..
make

The executables are then created in the build folder

