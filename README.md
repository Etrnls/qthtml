This is the Qt HTML backend utilizing the Qt Platform Abstraction (QPA).
Compile against Qt5

[Video Demo](http://www.youtube.com/watch?v=qDyWjNju1yU)

See also: [Gtk+ HTML backend (broadway)](https://www.google.com/search?q=gtk%20html%20backend)

Dependancy on Ubuntu 14.04:
 - apt-get install qt5-default qtbase5-private-dev libxrender-dev libfontconfig1-dev libfreetype6-dev libudev-dev libglib2.0-dev

How to build:
 - qmake qthtml.pro
 - make

Compiled library will be at ./plugins/platforms/libqthtml.so

How to run Qt5 based application with plugin:
    - copy plugin to [QT_DIR]/plugins/platforms/
	for example for Ubuntu 14.04 QT_DIR is 
	    /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms
    - or (if you don't have root rights or don't want to change original qt installation) perform command “export QT_QPA_PLATFORM_PLUGIN_PATH="path_where_plugin_is_located" ” in console where qt5 application will run from
    - qt5_app -platform html
    - application will be accessible at 127.0.0.1:8080 in any HTML5 capable web browser
