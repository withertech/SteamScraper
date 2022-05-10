/***************************************************************************
 *            cache.cpp
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

#include <iostream>

#include <QBuffer>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>

#include "cache.h"
#include "nametools.h"
#include "queue.h"

Cache::Cache(const QString &cacheFolder)
{
	cacheDir = QDir(cacheFolder);
}

bool Cache::createFolders(const QString &scraper)
{
	if (scraper != "cache")
	{
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/covers/" + scraper))
		{
			return false;
		}
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/screenshots/" + scraper))
		{
			return false;
		}
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/logos/" + scraper))
		{
			return false;
		}
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/marquees/" + scraper))
		{
			return false;
		}
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/steamgrids/" + scraper))
		{
			return false;
		}
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/icons/" + scraper))
		{
			return false;
		}
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/heroes/" + scraper))
		{
			return false;
		}
		if (!cacheDir.mkpath(cacheDir.absolutePath() + "/videos/" + scraper))
		{
			return false;
		}
	}

	// Copy priorities.xml example file to cache folder if it doesn't already exist
	if (!QFileInfo::exists(cacheDir.absolutePath() + "/priorities.xml"))
	{
		QFile::copy("cache/priorities.xml.example",
		            cacheDir.absolutePath() + "/priorities.xml");
	}

	return true;
}

bool Cache::read()
{
	QFile quickIdFile(cacheDir.absolutePath() + "/quickid.xml");
	if (quickIdFile.open(QIODevice::ReadOnly))
	{
		printf("Reading and parsing quick id xml, please wait... ");
		fflush(stdout);
		QXmlStreamReader xml(&quickIdFile);
		while (!xml.atEnd())
		{
			if (xml.readNext() != QXmlStreamReader::StartElement)
			{
				continue;
			}
			if (xml.name() != "quickid")
			{
				continue;
			}
			QXmlStreamAttributes attribs = xml.attributes();
			if (!attribs.hasAttribute("filepath") ||
			    !attribs.hasAttribute("timestamp") ||
			    !attribs.hasAttribute("id"))
			{
				continue;
			}

			QPair<qint64, QString> pair;
			pair.first = attribs.value("timestamp").toULongLong();
			pair.second = attribs.value("id").toString();
			quickIds[attribs.value("filepath").toString()] = pair;
		}
		printf("\033[1;32mDone!\033[0m\n");
	}

	QFile cacheFile(cacheDir.absolutePath() + "/db.xml");
	if (cacheFile.open(QIODevice::ReadOnly))
	{
		printf("Reading and parsing resource cache, please wait... ");
		fflush(stdout);
		QXmlStreamReader xml(&cacheFile);
		while (!xml.atEnd())
		{
			if (xml.readNext() != QXmlStreamReader::StartElement)
			{
				continue;
			}
			if (xml.name() != "resource")
			{
				continue;
			}
			QXmlStreamAttributes attribs = xml.attributes();
			if (!attribs.hasAttribute("sha1") && !attribs.hasAttribute("id"))
			{
				printf("Resource is missing unique id, skipping...\n");
				continue;
			}

			Resource resource;
			if (attribs.hasAttribute("sha1"))
			{// Obsolete, but needed for backwards compat
				resource.cacheId = attribs.value("sha1").toString();
			}
			else
			{
				resource.cacheId = attribs.value("id").toString();
			}

			if (attribs.hasAttribute("source"))
			{
				resource.source = attribs.value("source").toString();
			}
			else
			{
				resource.source = "generic";
			}
			if (attribs.hasAttribute("type"))
			{
				resource.type = attribs.value("type").toString();
				addToResCounts(resource.source, resource.type);
			}
			else
			{
				printf("Resource with cache id '%s' is missing 'type' attribute, skipping...\n",
				       resource.cacheId.toStdString().c_str());
				continue;
			}
			if (attribs.hasAttribute("timestamp"))
			{
				resource.timestamp = attribs.value("timestamp").toULongLong();
			}
			else
			{
				printf("Resource with cache id '%s' is missing 'timestamp' attribute, skipping...\n",
				       resource.cacheId.toStdString().c_str());
				continue;
			}
			resource.value = xml.readElementText();
			if (resource.type == "cover" || resource.type == "screenshot" ||
			    resource.type == "logo" || resource.type == "marquee" ||
			    resource.type == "steamgrid" || resource.type == "icon" ||
			    resource.type == "hero" || resource.type == "video")
			{
				if (!QFileInfo::exists(cacheDir.absolutePath() + "/" + resource.value))
				{
					printf("Source file '%s' missing, skipping entry...\n",
					       resource.value.toStdString().c_str());
					continue;
				}
			}

			resources.append(resource);
		}
		cacheFile.close();
		resAtLoad = resources.length();
		printf("\033[1;32mDone!\033[0m\n");
		printf("Successfully parsed %d resources!\n\n", resources.length());
		return true;
	}
	return false;
}

void Cache::printPriorities(QString cacheId)
{
	GameEntry game;
	game.cacheId = cacheId;
	fillBlanks(game);
	printf("\033[1;34mCurrent resource priorities for this rom:\033[0m\n");
	printf("Title:          '\033[1;32m%s\033[0m' (%s)\n",
	       game.title.toStdString().c_str(),
	       (game.titleSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m") : game.titleSrc).toStdString().c_str());
	printf("Platform:       '\033[1;32m%s\033[0m' (%s)\n",
	       game.platform.toStdString().c_str(),
	       (game.platformSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m") : game.platformSrc).toStdString().c_str());
	printf("Release Date:   '\033[1;32m%s\033[0m' (%s)\n",
	       game.releaseDate.toStdString().c_str(),
	       (game.releaseDateSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m")
	                                      : game.releaseDateSrc)
	           .toStdString()
	           .c_str());
	printf("Developer:      '\033[1;32m%s\033[0m' (%s)\n",
	       game.developer.toStdString().c_str(),
	       (game.developerSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m")
	                                    : game.developerSrc)
	           .toStdString()
	           .c_str());
	printf("Publisher:      '\033[1;32m%s\033[0m' (%s)\n",
	       game.publisher.toStdString().c_str(),
	       (game.publisherSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m")
	                                    : game.publisherSrc)
	           .toStdString()
	           .c_str());
	printf("Players:        '\033[1;32m%s\033[0m' (%s)\n",
	       game.players.toStdString().c_str(),
	       (game.playersSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m") : game.playersSrc).toStdString().c_str());
	printf("Ages:           '\033[1;32m%s\033[0m' (%s)\n",
	       game.ages.toStdString().c_str(),
	       (game.agesSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m") : game.agesSrc).toStdString().c_str());
	printf("Tags:           '\033[1;32m%s\033[0m' (%s)\n",
	       game.tags.toStdString().c_str(),
	       (game.tagsSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m") : game.tagsSrc).toStdString().c_str());
	printf("Rating:         '\033[1;32m%s\033[0m' (%s)\n",
	       game.rating.toStdString().c_str(),
	       (game.ratingSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m") : game.ratingSrc).toStdString().c_str());
	printf("Cover:          '");
	if (game.coverSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.coverSrc.toStdString().c_str());
	}
	printf("Screenshot:     '");
	if (game.screenshotSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.screenshotSrc.toStdString().c_str());
	}
	printf("Logo:          '");
	if (game.logoSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.logoSrc.toStdString().c_str());
	}
	printf("Marquee:        '");
	if (game.marqueeSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.marqueeSrc.toStdString().c_str());
	}
	printf("Steamgrid:        '");
	if (game.steamgridSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.steamgridSrc.toStdString().c_str());
	}
	printf("Icon:        '");
	if (game.iconSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.iconSrc.toStdString().c_str());
	}
	printf("Hero:        '");
	if (game.heroSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.heroSrc.toStdString().c_str());
	}
	printf("Video:          '");
	if (game.videoSrc.isEmpty())
	{
		printf("\033[1;31mNO\033[0m' ()\n");
	}
	else
	{
		printf("\033[1;32mYES\033[0m' (%s)\n", game.videoSrc.toStdString().c_str());
	}
	printf("Description: (%s)\n'\033[1;32m%s\033[0m'",
	       (game.descriptionSrc.isEmpty() ? QString("\033[1;31mmissing\033[0m")
	                                      : game.descriptionSrc)
	           .toStdString()
	           .c_str(),
	       game.description.toStdString().c_str());
	printf("\n\n");
}

void Cache::editResources(QSharedPointer<Queue> queue,
                          const QString &command,
                          const QString &type)
{
	// Check sanity of command and parameters, if any
	if (!command.isEmpty())
	{
		if (command == "new")
		{
			if (type != "title" &&
			    type != "platform" &&
			    type != "releasedate" &&
			    type != "developer" &&
			    type != "publisher" &&
			    type != "players" &&
			    type != "ages" &&
			    type != "genres" &&
			    type != "rating" &&
			    type != "description")
			{
				printf("Unknown resource type '%s', please specify any of the following: 'title', 'platform', 'releasedate', 'developer', 'publisher', 'players', 'ages', 'genres', 'rating', 'description'.\n",
				       type.toStdString().c_str());
				return;
			}
		}
		else
		{
			printf("Unknown command '%s', please specify one of the following: 'new'.\n",
			       command.toStdString().c_str());
			return;
		}
	}

	int queueLength = queue->length();
	printf("\033[1;33mEntering resource cache editing mode! This mode allows you to edit textual resources for your files. To add media resources use the 'import' scraping module instead.\nNote that you can provide one or more file names on command line to edit resources for just those specific files. You can also use the '--startat' and '--endat' command line options to narrow down the span of the roms you wish to edit. Otherwise Skyscraper will edit ALL files found in the input folder one by one.\033[0m\n\n");
	while (queue->hasEntry())
	{
		QFileInfo info = queue->takeEntry();
		QString cacheId = getQuickId(info);
		if (cacheId.isEmpty())
		{
			cacheId = NameTools::getCacheId(info);
			addQuickId(info, cacheId);
		}
		bool doneEdit = false;
		printPriorities(cacheId);
		while (!doneEdit)
		{
			printf("\033[0;32m#%d/%d\033[0m \033[1;33m\nCURRENT FILE: \033[0m\033[1;32m%s\033[0m\033[1;33m\033[0m\n",
			       queueLength - queue->length(), queueLength, info.fileName().toStdString().c_str());
			std::string userInput = "";
			if (command.isEmpty())
			{
				printf("\033[1;34mWhat would you like to do?\033[0m (Press enter to continue to next rom in queue)\n");
				printf("\033[1;33ms\033[0m) Show current resource priorities for this rom\n");
				printf("\033[1;33mS\033[0m) Show all cached resources for this rom\n");
				printf("\033[1;33mn\033[0m) Create new prioritized resource for this rom\n");
				printf("\033[1;33md\033[0m) Remove specific resource connected to this rom\n");
				printf("\033[1;33mD\033[0m) Remove ALL resources connected to this rom\n");
				printf("\033[1;33mm\033[0m) Remove ALL resources connected to this rom from a specific module\n");
				printf("\033[1;33mt\033[0m) Remove ALL resources connected to this rom of a specific type\n");
				printf("\033[1;33mc\033[0m) Cancel all cache changes and exit\n");
				printf("\033[1;33mq\033[0m) Save all cache changes and exit\n");
				printf("> ");
				getline(std::cin, userInput);
				printf("\n");
			}
			else
			{
				if (command == "new")
				{
					userInput = "n";
					doneEdit = true;
				}
				else if (command == "quit")
				{
					userInput = "q";
				}
			}
			if (userInput == "")
			{
				doneEdit = true;
				continue;
			}
			else if (userInput == "s")
			{
				printPriorities(cacheId);
			}
			else if (userInput == "S")
			{
				printf("\033[1;34mResources connected to this rom:\033[0m\n");
				bool found = false;
				for (const auto &res: resources)
				{
					if (res.cacheId == cacheId)
					{
						printf("\033[1;33m%s\033[0m (%s): '\033[1;32m%s\033[0m'\n",
						       res.type.toStdString().c_str(),
						       res.source.toStdString().c_str(),
						       res.value.toStdString().c_str());
						found = true;
					}
				}
				if (!found)
					printf("None\n");
				printf("\n");
			}
			else if (userInput == "n")
			{
				GameEntry game;
				game.cacheId = cacheId;
				fillBlanks(game);
				std::string typeInput = "";
				if (type.isEmpty())
				{
					printf("\033[1;34mWhich resource type would you like to create?\033[0m (Enter to cancel)\n");
					printf("\033[1;33m0\033[0m) Title %s\n",
					       QString((game.titleSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                        : ""))
					           .toStdString()
					           .c_str());
					printf("\033[1;33m1\033[0m) Platform %s\n",
					       QString((game.platformSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                           : ""))
					           .toStdString()
					           .c_str());
					printf("\033[1;33m2\033[0m) Release date %s\n",
					       QString((game.releaseDateSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                              : ""))
					           .toStdString()
					           .c_str());
					printf("\033[1;33m3\033[0m) Developer %s\n",
					       QString((game.developerSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                            : ""))
					           .toStdString()
					           .c_str());
					printf("\033[1;33m4\033[0m) Publisher %s\n",
					       QString((game.publisherSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                            : ""))
					           .toStdString()
					           .c_str());
					printf("\033[1;33m5\033[0m) Number of players %s\n",
					       QString((game.playersSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                          : ""))
					           .toStdString()
					           .c_str());
					printf("\033[1;33m6\033[0m) Age rating %s\n",
					       QString((game.agesSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)" : "")).toStdString().c_str());
					printf("\033[1;33m7\033[0m) Genres %s\n",
					       QString((game.tagsSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)" : "")).toStdString().c_str());
					printf("\033[1;33m8\033[0m) Game rating %s\n",
					       QString((game.ratingSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                         : ""))
					           .toStdString()
					           .c_str());
					printf("\033[1;33m9\033[0m) Description %s\n",
					       QString((game.descriptionSrc.isEmpty() ? "(\033[1;31mmissing\033[0m)"
					                                              : ""))
					           .toStdString()
					           .c_str());
					printf("> ");
					getline(std::cin, typeInput);
					printf("\n");
				}
				else
				{
					if (type == "title")
					{
						typeInput = "0";
					}
					else if (type == "platform")
					{
						typeInput = "1";
					}
					else if (type == "releasedate")
					{
						typeInput = "2";
					}
					else if (type == "developer")
					{
						typeInput = "3";
					}
					else if (type == "publisher")
					{
						typeInput = "4";
					}
					else if (type == "players")
					{
						typeInput = "5";
					}
					else if (type == "ages")
					{
						typeInput = "6";
					}
					else if (type == "genres")
					{
						typeInput = "7";
					}
					else if (type == "rating")
					{
						typeInput = "8";
					}
					else if (type == "description")
					{
						typeInput = "9";
					}
				}
				if (typeInput == "")
				{
					printf("Resource creation cancelled...\n\n");
					continue;
				}
				else
				{
					Resource newRes;
					newRes.cacheId = cacheId;
					newRes.source = "user";
					newRes.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
					std::string valueInput = "";
					QString expression = ".+";// Default, matches everything except empty
					if (typeInput == "0")
					{
						newRes.type = "title";
						printf("\033[1;34mPlease enter title:\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
					}
					else if (typeInput == "1")
					{
						newRes.type = "platform";
						printf("\033[1;34mPlease enter platform:\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
					}
					else if (typeInput == "2")
					{
						newRes.type = "releasedate";
						printf("\033[1;34mPlease enter a release date in the format 'yyyy-MM-dd':\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
						expression = "^[1-2]{1}[0-9]{3}-[0-1]{1}[0-9]{1}-[0-3]{1}[0-9]{1}$";
					}
					else if (typeInput == "3")
					{
						newRes.type = "developer";
						printf("\033[1;34mPlease enter developer:\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
					}
					else if (typeInput == "4")
					{
						newRes.type = "publisher";
						printf("\033[1;34mPlease enter publisher:\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
					}
					else if (typeInput == "5")
					{
						newRes.type = "players";
						printf("\033[1;34mPlease enter highest number of players such as '4':\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
						expression = "^[0-9]{1,2}$";
					}
					else if (typeInput == "6")
					{
						newRes.type = "ages";
						printf("\033[1;34mPlease enter lowest age this should be played at such as '10' which means 10+:\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
						expression = "^[0-9]{1}[0-9]{0,1}$";
					}
					else if (typeInput == "7")
					{
						newRes.type = "tags";
						printf("\033[1;34mPlease enter comma-separated genres in the format 'Platformer, Sidescrolling':\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
					}
					else if (typeInput == "8")
					{
						newRes.type = "rating";
						printf("\033[1;34mPlease enter game rating from 0.0 to 1.0:\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
						expression = "^[0-1]{1}\\.{1}[0-9]{1}[0-9]{0,1}$";
					}
					else if (typeInput == "9")
					{
						newRes.type = "description";
						printf("\033[1;34mPlease enter game description. Type '\\n' for newlines:\033[0m (Enter to cancel)\n> ");
						getline(std::cin, valueInput);
					}
					else
					{
						printf("Invalid input, resource creation cancelled...\n\n");
						continue;
					}
					QString value = valueInput.c_str();
					printf("\n");
					value.replace("\\n", "\n");
					if (valueInput == "")
					{
						printf("Resource creation cancelled...\n\n");
						continue;
					}
					else if (!value.isEmpty() && QRegularExpression(expression).match(value).hasMatch())
					{
						newRes.value = value;
						bool updated = false;
						QMutableListIterator<Resource> it(resources);
						while (it.hasNext())
						{
							Resource res = it.next();
							if (res.cacheId == newRes.cacheId &&
							    res.type == newRes.type &&
							    res.source == newRes.source)
							{
								it.remove();
								updated = true;
							}
						}
						resources.append(newRes);
						if (updated)
						{
							printf(">>> Updated existing ");
						}
						else
						{
							printf(">>> Added ");
						}
						printf("resource with value '\033[1;32m%s\033[0m'\n\n", value.toStdString().c_str());
						continue;
					}
					else
					{
						printf("\033[1;31mWrong format, resource hasn't been added...\033[0m\n\n");
						continue;
					}
				}
			}
			else if (userInput == "d")
			{
				int b = 1;
				QList<int> resIds;
				printf("\033[1;34mWhich resource id would you like to remove?\033[0m (Enter to cancel)\n");
				for (int a = 0; a < resources.length(); ++a)
				{
					if (resources.at(a).cacheId == cacheId &&
					    resources.at(a).type != "screenshot" &&
					    resources.at(a).type != "cover" &&
					    resources.at(a).type != "logo" &&
					    resources.at(a).type != "marquee" &&
					    resources.at(a).type != "steamgrid" &&
					    resources.at(a).type != "icon" &&
					    resources.at(a).type != "hero" &&
					    resources.at(a).type != "video")
					{
						printf("\033[1;33m%d\033[0m) \033[1;33m%s\033[0m (%s): '\033[1;32m%s\033[0m'\n", b,
						       resources.at(a).type.toStdString().c_str(),
						       resources.at(a).source.toStdString().c_str(),
						       resources.at(a).value.toStdString().c_str());
						resIds.append(a);
						b++;
					}
				}
				if (b == 1)
				{
					printf("No resources found, cancelling...\n\n");
					continue;
				}
				printf("> ");
				std::string typeInput = "";
				getline(std::cin, typeInput);
				printf("\n");
				if (typeInput == "")
				{
					printf("Resource removal cancelled...\n\n");
					continue;
				}
				else
				{
					int chosen = atoi(typeInput.c_str());
					if (chosen >= 1 && chosen <= resIds.length())
					{
						resources.removeAt(resIds.at(chosen - 1));// -1 because lists start at 0
						printf("<<< Removed resource id %d\n\n", chosen);
					}
					else
					{
						printf("Incorrect resource id, cancelling...\n\n");
					}
				}
			}
			else if (userInput == "D")
			{
				QMutableListIterator<Resource> it(resources);
				bool found = false;
				while (it.hasNext())
				{
					Resource res = it.next();
					if (res.cacheId == cacheId)
					{
						printf("<<< Removed \033[1;33m%s\033[0m (%s) with value '\033[1;32m%s\033[0m'\n",
						       res.type.toStdString().c_str(),
						       res.source.toStdString().c_str(),
						       res.value.toStdString().c_str());
						it.remove();
						found = true;
					}
				}
				if (!found)
					printf("No resources found for this rom...\n");
				printf("\n");
			}
			else if (userInput == "m")
			{
				printf("\033[1;34mResources from which module would you like to remove?\033[0m (Enter to cancel)\n");
				QMap<QString, int> modules;
				for (const auto &res: resources)
				{
					if (res.cacheId == cacheId)
					{
						modules[res.source] += 1;
					}
				}
				QMap<QString, int>::iterator it;
				for (it = modules.begin(); it != modules.end(); ++it)
				{
					printf("'\033[1;33m%s\033[0m': %d resource(s) found\n", it.key().toStdString().c_str(), it.value());
				}
				if (modules.isEmpty())
				{
					printf("No resources found, cancelling...\n\n");
					continue;
				}
				printf("> ");
				std::string typeInput = "";
				getline(std::cin, typeInput);
				printf("\n");
				if (typeInput == "")
				{
					printf("Resource removal cancelled...\n\n");
					continue;
				}
				else if (modules.contains(QString(typeInput.c_str())))
				{
					QMutableListIterator<Resource> it(resources);
					int removed = 0;
					while (it.hasNext())
					{
						Resource res = it.next();
						if (res.cacheId == cacheId && res.source == QString(typeInput.c_str()))
						{
							it.remove();
							removed++;
						}
					}
					printf("<<< Removed %d resource(s) connected to rom from module '\033[1;32m%s\033[0m'\n\n", removed,
					       typeInput.c_str());
				}
				else
				{
					printf("No resources from module '\033[1;32m%s\033[0m' found, cancelling...\n\n",
					       typeInput.c_str());
				}
			}
			else if (userInput == "t")
			{
				printf("\033[1;34mResources of which type would you like to remove?\033[0m (Enter to cancel)\n");
				QMap<QString, int> types;
				for (const auto &res: resources)
				{
					if (res.cacheId == cacheId)
					{
						types[res.type] += 1;
					}
				}
				QMap<QString, int>::iterator it;
				for (it = types.begin(); it != types.end(); ++it)
				{
					printf("'\033[1;33m%s\033[0m': %d resource(s) found\n", it.key().toStdString().c_str(), it.value());
				}
				if (types.isEmpty())
				{
					printf("No resources found, cancelling...\n\n");
					continue;
				}
				printf("> ");
				std::string typeInput = "";
				getline(std::cin, typeInput);
				printf("\n");
				if (typeInput == "")
				{
					printf("Resource removal cancelled...\n\n");
					continue;
				}
				else if (types.contains(QString(typeInput.c_str())))
				{
					QMutableListIterator<Resource> it(resources);
					int removed = 0;
					while (it.hasNext())
					{
						Resource res = it.next();
						if (res.cacheId == cacheId && res.type == QString(typeInput.c_str()))
						{
							it.remove();
							removed++;
						}
					}
					printf("<<< Removed %d resource(s) connected to rom of type '\033[1;32m%s\033[0m'\n\n", removed,
					       typeInput.c_str());
				}
				else
				{
					printf("No resources of type '\033[1;32m%s\033[0m' found, cancelling...\n\n", typeInput.c_str());
				}
			}
			else if (userInput == "c")
			{
				printf("Exiting without saving changes.\n");
				exit(0);
			}
			else if (userInput == "q")
			{
				queue->clear();
				doneEdit = true;
				continue;
			}
		}
	}
}

bool Cache::purgeResources(QString purgeStr)
{
	purgeStr.replace("purge:", "");
	printf("Purging requested resources from cache, please wait...\n");

	QString module = "";
	QString type = "";

	QList<QString> definitions = purgeStr.split(",");
	for (const auto &definition: definitions)
	{
		if (definition.left(2) == "m=")
		{
			module = definition.split("=").at(1).simplified();
			printf("Module: '%s'\n", module.toStdString().c_str());
		}
		if (definition.left(2) == "t=")
		{
			type = definition.split("=").at(1).simplified();
			printf("Type: '%s'\n", type.toStdString().c_str());
		}
	}

	int purged = 0;

	QMutableListIterator<Resource> it(resources);
	while (it.hasNext())
	{
		Resource res = it.next();
		bool remove = false;
		if (res.source == module || res.type == type)
		{
			remove = true;
		}
		if (remove)
		{
			if (res.type == "cover" || res.type == "screenshot" ||
			    res.type == "logo" || res.type == "marquee" ||
			    res.type == "steamgrid" || res.type == "icon" ||
			    res.type == "hero" || res.type == "video")
			{
				if (!QFile::remove(cacheDir.absolutePath() + "/" + res.value))
				{
					printf("Couldn't purge media file '%s', skipping...\n", res.value.toStdString().c_str());
					continue;
				}
			}
			it.remove();
			purged++;
		}
	}
	printf("Successfully purged %d resources from the cache.\n", purged);
	return true;
}

bool Cache::purgeAll(const bool unattend)
{
	if (!unattend)
	{
		printf("\033[1;31mWARNING! You are about to purge / remove ALL resources from the Skyscaper cache connected to the currently selected platform. THIS CANNOT BE UNDONE!\033[0m\n\n");
		printf("\033[1;34mDo you wish to continue\033[0m (y/N)? ");
		std::string userInput = "";
		getline(std::cin, userInput);
		if (userInput != "y")
		{
			printf("User chose not to continue, cancelling purge...\n\n");
			return false;
		}
	}

	printf("Purging ALL resources for the selected platform, please wait...");

	int purged = 0;
	int dots = 0;
	// Always make dotMod at least 1 or it will give "floating point exception" when modulo
	int dotMod = resources.size() * 0.1 + 1;

	QMutableListIterator<Resource> it(resources);
	while (it.hasNext())
	{
		if (dots % dotMod == 0)
		{
			printf(".");
			fflush(stdout);
		}
		dots++;
		Resource res = it.next();
		if (res.type == "cover" || res.type == "screenshot" ||
		    res.type == "logo" || res.type == "marquee" ||
		    res.type == "steamgrid" || res.type == "icon" ||
		    res.type == "hero" || res.type == "video")
		{
			if (!QFile::remove(cacheDir.absolutePath() + "/" + res.value))
			{
				printf("Couldn't purge media file '%s', skipping...\n", res.value.toStdString().c_str());
				continue;
			}
		}
		it.remove();
		purged++;
	}
	printf("\033[1;32m Done!\033[0m\n");
	if (purged == 0)
	{
		printf("No resources for the current platform found in the resource cache.\n");
		return false;
	}
	else
	{
		printf("Successfully purged %d resources from the resource cache.\n", purged);
	}
	printf("\n");
	return true;
}

QList<QFileInfo> Cache::getFileInfos(const QString &inputFolder, const QString &filter, const bool subdirs)
{
	QList<QFileInfo> fileInfos;
	QStringList filters = filter.split(" ");
	if (filter.size() >= 2)
	{
		QDirIterator dirIt(inputFolder,
		                   filters,
		                   QDir::Files | QDir::NoDotAndDotDot,
		                   (subdirs ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags));
		while (dirIt.hasNext())
		{
			dirIt.next();
			fileInfos.append(dirIt.fileInfo());
		}
		if (fileInfos.isEmpty())
		{
			printf("\nInput folder returned no entries...\n\n");
		}
	}
	else
	{
		printf("Found less than 2 suffix filters. Something is wrong...\n");
	}
	return fileInfos;
}

QList<QString> Cache::getCacheIdList(const QList<QFileInfo> &fileInfos)
{
	QList<QString> cacheIdList;
	int dots = 0;
	// Always make dotMod at least 1 or it will give "floating point exception" when modulo
	int dotMod = fileInfos.size() * 0.1 + 1;
	for (const auto &info: fileInfos)
	{
		if (dots % dotMod == 0)
		{
			printf(".");
			fflush(stdout);
		}
		dots++;
		QString cacheId = getQuickId(info);
		if (cacheId.isEmpty())
		{
			cacheId = NameTools::getCacheId(info);
			addQuickId(info, cacheId);
		}
		cacheIdList.append(cacheId);
	}
	return cacheIdList;
}

void Cache::assembleReport(const Settings &config, const QString filter)
{
	QString reportStr = config.cacheOptions;

	if (!reportStr.contains("report:missing="))
	{
		printf("Don't understand report option, please check '--cache help' for more info.\n");
		return;
	}
	reportStr.replace("report:missing=", "");

	QString missingOption = reportStr.simplified();
	QList<QString> resTypeList;
	if (missingOption.contains(","))
	{
		resTypeList = missingOption.split(",");
	}
	else
	{
		if (missingOption == "all")
		{
			resTypeList.append("title");
			resTypeList.append("platform");
			resTypeList.append("description");
			resTypeList.append("publisher");
			resTypeList.append("developer");
			resTypeList.append("players");
			resTypeList.append("ages");
			resTypeList.append("tags");
			resTypeList.append("rating");
			resTypeList.append("releasedate");
			resTypeList.append("cover");
			resTypeList.append("screenshot");
			resTypeList.append("logo");
			resTypeList.append("marquee");
			resTypeList.append("steamgrid");
			resTypeList.append("icon");
			resTypeList.append("hero");
			resTypeList.append("video");
		}
		else if (missingOption == "textual")
		{
			resTypeList.append("title");
			resTypeList.append("platform");
			resTypeList.append("description");
			resTypeList.append("publisher");
			resTypeList.append("developer");
			resTypeList.append("players");
			resTypeList.append("ages");
			resTypeList.append("tags");
			resTypeList.append("rating");
			resTypeList.append("releasedate");
		}
		else if (missingOption == "artwork")
		{
			resTypeList.append("cover");
			resTypeList.append("screenshot");
			resTypeList.append("logo");
			resTypeList.append("marquee");
			resTypeList.append("steamgrid");
			resTypeList.append("icon");
			resTypeList.append("hero");
		}
		else if (missingOption == "media")
		{
			resTypeList.append("cover");
			resTypeList.append("screenshot");
			resTypeList.append("logo");
			resTypeList.append("marquee");
			resTypeList.append("steamgrid");
			resTypeList.append("icon");
			resTypeList.append("hero");
			resTypeList.append("video");
		}
		else
		{
			resTypeList.append(missingOption);// If a single type is given
		}
	}
	for (const auto &resType: resTypeList)
	{
		if (resType != "title" &&
		    resType != "platform" &&
		    resType != "description" &&
		    resType != "publisher" &&
		    resType != "developer" &&
		    resType != "ages" &&
		    resType != "players" &&
		    resType != "tags" &&
		    resType != "rating" &&
		    resType != "releasedate" &&
		    resType != "cover" &&
		    resType != "screenshot" &&
		    resType != "logo" &&
		    resType != "marquee" &&
		    resType != "steamgrid" &&
		    resType != "icon" &&
		    resType != "hero" &&
		    resType != "video")
		{
			if (resType != "help")
			{
				printf("\033[1;31mUnknown resource type '%s'!\033[0m\n", resType.toStdString().c_str());
			}
			printf("Please use one of the following:\n");
			printf("  \033[1;32mhelp\033[0m: Shows this help message\n");
			printf("  \033[1;32mall\033[0m: Creates reports for all resource types\n");
			printf("  \033[1;32mtextual\033[0m: Creates reports for all textual resource types\n");
			printf("  \033[1;32martwork\033[0m: Creates reports for all artwork related resource types excluding 'video'\n");
			printf("  \033[1;32mmedia\033[0m: Creates reports for all media resource types including 'video'\n");
			printf("  \033[1;32mtype1,type2,type3,...\033[0m: Creates reports for selected types. Example: 'developer,screenshot,rating'\n");
			printf("\nAvailable resource types:\n");
			printf("  \033[1;32mtitle\033[0m\n");
			printf("  \033[1;32mplatform\033[0m\n");
			printf("  \033[1;32mdescription\033[0m\n");
			printf("  \033[1;32mpublisher\033[0m\n");
			printf("  \033[1;32mdeveloper\033[0m\n");
			printf("  \033[1;32mplayers\033[0m\n");
			printf("  \033[1;32mages\033[0m\n");
			printf("  \033[1;32mtags\033[0m\n");
			printf("  \033[1;32mrating\033[0m\n");
			printf("  \033[1;32mreleasedate\033[0m\n");
			printf("  \033[1;32mcover\033[0m\n");
			printf("  \033[1;32mscreenshot\033[0m\n");
			printf("  \033[1;32mlogo\033[0m\n");
			printf("  \033[1;32mmarquee\033[0m\n");
			printf("  \033[1;32msteamgrid\033[0m\n");
			printf("  \033[1;32micon\033[0m\n");
			printf("  \033[1;32mhero\033[0m\n");
			printf("  \033[1;32mvideo\033[0m\n");
			printf("\n");
			return;
		}
	}
	if (resTypeList.isEmpty())
	{
		printf("Resource type list is empty, cancelling...\n");
		return;
	}
	else
	{
		printf("Creating report(s) for resource type(s):\n");
		for (const auto &resType: resTypeList)
		{
			printf("  %s\n", resType.toStdString().c_str());
		}
		printf("\n");
	}

	// Create the reports folder
	QDir reportsDir(QDir::currentPath() + "/reports");
	if (!reportsDir.exists())
	{
		if (!reportsDir.mkpath("."))
		{
			printf("Couldn't create reports folder '%s'. Please check permissions then try again...\n",
			       reportsDir.absolutePath().toStdString().c_str());
			return;
		}
	}

	Queue fileInfos;
	fileInfos.append(getFileInfos(config.inputFolder, filter, config.subdirs));
	if (!config.excludePattern.isEmpty())
	{
		fileInfos.filterFiles(config.excludePattern);
	}
	if (!config.includePattern.isEmpty())
	{
		fileInfos.filterFiles(config.includePattern, true);
	}
	printf("%d compatible files found for the '%s' platform!\n", fileInfos.length(),
	       config.platform.toStdString().c_str());
	printf("Creating file id list for all files, please wait...");
	QList<QString> cacheIdList = getCacheIdList(fileInfos);
	printf("\n\n");

	if (fileInfos.length() != cacheIdList.length())
	{
		printf("Length of cache id list mismatch the number of files, something is wrong! Please report this. Can't continue...\n");
		return;
	}

	QString dateTime = QDateTime::currentDateTime().toString("yyyyMMdd");
	for (const auto &resType: resTypeList)
	{
		QFile reportFile(
		    reportsDir.absolutePath() + "/report-" + config.platform + "-missing_" + resType + "-" + dateTime +
		    ".txt");
		printf("Report filename: '\033[1;32m%s\033[0m'\nAssembling report, please wait...",
		       reportFile.fileName().toStdString().c_str());
		if (reportFile.open(QIODevice::WriteOnly))
		{
			int missing = 0;
			int dots = 0;
			// Always make dotMod at least 1 or it will give "floating point exception" when modulo
			int dotMod = fileInfos.size() * 0.1 + 1;

			for (int a = 0; a < fileInfos.length(); ++a)
			{
				if (dots % dotMod == 0)
				{
					printf(".");
					fflush(stdout);
				}
				dots++;
				bool found = false;
				for (const auto &res: resources)
				{
					if (res.cacheId == cacheIdList.at(a))
					{
						if (res.type == resType)
						{
							found = true;
							break;
						}
					}
				}
				if (!found)
				{
					missing++;
					reportFile.write(fileInfos.at(a).absoluteFilePath().toUtf8() + "\n");
				}
			}
			reportFile.close();
			printf("\033[1;32m Done!\033[0m\n\033[1;33m%d file(s) is/are missing the '%s' resource.\033[0m\n\n",
			       missing, resType.toStdString().c_str());
		}
		else
		{
			printf("Report file could not be opened for writing, please check permissions of folder '/home/USER/.skyscraper', then try again...\n");
			return;
		}
	}
	printf("\033[1;32mAll done!\033[0m\nConsider using the '\033[1;33m--cache edit --fromfile <REPORTFILE>\033[0m' or the '\033[1;33m-s import\033[0m' module to add the missing resources. Check '\033[1;33m--help\033[0m' and '\033[1;33m--cache help\033[0m' for more information.\n\n");
}

bool Cache::vacuumResources(const QString inputFolder, const QString filter,
                            const int verbosity, const bool unattend)
{
	if (!unattend)
	{
		std::string userInput = "";
		printf("\033[1;33mWARNING! Vacuuming your Skyscraper cache removes all resources that don't match your current romset (files located at '%s' or any of its subdirectories matching the suffixes supported by the platform and any extension(s) you might have added manually). Please consider making a backup of your Skyscraper cache before performing this action. The cache for this platform is listed under 'Cache folder' further up and is usually located under '/home/USER/.skyscraper/' unless you've set it manually.\033[0m\n\n",
		       inputFolder.toStdString().c_str());
		printf("\033[1;34mDo you wish to continue\033[0m (y/N)? ");
		getline(std::cin, userInput);
		if (userInput != "y")
		{
			printf("User chose not to continue, cancelling vacuum...\n\n");
			return false;
		}
	}

	printf("Vacuuming cache, this can take several minutes, please wait...");
	QList<QFileInfo> fileInfos = getFileInfos(inputFolder, filter);
	// Clean the quick id's aswell
	QMap<QString, QPair<qint64, QString>> quickIdsCleaned;
	for (const auto &info: fileInfos)
	{
		QString filePath = info.absoluteFilePath();
		if (quickIds.contains(filePath))
		{
			quickIdsCleaned[filePath] = quickIds[filePath];
		}
	}
	quickIds = quickIdsCleaned;
	QList<QString> cacheIdList = getCacheIdList(fileInfos);
	if (cacheIdList.isEmpty())
	{
		printf("No cache id's found, something is wrong, cancelling...\n");
		return false;
	}

	int vacuumed = 0;
	{
		int dots = 0;
		// Always make dotMod at least 1 or it will give "floating point exception" when modulo
		int dotMod = resources.size() * 0.1 + 1;

		QMutableListIterator<Resource> it(resources);
		while (it.hasNext())
		{
			if (dots % dotMod == 0)
			{
				printf(".");
				fflush(stdout);
			}
			dots++;
			Resource res = it.next();
			bool remove = true;
			for (const auto &cacheId: cacheIdList)
			{
				if (res.cacheId == cacheId)
				{
					remove = false;
					break;
				}
			}
			if (remove)
			{
				if (res.type == "cover" || res.type == "screenshot" ||
				    res.type == "logo" || res.type == "marquee" ||
				    res.type == "steamgrid" || res.type == "icon" ||
				    res.type == "hero" || res.type == "video")
				{
					if (!QFile::remove(cacheDir.absolutePath() + "/" + res.value))
					{
						printf("Couldn't purge media file '%s', skipping...\n", res.value.toStdString().c_str());
						continue;
					}
				}
				if (verbosity > 1)
					printf("Purged resource for '%s' with value '%s'...\n", res.cacheId.toStdString().c_str(),
					       res.value.toStdString().c_str());
				it.remove();
				vacuumed++;
			}
		}
	}
	printf("\033[1;32m Done!\033[0m\n");
	if (vacuumed == 0)
	{
		printf("All resources match a file in your romset. No resources vacuumed.\n");
		return false;
	}
	else
	{
		printf("Successfully vacuumed %d resources from the resource cache.\n", vacuumed);
	}
	printf("\n");
	return true;
}

void Cache::showStats(int verbosity)
{
	printf("Resource cache stats for selected platform:\n");
	if (verbosity == 1)
	{
		int titles = 0;
		int platforms = 0;
		int descriptions = 0;
		int publishers = 0;
		int developers = 0;
		int players = 0;
		int ages = 0;
		int tags = 0;
		int ratings = 0;
		int releaseDates = 0;
		int covers = 0;
		int screenshots = 0;
		int logos = 0;
		int marquees = 0;
		int steamgrids = 0;
		int icons = 0;
		int heroes = 0;
		int videos = 0;
		for (QMap<QString, ResCounts>::iterator it = resCountsMap.begin();
		     it != resCountsMap.end(); ++it)
		{
			titles += it.value().titles;
			platforms += it.value().platforms;
			descriptions += it.value().descriptions;
			publishers += it.value().publishers;
			developers += it.value().developers;
			players += it.value().players;
			ages += it.value().ages;
			tags += it.value().tags;
			ratings += it.value().ratings;
			releaseDates += it.value().releaseDates;
			covers += it.value().covers;
			screenshots += it.value().screenshots;
			logos += it.value().logos;
			marquees += it.value().marquees;
			steamgrids += it.value().steamgrids;
			icons += it.value().icons;
			heroes += it.value().heroes;
			videos += it.value().videos;
		}
		printf("  Titles       : %d\n", titles);
		printf("  Platforms    : %d\n", platforms);
		printf("  Descriptions : %d\n", descriptions);
		printf("  Publishers   : %d\n", publishers);
		printf("  Developers   : %d\n", developers);
		printf("  Players      : %d\n", players);
		printf("  Ages         : %d\n", ages);
		printf("  Tags         : %d\n", tags);
		printf("  Ratings      : %d\n", ratings);
		printf("  ReleaseDates : %d\n", releaseDates);
		printf("  Covers       : %d\n", covers);
		printf("  Screenshots  : %d\n", screenshots);
		printf("  Logos        : %d\n", logos);
		printf("  Marquees     : %d\n", marquees);
		printf("  Steamgrids   : %d\n", steamgrids);
		printf("  Icons        : %d\n", icons);
		printf("  Heroes       : %d\n", heroes);
		printf("  Videos       : %d\n", videos);
	}
	else if (verbosity > 1)
	{
		for (QMap<QString, ResCounts>::iterator it = resCountsMap.begin();
		     it != resCountsMap.end(); ++it)
		{
			printf("'\033[1;32m%s\033[0m' module\n", it.key().toStdString().c_str());
			printf("  Titles       : %d\n", it.value().titles);
			printf("  Platforms    : %d\n", it.value().platforms);
			printf("  Descriptions : %d\n", it.value().descriptions);
			printf("  Publishers   : %d\n", it.value().publishers);
			printf("  Developers   : %d\n", it.value().developers);
			printf("  Ages         : %d\n", it.value().ages);
			printf("  Tags         : %d\n", it.value().tags);
			printf("  Ratings      : %d\n", it.value().ratings);
			printf("  ReleaseDates : %d\n", it.value().releaseDates);
			printf("  Covers       : %d\n", it.value().covers);
			printf("  Screenshots  : %d\n", it.value().screenshots);
			printf("  Logos        : %d\n", it.value().logos);
			printf("  Marquees     : %d\n", it.value().marquees);
			printf("  Steamgrids   : %d\n", it.value().steamgrids);
			printf("  Icons        : %d\n", it.value().icons);
			printf("  Heroes       : %d\n", it.value().heroes);
			printf("  Videos       : %d\n", it.value().videos);
		}
	}
	printf("\n");
}

void Cache::addToResCounts(const QString source, const QString type)
{
	if (type == "title")
	{
		resCountsMap[source].titles++;
	}
	else if (type == "platform")
	{
		resCountsMap[source].platforms++;
	}
	else if (type == "description")
	{
		resCountsMap[source].descriptions++;
	}
	else if (type == "publisher")
	{
		resCountsMap[source].publishers++;
	}
	else if (type == "developer")
	{
		resCountsMap[source].developers++;
	}
	else if (type == "players")
	{
		resCountsMap[source].players++;
	}
	else if (type == "ages")
	{
		resCountsMap[source].ages++;
	}
	else if (type == "tags")
	{
		resCountsMap[source].tags++;
	}
	else if (type == "rating")
	{
		resCountsMap[source].ratings++;
	}
	else if (type == "releasedate")
	{
		resCountsMap[source].releaseDates++;
	}
	else if (type == "cover")
	{
		resCountsMap[source].covers++;
	}
	else if (type == "screenshot")
	{
		resCountsMap[source].screenshots++;
	}
	else if (type == "logo")
	{
		resCountsMap[source].logos++;
	}
	else if (type == "marquee")
	{
		resCountsMap[source].marquees++;
	}
	else if (type == "steamgrid")
	{
		resCountsMap[source].steamgrids++;
	}
	else if (type == "icon")
	{
		resCountsMap[source].icons++;
	}
	else if (type == "hero")
	{
		resCountsMap[source].heroes++;
	}
	else if (type == "video")
	{
		resCountsMap[source].videos++;
	}
}

void Cache::readPriorities()
{
	QDomDocument prioDoc;
	QFile prioFile(cacheDir.absolutePath() + "/priorities.xml");
	printf("Looking for optional '\033[1;33mpriorities.xml\033[0m' file in cache folder... ");
	if (prioFile.open(QIODevice::ReadOnly))
	{
		printf("\033[1;32mFound!\033[0m\n");
		if (!prioDoc.setContent(prioFile.readAll()))
		{
			printf("Document is not XML compliant, skipping...\n\n");
			return;
		}
	}
	else
	{
		printf("Not found, skipping...\n\n");
		return;
	}

	QDomNodeList orderNodes = prioDoc.elementsByTagName("order");

	int errors = 0;
	for (int a = 0; a < orderNodes.length(); ++a)
	{
		QDomElement orderElem = orderNodes.at(a).toElement();
		if (!orderElem.hasAttribute("type"))
		{
			printf("Priority 'order' node missing 'type' attribute, skipping...\n");
			errors++;
			continue;
		}
		QString type = orderElem.attribute("type");
		QList<QString> sources;
		// ALWAYS prioritize 'user' resources highest (added with edit mode)
		sources.append("user");
		QDomNodeList sourceNodes = orderNodes.at(a).childNodes();
		if (sourceNodes.isEmpty())
		{
			printf("'source' node(s) missing for type '%s' in priorities.xml, skipping...\n",
			       type.toStdString().c_str());
			errors++;
			continue;
		}
		for (int b = 0; b < sourceNodes.length(); ++b)
		{
			sources.append(sourceNodes.at(b).toElement().text());
		}
		prioMap[type] = sources;
	}
	printf("Priorities loaded successfully");
	if (errors != 0)
	{
		printf(", but %d error(s) encountered, please correct this", errors);
	}
	printf("!\n\n");
}

bool Cache::write(const bool onlyQuickId)
{
	QMutexLocker locker(&cacheMutex);

	QFile quickIdFile(cacheDir.absolutePath() + "/quickid.xml");
	if (quickIdFile.open(QIODevice::WriteOnly))
	{
		printf("Writing quick id xml, please wait... ");
		fflush(stdout);
		QXmlStreamWriter xml(&quickIdFile);
		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("quickids");
		for (const auto &key: quickIds.keys())
		{
			xml.writeStartElement("quickid");
			xml.writeAttribute("filepath", key);
			xml.writeAttribute("timestamp", QString::number(quickIds[key].first));
			xml.writeAttribute("id", quickIds[key].second);
			xml.writeEndElement();
		}
		xml.writeEndElement();
		xml.writeEndDocument();
		printf("\033[1;32mDone!\033[0m\n");
		quickIdFile.close();
		if (onlyQuickId)
		{
			return true;
		}
	}

	bool result = false;
	QFile cacheFile(cacheDir.absolutePath() + "/db.xml");
	if (cacheFile.open(QIODevice::WriteOnly))
	{
		printf("Writing %d (%d new) resources to cache, please wait... ",
		       resources.length(), resources.length() - resAtLoad);
		fflush(stdout);
		QXmlStreamWriter xml(&cacheFile);
		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("resources");
		for (const auto &resource: resources)
		{
			xml.writeStartElement("resource");
			xml.writeAttribute("id", resource.cacheId);
			xml.writeAttribute("type", resource.type);
			xml.writeAttribute("source", resource.source);
			xml.writeAttribute("timestamp", QString::number(resource.timestamp));
			xml.writeCharacters(resource.value);
			xml.writeEndElement();
		}
		xml.writeEndElement();
		xml.writeEndDocument();
		result = true;
		printf("\033[1;32mDone!\033[0m\n\n");
		cacheFile.close();
	}
	return result;
}

// This verifies all attached media files and deletes those that have no entry in the cache
void Cache::validate()
{
	// TODO: Add format checks for each resource type, and remove if deemed corrupt

	printf("Starting resource cache validation run, please wait...\n");

	if (!QFileInfo::exists(cacheDir.absolutePath() + "/db.xml"))
	{
		printf("'db.xml' not found, cache cleaning cancelled...\n");
		return;
	}

	QDir coversDir(cacheDir.absolutePath() + "/covers", "*.*", QDir::Name, QDir::Files);
	QDir screenshotsDir(cacheDir.absolutePath() + "/screenshots", "*.*", QDir::Name, QDir::Files);
	QDir logosDir(cacheDir.absolutePath() + "/logos", "*.*", QDir::Name, QDir::Files);
	QDir marqueesDir(cacheDir.absolutePath() + "/marquees", "*.*", QDir::Name, QDir::Files);
	QDir steamgridsDir(cacheDir.absolutePath() + "/steamgrids", "*.*", QDir::Name, QDir::Files);
	QDir iconsDir(cacheDir.absolutePath() + "/icons", "*.*", QDir::Name, QDir::Files);
	QDir heroesDir(cacheDir.absolutePath() + "/heroes", "*.*", QDir::Name, QDir::Files);
	QDir videosDir(cacheDir.absolutePath() + "/videos", "*.*", QDir::Name, QDir::Files);

	QDirIterator coversDirIt(coversDir.absolutePath(),
	                         QDir::Files | QDir::NoDotAndDotDot,
	                         QDirIterator::Subdirectories);

	QDirIterator screenshotsDirIt(screenshotsDir.absolutePath(),
	                              QDir::Files | QDir::NoDotAndDotDot,
	                              QDirIterator::Subdirectories);

	QDirIterator logosDirIt(logosDir.absolutePath(),
	                        QDir::Files | QDir::NoDotAndDotDot,
	                        QDirIterator::Subdirectories);

	QDirIterator marqueesDirIt(marqueesDir.absolutePath(),
	                           QDir::Files | QDir::NoDotAndDotDot,
	                           QDirIterator::Subdirectories);

	QDirIterator steamgridsDirIt(steamgridsDir.absolutePath(),
	                             QDir::Files | QDir::NoDotAndDotDot,
	                             QDirIterator::Subdirectories);

	QDirIterator iconsDirIt(iconsDir.absolutePath(),
	                             QDir::Files | QDir::NoDotAndDotDot,
	                             QDirIterator::Subdirectories);

	QDirIterator heroesDirIt(heroesDir.absolutePath(),
	                         QDir::Files | QDir::NoDotAndDotDot,
	                         QDirIterator::Subdirectories);

	QDirIterator videosDirIt(videosDir.absolutePath(),
	                         QDir::Files | QDir::NoDotAndDotDot,
	                         QDirIterator::Subdirectories);

	int filesDeleted = 0;
	int filesNoDelete = 0;

	verifyFiles(coversDirIt, filesDeleted, filesNoDelete, "cover");
	verifyFiles(screenshotsDirIt, filesDeleted, filesNoDelete, "screenshot");
	verifyFiles(logosDirIt, filesDeleted, filesNoDelete, "logo");
	verifyFiles(marqueesDirIt, filesDeleted, filesNoDelete, "marquee");
	verifyFiles(steamgridsDirIt, filesDeleted, filesNoDelete, "steamgrid");
	verifyFiles(iconsDirIt, filesDeleted, filesNoDelete, "icon");
	verifyFiles(heroesDirIt, filesDeleted, filesNoDelete, "hero");
	verifyFiles(videosDirIt, filesDeleted, filesNoDelete, "video");

	if (filesDeleted == 0 && filesNoDelete == 0)
	{
		printf("No inconsistencies found in the database. :)\n\n");
	}
	else
	{
		printf("Successfully deleted %d files with no resource entry.\n", filesDeleted);
		if (filesNoDelete != 0)
		{
			printf("%d files couldn't be deleted, please check file permissions and re-run with '--cache validate'.\n",
			       filesNoDelete);
		}
		printf("\n");
	}
}

void Cache::verifyFiles(QDirIterator &dirIt, int &filesDeleted, int &filesNoDelete, QString resType)
{
	QList<QString> resFileNames;
	for (const auto &resource: resources)
	{
		if (resource.type == resType)
		{
			QFileInfo resInfo(cacheDir.absolutePath() + "/" + resource.value);
			resFileNames.append(resInfo.absoluteFilePath());
		}
	}

	while (dirIt.hasNext())
	{
		QFileInfo fileInfo(dirIt.next());
		if (!resFileNames.contains(fileInfo.absoluteFilePath()))
		{
			printf("No resource entry for file '%s', deleting... ",
			       fileInfo.absoluteFilePath().toStdString().c_str());
			if (QFile::remove(fileInfo.absoluteFilePath()))
			{
				printf("OK!\n");
				filesDeleted++;
			}
			else
			{
				printf("ERROR! File couldn't be deleted :/\n");
				filesNoDelete++;
			}
		}
	}
}

void Cache::merge(Cache &mergeCache, bool overwrite, const QString &mergeCacheFolder)
{
	printf("Merging databases, please wait...\n");
	QList<Resource> mergeResources = mergeCache.getResources();

	QDir mergeCacheDir(mergeCacheFolder);

	int resUpdated = 0;
	int resMerged = 0;

	for (const auto &mergeResource: mergeResources)
	{
		bool resExists = false;
		// This type of iterator ensures we can delete items while iterating
		QMutableListIterator<Resource> it(resources);
		while (it.hasNext())
		{
			Resource res = it.next();
			if (res.cacheId == mergeResource.cacheId &&
			    res.type == mergeResource.type &&
			    res.source == mergeResource.source)
			{
				if (overwrite)
				{
					if (res.type == "cover" || res.type == "screenshot" ||
					    res.type == "logo" || res.type == "marquee" ||
					    res.type == "steamgrid" || res.type == "icon" ||
					    res.type == "hero" || res.type == "video")
					{
						if (!QFile::remove(cacheDir.absolutePath() + "/" + res.value))
						{
							printf("Couldn't remove media file '%s' for updating, skipping...\n",
							       res.value.toStdString().c_str());
							continue;
						}
					}
					it.remove();
				}
				else
				{
					resExists = true;
					break;
				}
			}
		}
		if (!resExists)
		{
			if (mergeResource.type == "cover" || mergeResource.type == "screenshot" ||
			    mergeResource.type == "logo" || mergeResource.type == "marquee" ||
			    mergeResource.type == "steamgrid" || mergeResource.type == "icon" ||
			    mergeResource.type == "hero" || mergeResource.type == "video")
			{
				cacheDir.mkpath(QFileInfo(cacheDir.absolutePath() + "/" + mergeResource.value).absolutePath());
				if (!QFile::copy(mergeCacheDir.absolutePath() + "/" + mergeResource.value,
				                 cacheDir.absolutePath() + "/" + mergeResource.value))
				{
					printf("Couldn't copy media file '%s', skipping...\n", mergeResource.value.toStdString().c_str());
					continue;
				}
			}
			if (overwrite)
			{
				resUpdated++;
			}
			else
			{
				resMerged++;
			}
			resources.append(mergeResource);
		}
	}
	printf("Successfully updated %d resource(s) in cache!\n", resUpdated);
	printf("Successfully merged %d new resource(s) into cache!\n\n", resMerged);
}

QList<Resource> Cache::getResources()
{
	return resources;
}

void Cache::addResources(GameEntry &entry, const Settings &config, QString &output)
{
	QString cacheAbsolutePath = cacheDir.absolutePath();

	if (entry.source.isEmpty())
	{
		printf("Something is wrong, resource with cache id '%s' has no source, exiting...\n",
		       entry.cacheId.toStdString().c_str());
		exit(1);
	}

	if (entry.cacheId != "")
	{
		Resource resource;
		resource.cacheId = entry.cacheId;
		resource.source = entry.source;
		resource.timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
		if (entry.title != "")
		{
			resource.type = "title";
			resource.value = entry.title;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.platform != "")
		{
			resource.type = "platform";
			resource.value = entry.platform;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.description != "")
		{
			resource.type = "description";
			resource.value = entry.description;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.publisher != "")
		{
			resource.type = "publisher";
			resource.value = entry.publisher;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.developer != "")
		{
			resource.type = "developer";
			resource.value = entry.developer;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.players != "")
		{
			resource.type = "players";
			resource.value = entry.players;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.ages != "")
		{
			resource.type = "ages";
			resource.value = entry.ages;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.tags != "")
		{
			resource.type = "tags";
			resource.value = entry.tags;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.rating != "")
		{
			resource.type = "rating";
			resource.value = entry.rating;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.releaseDate != "")
		{
			resource.type = "releasedate";
			resource.value = entry.releaseDate;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (entry.videoData != "" && entry.videoFormat != "")
		{
			resource.type = "video";
			resource.value = "videos/" + entry.source + "/" + entry.cacheId + "." + entry.videoFormat;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (!entry.coverData.isNull() && config.cacheCovers)
		{
			resource.type = "cover";
			resource.value = "covers/" + entry.source + "/" + entry.cacheId;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (!entry.screenshotData.isNull() && config.cacheScreenshots)
		{
			resource.type = "screenshot";
			resource.value = "screenshots/" + entry.source + "/" + entry.cacheId;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (!entry.logoData.isNull() && config.cacheLogos)
		{
			resource.type = "logo";
			resource.value = "logos/" + entry.source + "/" + entry.cacheId;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (!entry.marqueeData.isNull() && config.cacheMarquees)
		{
			resource.type = "marquee";
			resource.value = "marquees/" + entry.source + "/" + entry.cacheId;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (!entry.steamgridData.isNull() && config.cacheSteamgrids)
		{
			resource.type = "steamgrid";
			resource.value = "steamgrids/" + entry.source + "/" + entry.cacheId;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (!entry.iconData.isNull() && config.cacheIcons)
		{
			resource.type = "icon";
			resource.value = "icons/" + entry.source + "/" + entry.cacheId;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
		if (!entry.heroData.isNull() && config.cacheHeroes)
		{
			resource.type = "hero";
			resource.value = "heroes/" + entry.source + "/" + entry.cacheId;
			addResource(resource, entry, cacheAbsolutePath, config, output);
		}
	}
}

void Cache::addResource(Resource &resource,
                        GameEntry &entry,
                        const QString &cacheAbsolutePath,
                        const Settings &config,
                        QString &output)
{
	QMutexLocker locker(&cacheMutex);
	bool notFound = true;
	// This type of iterator ensures we can delete items while iterating
	QMutableListIterator<Resource> it(resources);
	while (it.hasNext())
	{
		Resource res = it.next();
		if (res.cacheId == resource.cacheId &&
		    res.type == resource.type &&
		    res.source == resource.source)
		{
			if (config.refresh)
			{
				it.remove();
			}
			else
			{
				notFound = false;
			}
			break;
		}
	}

	if (notFound)
	{
		bool okToAppend = true;
		QString cacheFile = cacheAbsolutePath + "/" + resource.value;
		if (resource.type == "cover" ||
		    resource.type == "screenshot" ||
		    resource.type == "logo" ||
		    resource.type == "marquee" ||
		    resource.type == "steamgrid" ||
		    resource.type == "icon" ||
		    resource.type == "hero")
		{
			QByteArray *imageData = nullptr;
			if (resource.type == "cover")
			{
				imageData = &entry.coverData;
			}
			else if (resource.type == "screenshot")
			{
				imageData = &entry.screenshotData;
			}
			else if (resource.type == "logo")
			{
				imageData = &entry.logoData;
			}
			else if (resource.type == "marquee")
			{
				imageData = &entry.marqueeData;
			}
			else if (resource.type == "steamgrid")
			{
				imageData = &entry.steamgridData;
			}
			else if (resource.type == "icon")
			{
				imageData = &entry.iconData;
			}
			else if (resource.type == "hero")
			{
				imageData = &entry.heroData;
			}
			if (config.cacheResize)
			{
				QImage image;
				if (imageData->size() > 0 &&
				    image.loadFromData(*imageData) &&
				    !image.isNull())
				{
					int max = 800;
					if (image.width() > max || image.height() > max)
					{
						image = image.scaled(max, max, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					}
					QByteArray resizedData;
					QBuffer b(&resizedData);
					b.open(QIODevice::WriteOnly);
					if ((image.hasAlphaChannel() && hasAlpha(image)) || resource.type == "screenshot")
					{
						okToAppend = image.save(&b, "png");
					}
					else
					{
						okToAppend = image.save(&b, "jpg", config.jpgQuality);
					}
					b.close();
					if (imageData->size() > resizedData.size())
					{
						if (config.verbosity >= 3)
						{
							printf("%s: '%d' > '%d', choosing resize for optimal result!\n",
							       resource.type.toStdString().c_str(),
							       imageData->size(),
							       resizedData.size());
						}
						*imageData = resizedData;
					}
				}
				else
				{
					okToAppend = false;
				}
			}
			if (okToAppend)
			{
				QFile f(cacheFile);
				if (f.open(QIODevice::WriteOnly))
				{
					f.write(*imageData);
					f.close();
				}
				else
				{
					output.append("Error writing file: '" + f.fileName() + "' to cache. Please check permissions.");
					okToAppend = false;
				}
			}
			else
			{
				// Image was faulty and could not be saved to cache so we clear
				// the QByteArray data in game entry to make sure we get a "NO"
				// in the terminal output from scraperworker.cpp.
				imageData->clear();
			}
		}
		else if (resource.type == "video")
		{
			if (entry.videoData.size() <= config.videoSizeLimit)
			{
				QFile f(cacheFile);
				if (f.open(QIODevice::WriteOnly))
				{
					f.write(entry.videoData);
					f.close();
					if (!config.videoConvertCommand.isEmpty())
					{
						output.append("Video conversion: ");
						if (doVideoConvert(resource,
						                   cacheFile,
						                   cacheAbsolutePath,
						                   config,
						                   output))
						{
							output.append("\033[1;32mSuccess!\033[0m");
						}
						else
						{
							output.append("\033[1;31mFailed!\033[0m (set higher '--verbosity N' level for more info)");
							f.remove();
							okToAppend = false;
						}
					}
				}
				else
				{
					output.append("Error writing file: '" + f.fileName() + "' to cache. Please check permissions.");
					okToAppend = false;
				}
			}
			else
			{
				output.append("Video exceeds maximum size of " + QString::number(config.videoSizeLimit / 1024 / 1024) +
				              " MB. Adjust this limit with the 'videoSizeLimit' variable in '/home/USER/.skyscraper/config.ini.'");
				okToAppend = false;
			}
		}

		if (okToAppend)
		{
			if (resource.type == "cover" ||
			    resource.type == "screenshot" ||
			    resource.type == "logo" ||
			    resource.type == "marquee" ||
			    resource.type == "steamgrid" ||
			    resource.type == "icon" ||
			    resource.type == "hero")
			{
				// Remove old style cache image if it exists
				if (QFile::exists(cacheFile + ".png"))
				{
					QFile::remove(cacheFile + ".png");
				}
			}
			resources.append(resource);
		}
		else
		{
			printf("\033[1;33mWarning! Couldn't add resource to cache. Have you run out of disk space?\n\033[0m");
		}
	}
}

bool Cache::doVideoConvert(Resource &resource,
                           QString &cacheFile,
                           const QString &cacheAbsolutePath,
                           const Settings &config,
                           QString &output)
{
	if (config.verbosity >= 2)
	{
		output.append("\n");
	}
	QString videoConvertCommand = config.videoConvertCommand;
	if (!videoConvertCommand.contains("%i"))
	{
		output.append("'videoConvertCommand' is missing the required %i tag.\n");
		return false;
	}
	if (!videoConvertCommand.contains("%o"))
	{
		output.append("'videoConvertCommand' is missing the required %o tag.\n");
		return false;
	}
	QFileInfo cacheFileInfo(cacheFile);
	QString tmpCacheFile = cacheFileInfo.absolutePath() + "/tmpfile_" +
	                       (config.videoConvertExtension.isEmpty() ? cacheFileInfo.fileName() : cacheFileInfo.completeBaseName() + "." + config.videoConvertExtension);
	videoConvertCommand.replace("%i", cacheFile);
	videoConvertCommand.replace("%o", tmpCacheFile);
	if (QFile::exists(tmpCacheFile))
	{
		if (!QFile::remove(tmpCacheFile))
		{
			output.append("'" + tmpCacheFile + "' already exists and can't be removed.\n");
			return false;
		}
	}
	if (config.verbosity >= 2)
	{
		output.append("%i: '" + cacheFile + "'\n");
		output.append("%o: '" + tmpCacheFile + "'\n");
	}
	if (config.verbosity >= 3)
	{
		output.append("Running command: '" + videoConvertCommand + "'\n");
	}
	QProcess convertProcess;
	if (videoConvertCommand.contains(" "))
	{
		convertProcess.start(videoConvertCommand.split(' ').first(),
		                     QStringList({videoConvertCommand.split(' ').mid(1)}));
	}
	else
	{
		convertProcess.start(videoConvertCommand, QStringList({}));
	}
	// Wait 10 minutes max for conversion to complete
	if (convertProcess.waitForFinished(1000 * 60 * 10) &&
	    convertProcess.exitStatus() == QProcess::NormalExit &&
	    QFile::exists(tmpCacheFile))
	{
		if (!QFile::remove(cacheFile))
		{
			output.append("Original '" + cacheFile + "' file couldn't be removed.\n");
			return false;
		}
		cacheFile = tmpCacheFile;
		cacheFile.replace("tmpfile_", "");
		if (QFile::exists(cacheFile))
		{
			if (!QFile::remove(cacheFile))
			{
				output.append("'" + cacheFile + "' already exists and can't be removed.\n");
				return false;
			}
		}
		if (QFile::rename(tmpCacheFile, cacheFile))
		{
			resource.value = cacheFile.replace(cacheAbsolutePath + "/", "");
		}
		else
		{
			output.append(
			    "Couldn't rename file '" + tmpCacheFile + "' to '" + cacheFile + "', please check permissions!\n");
			return false;
		}
	}
	else
	{
		if (config.verbosity >= 3)
		{
			output.append(convertProcess.readAllStandardOutput() + "\n");
			output.append(convertProcess.readAllStandardError() + "\n");
		}
		return false;
	}
	if (config.verbosity >= 3)
	{
		output.append(convertProcess.readAllStandardOutput() + "\n");
		output.append(convertProcess.readAllStandardError() + "\n");
	}
	return true;
}

bool Cache::hasAlpha(const QImage &image)
{
	QRgb *constBits = (QRgb *) image.constBits();
	for (int a = 0; a < image.width() * image.height(); ++a)
	{
		if (qAlpha(constBits[a]) < 127)
		{
			return true;
		}
	}
	return false;
}

void Cache::addQuickId(const QFileInfo &info, const QString &cacheId)
{
	QMutexLocker locker(&quickIdMutex);
	QPair<qint64, QString> pair;// Quick id pair
	pair.first = info.lastModified().toMSecsSinceEpoch();
	pair.second = cacheId;
	quickIds[info.absoluteFilePath()] = pair;
}

QString Cache::getQuickId(const QFileInfo &info)
{
	QMutexLocker locker(&quickIdMutex);
	if (quickIds.contains(info.absoluteFilePath()) &&
	    info.lastModified().toMSecsSinceEpoch() <= quickIds[info.absoluteFilePath()].first)
	{
		return quickIds[info.absoluteFilePath()].second;
	}
	return QString();
}

bool Cache::hasEntries(const QString &cacheId, const QString scraper)
{
	QMutexLocker locker(&cacheMutex);
	for (const auto &res: resources)
	{
		if (scraper.isEmpty())
		{
			if (res.cacheId == cacheId)
			{
				return true;
			}
		}
		else
		{
			if (res.cacheId == cacheId && res.source == scraper)
			{
				return true;
			}
		}
	}
	return false;
}

void Cache::fillBlanks(GameEntry &entry, const QString scraper)
{
	QMutexLocker locker(&cacheMutex);
	QList<Resource> matchingResources;
	// Find all resources related to this particular rom
	for (const auto &resource: resources)
	{
		if (scraper.isEmpty())
		{
			if (entry.cacheId == resource.cacheId)
			{
				matchingResources.append(resource);
			}
		}
		else
		{
			if (entry.cacheId == resource.cacheId && resource.source == scraper)
			{
				matchingResources.append(resource);
			}
		}
	}

	{
		QString type = "title";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.title = result;
			entry.titleSrc = source;
		}
	}
	{
		QString type = "platform";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.platform = result;
			entry.platformSrc = source;
		}
	}
	{
		QString type = "description";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.description = result;
			entry.descriptionSrc = source;
		}
	}
	{
		QString type = "publisher";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.publisher = result;
			entry.publisherSrc = source;
		}
	}
	{
		QString type = "developer";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.developer = result;
			entry.developerSrc = source;
		}
	}
	{
		QString type = "players";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.players = result;
			entry.playersSrc = source;
		}
	}
	{
		QString type = "ages";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.ages = result;
			entry.agesSrc = source;
		}
	}
	{
		QString type = "tags";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.tags = result;
			entry.tagsSrc = source;
		}
	}
	{
		QString type = "rating";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.rating = result;
			entry.ratingSrc = source;
		}
	}
	{
		QString type = "releasedate";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			entry.releaseDate = result;
			entry.releaseDateSrc = source;
		}
	}
	{
		QString type = "cover";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFile f(cacheDir.absolutePath() + "/" + result);
			if (f.open(QIODevice::ReadOnly))
			{
				entry.coverData = f.readAll();
				f.close();
			}
			entry.coverSrc = source;
		}
	}
	{
		QString type = "screenshot";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFile f(cacheDir.absolutePath() + "/" + result);
			if (f.open(QIODevice::ReadOnly))
			{
				entry.screenshotData = f.readAll();
				f.close();
			}
			entry.screenshotSrc = source;
		}
	}
	{
		QString type = "logo";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFile f(cacheDir.absolutePath() + "/" + result);
			if (f.open(QIODevice::ReadOnly))
			{
				entry.logoData = f.readAll();
				f.close();
			}
			entry.logoSrc = source;
		}
	}
	{
		QString type = "marquee";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFile f(cacheDir.absolutePath() + "/" + result);
			if (f.open(QIODevice::ReadOnly))
			{
				entry.marqueeData = f.readAll();
				f.close();
			}
			entry.marqueeSrc = source;
		}
	}
	{
		QString type = "steamgrid";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFile f(cacheDir.absolutePath() + "/" + result);
			if (f.open(QIODevice::ReadOnly))
			{
				entry.steamgridData = f.readAll();
				f.close();
			}
			entry.steamgridSrc = source;
		}
	}
	{
		QString type = "icon";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFile f(cacheDir.absolutePath() + "/" + result);
			if (f.open(QIODevice::ReadOnly))
			{
				entry.iconData = f.readAll();
				f.close();
			}
			entry.iconSrc = source;
		}
	}
	{
		QString type = "hero";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFile f(cacheDir.absolutePath() + "/" + result);
			if (f.open(QIODevice::ReadOnly))
			{
				entry.heroData = f.readAll();
				f.close();
			}
			entry.heroSrc = source;
		}
	}
	{
		QString type = "video";
		QString result = "";
		QString source = "";
		if (fillType(type, matchingResources, result, source))
		{
			QFileInfo info(cacheDir.absolutePath() + "/" + result);
			QFile f(info.absoluteFilePath());
			if (f.open(QIODevice::ReadOnly))
			{
				entry.videoData = f.readAll();
				f.close();
				entry.videoFormat = info.suffix();
				entry.videoFile = info.absoluteFilePath();
				entry.videoSrc = source;
			}
		}
	}
}

bool Cache::fillType(QString &type, QList<Resource> &matchingResources,
                     QString &result, QString &source)
{
	QList<Resource> typeResources;
	for (const auto &resource: matchingResources)
	{
		if (resource.type == type)
		{
			typeResources.append(resource);
		}
	}
	if (typeResources.isEmpty())
	{
		return false;
	}
	if (prioMap.contains(type))
	{
		for (int a = 0; a < prioMap.value(type).length(); ++a)
		{
			for (const auto &resource: typeResources)
			{
				if (resource.source == prioMap.value(type).at(a))
				{
					result = resource.value;
					source = resource.source;
					return true;
				}
			}
		}
	}
	qint64 newest = 0;
	for (const auto &resource: typeResources)
	{
		if (resource.timestamp >= newest)
		{
			newest = resource.timestamp;
			result = resource.value;
			source = resource.source;
		}
	}
	return true;
}
