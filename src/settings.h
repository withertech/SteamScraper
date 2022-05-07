/***************************************************************************
 *            settings.h
 *
 *  Wed Jun 14 12:00:00 CEST 2017
 *  Copyright 2017 Lars Muldjord
 *  muldjordlars@gmail.com
 ****************************************************************************/
/*
 *  This file is part of skyscraper.
 *
 *  skyscraper is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  skyscraper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with skyscraper; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>

struct Settings {
  QString currentDir = "";

  QString cacheFolder = "";
  QString gameListFileString = "";
  QString skippedFileString = "";
  QString configFile = "";
  QString frontend = "emulationstation";
  QString frontendExtra = ""; // For use with 'emulator' with Attractmode and 'command' for Pegasus
  QString platform = "";
  QString scraper = "";
  QString userCreds = "";
  QString igdbToken = "";
  QString inputFolder = "";
  QString gameListFolder = "";
  QString mediaFolder = "";
  bool mediaFolderHidden = false; // Only relevant for EmulationStation
  QString screenshotsFolder = "";
  QString coversFolder = "";
  QString logosFolder = "";
  QString marqueesFolder = "";
  QString steamgridsFolder = "";
  QString heroesFolder = "";
  QString videosFolder = "";
  QString importFolder = "import";
  QString nameTemplate = "";
  int doneThreads = 0;
  int threads = 4;
  bool threadsSet = false;
  int minMatch = 65;
  bool minMatchSet = false;
  int notFound = 0;
  int found = 0;
  int currentFile = 0;
  int totalFiles = 0;
  int maxLength = 2500;
  bool brackets = true;
  bool refresh = false;
  QString cacheOptions = "";
  bool cacheResize = true;
  int jpgQuality = 95;
  bool subdirs = true;
  bool onlyMissing = false;
  QString startAt = "";
  QString endAt = "";
  bool pretend = false;
  bool unattend = false;
  bool unattendSkip = false;
  bool interactive = false;
  bool forceFilename = false;
  bool stats = false;
  bool hints = true;
  int verbosity = 0;
  int maxFails = 42;
  bool skipped = false;
  QString artworkConfig = "artwork.xml";
  QByteArray artworkXml = "";
  QString excludePattern = "";
  QString includePattern = "";
  QString includeFrom = "";
  QString excludeFrom = "";
  bool relativePaths = false;
  QString extensions = "";
  QString addExtensions = "";
  bool unpack = false;
  bool theInFront = false;
  bool gameListBackup = false;
  bool preserveOldGameList = true;
  bool spaceCheck = true;
  QString scummIni = "";

  int romLimit = -1;

  bool videos = false;
  bool videoPreferNormalized = true;
  int videoSizeLimit = 100*1024*1024;
  QString videoConvertCommand = "";
  QString videoConvertExtension = "";
  bool symlink = false;
  bool skipExistingVideos = false;
  bool cacheCovers = true;
  bool skipExistingCovers = false;
  bool cacheScreenshots = true;
  bool skipExistingScreenshots= false;
  bool cropBlack = true; // If true, will crop away black borders around screenshots
  bool cacheLogos = true;
  bool skipExistingLogos = false;
  bool cacheMarquees = true;
  bool skipExistingMarquees = false;
  bool cacheSteamgrids = true;
  bool skipExistingSteamgrids = false;
  bool cacheHeroes = true;
  bool skipExistingHeroes = false;
  
  QString user = "";
  QString password = "";
  QString lang = "";
  QString region = "";
  QString langPriosStr = "";
  QString regionPriosStr = "";

  QString searchName = "";

  QMap<QString, QImage> resources;
  QMap<QString, QString> mameMap;
  QMap<QString, QString> aliasMap;
  QMap<QString, QPair<QString, QString> > whdLoadMap;

  QList<QString> regionPrios;
  QList<QString> langPrios;

};

#endif // SETTINGS_H
