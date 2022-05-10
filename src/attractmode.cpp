/***************************************************************************
 *            attractmode.cpp
 *
 *  Wed Jun 18 12:00:00 CEST 2017
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

#include "attractmode.h"
#include "nametools.h"
#include "strtools.h"

#include <QDate>

AttractMode::AttractMode()
{
}

bool AttractMode::loadOldGameList(const QString &gameListFileString)
{
	QFile gameListFile(gameListFileString);
	if (gameListFile.open(QIODevice::ReadOnly))
	{
		while (!gameListFile.atEnd())
		{
			QList<QByteArray> snippets = gameListFile.readLine().split(';');
			if (snippets.length() >= 17)
			{
				if (snippets.at(0) == "#Name")
				{
					continue;
				}
				GameEntry entry;
				entry.baseName = snippets.at(0);
				// Do NOT get sqr and par notes here. They are not used by skipExisting
				entry.title = snippets.at(1);
				//entry.aMEmulator = snippets.at(2);
				entry.aMCloneOf = snippets.at(3);
				entry.releaseDate = snippets.at(4) + "0101";
				entry.publisher = snippets.at(5);
				entry.tags = snippets.at(6);
				entry.players = snippets.at(7);
				entry.aMRotation = snippets.at(8);
				entry.aMControl = snippets.at(9);
				entry.aMStatus = snippets.at(10);
				entry.aMDisplayCount = snippets.at(11);
				entry.aMDisplayType = snippets.at(12);
				entry.aMAltRomName = snippets.at(13);
				entry.aMAltTitle = snippets.at(14);
				entry.aMExtra = snippets.at(15);
				entry.aMButtons = snippets.at(16);
				oldEntries.append(entry);
			}
		}
		gameListFile.close();
		return true;
	}

	return false;
}

bool AttractMode::skipExisting(QList<GameEntry> &gameEntries, QSharedPointer<Queue> queue)
{
	gameEntries = oldEntries;

	printf("Resolving missing entries...");
	int dots = 0;
	for (int a = 0; a < gameEntries.length(); ++a)
	{
		dots++;
		if (dots % 100 == 0)
		{
			printf(".");
			fflush(stdout);
		}
		for (int b = 0; b < queue->length(); ++b)
		{
			if (gameEntries.at(a).baseName == queue->at(b).completeBaseName())
			{
				queue->removeAt(b);
				// We assume baseName is unique, so break after getting first hit
				break;
			}
		}
	}
	printf(" \033[1;32mDone!\033[0m\n");
	return true;
}

void AttractMode::preserveFromOld(GameEntry &entry)
{
	for (const auto &oldEntry: oldEntries)
	{
		if (oldEntry.baseName == entry.baseName)
		{
			if (entry.developer.isEmpty())
				entry.developer = oldEntry.developer;
			if (entry.publisher.isEmpty())
				entry.publisher = oldEntry.publisher;
			if (entry.players.isEmpty())
				entry.players = oldEntry.players;
			if (entry.description.isEmpty())
				entry.description = oldEntry.description;
			if (entry.rating.isEmpty())
				entry.rating = oldEntry.rating;
			if (entry.releaseDate.isEmpty())
				entry.releaseDate = oldEntry.releaseDate;
			if (entry.tags.isEmpty())
				entry.tags = oldEntry.tags;
			if (entry.aMCloneOf.isEmpty())
				entry.aMCloneOf = oldEntry.aMCloneOf;
			if (entry.aMRotation.isEmpty())
				entry.aMRotation = oldEntry.aMRotation;
			if (entry.aMControl.isEmpty())
				entry.aMControl = oldEntry.aMControl;
			if (entry.aMStatus.isEmpty())
				entry.aMStatus = oldEntry.aMStatus;
			if (entry.aMDisplayCount.isEmpty())
				entry.aMDisplayCount = oldEntry.aMDisplayCount;
			if (entry.aMDisplayType.isEmpty())
				entry.aMDisplayType = oldEntry.aMDisplayType;
			if (entry.aMAltRomName.isEmpty())
				entry.aMAltRomName = oldEntry.aMAltRomName;
			if (entry.aMAltTitle.isEmpty())
				entry.aMAltTitle = oldEntry.aMAltTitle;
			if (entry.aMExtra.isEmpty())
				entry.aMExtra = oldEntry.aMExtra;
			if (entry.aMButtons.isEmpty())
				entry.aMButtons = oldEntry.aMButtons;
			break;
		}
	}
}

void AttractMode::assembleList(QString &finalOutput, QList<GameEntry> &gameEntries)
{
	int dots = 0;
	// Always make dotMod at least 1 or it will give "floating point exception" when modulo
	int dotMod = gameEntries.length() * 0.1 + 1;

	finalOutput.append(
	    "#Name;Title;Emulator;CloneOf;Year;Manufacturer;Category;Players;Rotation;Control;Status;DisplayCount;DisplayType;AltRomname;AltTitle;Extra;Buttons\n");
	for (auto &entry: gameEntries)
	{
		if (dots % dotMod == 0)
		{
			printf(".");
			fflush(stdout);
		}
		dots++;
		// Preserve certain data from old game list entry, but only for empty data
		preserveFromOld(entry);

		finalOutput.append(entry.baseName + ";" +
		                   entry.title);
		finalOutput.append(";" +
		                   emuInfo.completeBaseName() + ";" +
		                   entry.aMCloneOf + ";" +
		                   QDate::fromString(entry.releaseDate, "yyyyMMdd").toString("yyyy") + ";" +
		                   entry.publisher + ";" +
		                   entry.tags + ";" +
		                   entry.players + ";" +
		                   entry.aMRotation + ";" +
		                   entry.aMControl + ";" +
		                   entry.aMStatus + ";" +
		                   entry.aMDisplayCount + ";" +
		                   entry.aMDisplayType + ";" +
		                   entry.aMAltRomName + ";" +
		                   entry.aMAltTitle + ";" +
		                   entry.aMExtra + ";" +
		                   entry.aMButtons + ";\n");
		if (!entry.description.isEmpty() && saveDescFile)
		{
			QFile descFile(descDir.absolutePath() + "/" + entry.baseName + ".txt");
			if (descFile.open(QIODevice::WriteOnly))
			{
				descFile.write(entry.description.trimmed().toUtf8().left(config->maxLength));
				descFile.close();
			}
		}
	}
}

void AttractMode::checkReqs()
{
	if (config->frontendExtra.isEmpty())
	{
		printf("Frontend 'attractmode' requires emulator set with '-e'. Check '--help' for more information.\n");
		exit(0);
	}
	if (!config->frontendExtra.contains(".cfg"))
	{
		config->frontendExtra.append(".cfg");
	}


	emuInfo.setFile(config->frontendExtra);
	descDir.setPath(QDir::homePath() + "/.attract/scraper/" +
	                emuInfo.completeBaseName() + "/overview");
	if (descDir.exists())
	{
		saveDescFile = true;
	}
	else
	{
		if (descDir.mkpath(descDir.absolutePath()))
		{
			saveDescFile = true;
		}
	}

	printf("Looking for emulator cfg file:\n");

	if (checkEmulatorFile(config->frontendExtra))
	{
		return;
	}

	// For RetroPie this is linked directly to /opt/retropie/configs/all/attractmode/emulators/
	if (checkEmulatorFile(QDir::homePath() + "/.attract/emulators/" + config->frontendExtra))
	{
		return;
	}

	printf("Couldn't locate emulator cfg file, exiting...\n");
	exit(1);
}

bool AttractMode::checkEmulatorFile(QString fileName)
{
	QFileInfo info(fileName);
	printf("Trying '%s'... ", info.absoluteFilePath().toStdString().c_str());

	if (info.exists() && info.isFile())
	{
		config->frontendExtra = info.absoluteFilePath();
		printf("\033[1;32mFound!\033[0m\n\n");
		return true;
	}
	else
	{
		printf("Not found!\n");
		return false;
	}
}

bool AttractMode::canSkip()
{
	return true;
}

QString AttractMode::getGameListFileName()
{
	QFileInfo info(config->frontendExtra);
	return QString(info.completeBaseName() + ".txt");
}

QString AttractMode::getInputFolder()
{
	return QString(QDir::homePath() + "/RetroPie/roms/" + config->platform);
}

QString AttractMode::getGameListFolder()
{
	// For RetroPie this is linked directly to /opt/retropie/configs/all/attractmode/romlists/
	if (QFileInfo::exists(QDir::homePath() + "/.attract/romlists"))
	{
		return QString(QDir::homePath() + "/.attract/romlists");
	}

	return config->inputFolder;
}

QString AttractMode::getCoversFolder()
{
	QString type = "flyer";
	QString mediaTypeFolder = getMediaTypeFolder(type);
	if (mediaTypeFolder.isEmpty())
	{
		mediaTypeFolder = config->mediaFolder + (config->mediaFolder.right(1) == "/" ? "" : "/") + type;
	}
	return mediaTypeFolder;
}

QString AttractMode::getScreenshotsFolder()
{
	QString type = "snap";
	QString mediaTypeFolder = getMediaTypeFolder(type);
	if (mediaTypeFolder.isEmpty())
	{
		mediaTypeFolder = config->mediaFolder + (config->mediaFolder.right(1) == "/" ? "" : "/") + type;
	}
	return mediaTypeFolder;
}

QString AttractMode::getLogosFolder()
{
	QString type = "wheel";
	QString mediaTypeFolder = getMediaTypeFolder(type);
	if (mediaTypeFolder.isEmpty())
	{
		mediaTypeFolder = config->mediaFolder + (config->mediaFolder.right(1) == "/" ? "" : "/") + type;
	}
	return mediaTypeFolder;
}

QString AttractMode::getMarqueesFolder()
{
	QString type = "marquee";
	QString mediaTypeFolder = getMediaTypeFolder(type);
	if (mediaTypeFolder.isEmpty())
	{
		mediaTypeFolder = config->mediaFolder + (config->mediaFolder.right(1) == "/" ? "" : "/") + type;
	}
	return mediaTypeFolder;
}

QString AttractMode::getVideosFolder()
{
	QString type = "video";
	QString mediaTypeFolder = getMediaTypeFolder(type);
	if (mediaTypeFolder.isEmpty())
	{
		mediaTypeFolder = getMediaTypeFolder("snap", true);
	}
	if (mediaTypeFolder.isEmpty())
	{
		mediaTypeFolder = config->mediaFolder + (config->mediaFolder.right(1) == "/" ? "" : "/") + type;
	}
	return mediaTypeFolder;
}

QString AttractMode::getMediaTypeFolder(QString type, bool detectVideoPath)
{
	QString mediaTypeFolder = "";
	QFile emulatorFile(config->frontendExtra);

	if (emulatorFile.exists() && emulatorFile.open(QIODevice::ReadOnly))
	{
		while (!emulatorFile.atEnd())
		{
			QByteArray line = emulatorFile.readLine();
			line = line.trimmed();
			line.replace("~", QDir::homePath().toUtf8());
			line.replace("$HOME", QDir::homePath().toUtf8());
			QString lookFor = "artwork";
			if (line.left(lookFor.length()) == lookFor)
			{
				line = line.remove(0, lookFor.length()).trimmed();
				if (line.left(type.length()) == type)
				{
					line = line.remove(0, type.length()).trimmed();
					QList<QByteArray> paths = line.split(';');
					// This is some weird case where the 'snap' artwork line can contain multiple paths
					// and one of those paths is actually the video path. It was reported in an issue
					// and this is seemlingly how it should work for those cases where the user does not
					// use an 'artwork video' line.
					if (detectVideoPath)
					{
						for (const auto &path: paths)
						{
							if (path.contains("video"))
							{
								mediaTypeFolder = path;
								break;
							}
						}
					}
					else
					{
						mediaTypeFolder = paths.first();
					}
				}
			}
		}
		emulatorFile.close();
	}

	return mediaTypeFolder;
}

/*
Emulator files: /opt/retropie/configs/all/attractmode/emulators
Romlists: /opt/retropie/configs/all/attractmode/romlists

---- Example emulator file 1 begin ----
executable /opt/retropie/supplementary/runcommand/runcommand.sh
args 0 _SYS_ snes "[romfilename]"
rompath /home/pi/RetroPie/roms/snes
system Super Nintendo
romext .7z;.bin;.bs;.smc;.sfc;.fig;.swc;.mgd;.zip;.7Z;.BIN;.BS;.SMC;.SFC;.FIG;.SWC;.MGD;.ZIP
artwork flyer /home/pi/RetroPie/roms/snes/flyer
artwork marquee /home/pi/RetroPie/roms/snes/marquee
artwork snap /home/pi/RetroPie/roms/snes/snap
artwork wheel /home/pi/RetroPie/roms/snes/wheel
---- End ----

---- Example emulator file 2 begin ----
# Generated by Attract-Mode v2.2.1
#
executable           /opt/retropie/supplementary/runcommand/runcommand.sh
args                 0 _SYS_ amiga "[romfilename]"
rompath              /home/pi/RetroPie/roms/amiga
romext               .adf;.adz;.dms;.ipf;.uae;.zip;.sh;.ADF;.ADZ;.DMS;.IPF;.UAE;.ZIP;.SH
system               Commodore Amiga
info_source          thegamesdb.net
artwork    boxart          /home/pi/RetroPie/roms/amiga/boxart
artwork    cartart         /home/pi/RetroPie/roms/amiga/cartart
artwork    snap            /home/pi/RetroPie/roms/amiga/snap
artwork    wheel           /home/pi/RetroPie/roms/amiga/wheel
---- End ----

---- Example romlist entry begin ----
#Name;Title;Emulator;CloneOf;Year;Manufacturer;Category;Players;Rotation;Control;Status;DisplayCount;DisplayType;AltRomname;AltTitle;Extra;Buttons
Bubble Bobble;Bubble Bobble;Commodore Amiga;;1987;Firebird / Taito;arcade, cartoonish, cooperative, cute, jumponthings, platform, powerup, single;1 - 2 (2 simultaneous);0;;;;;;;;;
Games - Winter Edition;The Games: Winter Edition;Commodore Amiga;;1989;U.S. Gold / Epyx;multievent, olympics, sports;1 - 8 (1 simultaneous);0;;;;;;;;;
---- End ----
*/
