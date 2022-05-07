project( SteamScraper )
cmake_minimum_required( VERSION 3.1 )
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_AUTOMOC ON )
set( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )
find_package ( Qt5Widgets REQUIRED )
find_package ( Qt5Core REQUIRED )
find_package ( Qt5Gui REQUIRED )
find_package ( Qt5Network REQUIRED )
find_package ( Qt5Xml REQUIRED )
file ( STRINGS ./VERSION VERSION )
add_compile_definitions( ${VERSION} )
set ( SteamScraper_HDRS
	src/xmlreader.h
	src/settings.h
	src/strtools.h
	src/imgtools.h
	src/cache.h
	src/gameentry.h
	src/crc32.h
	src/layer.h
	src/nametools.h
	src/queue.h
	)

set ( SteamScraper_SRCS
	src/main.cpp
	src/skyscraper.cpp
	src/netmanager.cpp
	src/netcomm.cpp
	src/xmlreader.cpp
	src/compositor.cpp
	src/strtools.cpp
	src/imgtools.cpp
	src/esgamelist.cpp
	src/scraperworker.cpp
	src/cache.cpp
	src/localscraper.cpp
	src/importscraper.cpp
	src/gameentry.cpp
	src/abstractscraper.cpp
	src/abstractfrontend.cpp
	src/emulationstation.cpp
	src/attractmode.cpp
	src/pegasus.cpp
		src/screenscraper.cpp
	src/crc32.cpp
        src/scripter.cpp
	src/platform.cpp
	src/layer.cpp
	src/fxshadow.cpp
	src/fxblur.cpp
	src/fxmask.cpp
	src/fxframe.cpp
	src/fxrounded.cpp
	src/fxstroke.cpp
	src/fxbrightness.cpp
	src/fxcontrast.cpp
	src/fxbalance.cpp
	src/fxopacity.cpp
	src/fxgamebox.cpp
	src/fxhue.cpp
	src/fxsaturation.cpp
	src/fxcolorize.cpp
	src/fxrotate.cpp
	src/fxscanlines.cpp
	src/nametools.cpp
	src/queue.cpp
	)

set ( SteamScraper_MOCS
	src/skyscraper.h
	src/netmanager.h
	src/netcomm.h
	src/compositor.h
	src/esgamelist.h
	src/scraperworker.h
	src/localscraper.h
	src/importscraper.h
	src/abstractscraper.h
	src/abstractfrontend.h
	src/emulationstation.h
	src/attractmode.h
	src/pegasus.h
		src/screenscraper.h
        src/scripter.h
	src/platform.h
	src/fxshadow.h
	src/fxblur.h
	src/fxmask.h
	src/fxframe.h
	src/fxrounded.h
	src/fxstroke.h
	src/fxbrightness.h
	src/fxcontrast.h
	src/fxbalance.h
	src/fxopacity.h
	src/fxgamebox.h
	src/fxhue.h
	src/fxsaturation.h
	src/fxcolorize.h
	src/fxrotate.h
	src/fxscanlines.h
	)
QT5_WRAP_CPP(MOCS ${SteamScraper_MOCS})

add_executable ( SteamScraper ${SteamScraper_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( SteamScraper  Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Xml )
