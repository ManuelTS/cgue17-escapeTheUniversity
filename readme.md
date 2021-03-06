# Escape the University
is a 3D single player stealth game by [Manuel T. Schrempf 0920136](mailto:e0920136@student.tuwien.ac.at) and [Stefan C. Wilker
0920293](mailto:e0920293@student.tuwien.ac.at) done in the course [Computergraphics 2017](https://tiss.tuwien.ac.at/course/educationDetails.xhtml?windowId=9cd&semester=2017S&courseNr=186831) on the [Technical University of Vienna](http://www.tuwien.ac.at/en/tuwien_home/). Both participants are enrolled in curriulum of 066 935 Medieninformatik.

## Build
1. Have or buy [Windows 8.1+](https://www.microsoft.com/en-us/windows) and an [AMD graphics card](https://www.amd.com/en)
2. Download and install [Virtual Studio 2015](https://www.visualstudio.com/) (VS15)
    1. Make sure C++11 or higher is installed or gets installed with VS15, since [VS 15 or higher C++ is not installed with the VS by default](https://blogs.msdn.microsoft.com/vcblog/2015/07/24/setup-changes-in-visual-studio-2015-affecting-c-developers/)
3. Download the [master branch](https://github.com/ManuelTS/cgue17-escapeTheUniversity.git)
4. In VS15 set _Tools/Options/Debugging/Symbols/Microsoft Symbol Servers_ to `true`
5. Right Click on the project _escapeTheUniversity_
    1. On the upper left set Configuration to _All Configurations_
    1. Enter in _Configuration Properties/Debugging/Working directory_ `$(SolutionDir)`
6. In VS15 select the build mode `release` or `debug` and `x86` as platform
7. Build the game and play
