# Escape the University
is a 3D single player stealth game by [Manuel T. Schrempf 0920136](mailto:e0920136@student.tuwien.ac.at) and [Stefan C. Wilker
0920293](mailto:e0920293@student.tuwien.ac.at) done in the course [Computergraphics 2017](https://tiss.tuwien.ac.at/course/educationDetails.xhtml?windowId=9cd&semester=2017S&courseNr=186831) on the [Technical University of Vienna](http://www.tuwien.ac.at/en/tuwien_home/). Both participants are enrolled in curriulum of 066 935 Medieninformatik.

## Build
1. Have or buy [Windows 8.1+](https://www.microsoft.com/en-us/windows) and an [AMD graphics card](https://www.amd.com/en)
2. Download [Virtual Studio 2015](https://www.visualstudio.com/) (VS15)
3. Download the [master branch](https://github.com/ManuelTS/cgue17-escapeTheUniversity.git)
4. In VS15 set _Tools/Options/Debugging/Symbols/Microsoft Symbol Servers_ to `true`
5. Right Click on the project _escapeTheUniversity_
  - On the upper left set Configuration to _All Configurations_
  - Enter in _Configuration Properties/Debugging/Working directory_ `$(SolutionDir)`
6. In VS15 select the build mode `release` or `debug` and `x86` as platform
7. Build the game and play
