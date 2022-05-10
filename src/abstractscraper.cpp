/***************************************************************************
 *            abstractscraper.cpp
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

#include "abstractscraper.h"
#include "nametools.h"
#include "platform.h"
#include "strtools.h"

#include <QDomDocument>
#include <QRegularExpression>

AbstractScraper::AbstractScraper(Settings *config,
                                 QSharedPointer<NetManager> manager)
    : config(config)
{
	netComm = new NetComm(manager);
	connect(netComm, &NetComm::dataReady, &q, &QEventLoop::quit);
}

AbstractScraper::~AbstractScraper()
{
	delete netComm;
}

void AbstractScraper::getSearchResults(QList<GameEntry> &gameEntries,
                                       QString searchName, QString platform)
{
	netComm->request(searchUrlPre + searchName + searchUrlPost);
	q.exec();
	gameData = netComm->getData();

	GameEntry game;

	while (gameData.indexOf(searchResultPre.toUtf8()) != -1)
	{
		nomNom(searchResultPre);

		// Digest until url
		for (const auto &nom: urlPre)
		{
			nomNom(nom);
		}
		game.url = baseUrl + "/" + gameData.left(gameData.indexOf(urlPost.toUtf8()));

		// Digest until title
		for (const auto &nom: titlePre)
		{
			nomNom(nom);
		}
		game.title = gameData.left(gameData.indexOf(titlePost.toUtf8()));

		// Digest until platform
		for (const auto &nom: platformPre)
		{
			nomNom(nom);
		}
		game.platform = gameData.left(gameData.indexOf(platformPost.toUtf8()));

		if (platformMatch(game.platform, platform))
		{
			gameEntries.append(game);
		}
	}
}

void AbstractScraper::getGameData(GameEntry &game)
{
	netComm->request(game.url);
	q.exec();
	gameData = netComm->getData();
	//printf("URL IS: '%s'\n", game.url.toStdString().c_str());
	//printf("DATA IS:\n'%s'\n", data.data());

	for (int a = 0; a < fetchOrder.length(); ++a)
	{
		switch (fetchOrder.at(a))
		{
			case DESCRIPTION:
				getDescription(game);
				break;
			case DEVELOPER:
				getDeveloper(game);
				break;
			case PUBLISHER:
				getPublisher(game);
				break;
			case PLAYERS:
				getPlayers(game);
				break;
			case AGES:
				getAges(game);
				break;
			case RATING:
				getRating(game);
				break;
			case TAGS:
				getTags(game);
				break;
			case RELEASEDATE:
				getReleaseDate(game);
				break;
			case COVER:
				getCover(game);
				break;
			case SCREENSHOT:
				getScreenshot(game);
				break;
			case LOGO:
				getLogo(game);
				break;
			case MARQUEE:
				getMarquee(game);
				break;
			case STEAMGRID:
				getSteamgrid(game);
				break;
			case HERO:
				getHero(game);
				break;
			case VIDEO:
				if (config->videos)
				{
					getVideo(game);
				}
				break;
			default:;
		}
	}
}

void AbstractScraper::getDescription(GameEntry &game)
{
	if (descriptionPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: descriptionPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: descriptionPre)
	{
		nomNom(nom);
	}

	game.description = gameData.left(gameData.indexOf(descriptionPost.toUtf8())).replace("&lt;", "<").replace("&gt;", ">");
	game.description = game.description.replace("\\n", "\n");

	// Remove all html tags within description
	game.description = StrTools::stripHtmlTags(game.description);
}

void AbstractScraper::getDeveloper(GameEntry &game)
{
	for (const auto &nom: developerPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: developerPre)
	{
		nomNom(nom);
	}
	game.developer = gameData.left(gameData.indexOf(developerPost.toUtf8()));
}

void AbstractScraper::getPublisher(GameEntry &game)
{
	if (publisherPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: publisherPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: publisherPre)
	{
		nomNom(nom);
	}
	game.publisher = gameData.left(gameData.indexOf(publisherPost.toUtf8()));
}

void AbstractScraper::getPlayers(GameEntry &game)
{
	if (playersPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: playersPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: playersPre)
	{
		nomNom(nom);
	}
	game.players = gameData.left(gameData.indexOf(playersPost.toUtf8()));
}

void AbstractScraper::getAges(GameEntry &game)
{
	if (agesPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: agesPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: agesPre)
	{
		nomNom(nom);
	}
	game.ages = gameData.left(gameData.indexOf(agesPost.toUtf8()));
}

void AbstractScraper::getTags(GameEntry &game)
{
	if (tagsPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: tagsPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: tagsPre)
	{
		nomNom(nom);
	}
	game.tags = gameData.left(gameData.indexOf(tagsPost.toUtf8()));
}

void AbstractScraper::getRating(GameEntry &game)
{
	if (ratingPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: ratingPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: ratingPre)
	{
		nomNom(nom);
	}
	game.rating = gameData.left(gameData.indexOf(ratingPost.toUtf8()));
	bool toDoubleOk = false;
	double rating = game.rating.toDouble(&toDoubleOk);
	if (toDoubleOk)
	{
		game.rating = QString::number(rating / 5.0);
	}
	else
	{
		game.rating = "";
	}
}

void AbstractScraper::getReleaseDate(GameEntry &game)
{
	if (releaseDatePre.isEmpty())
	{
		return;
	}
	for (const auto &nom: releaseDatePre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: releaseDatePre)
	{
		nomNom(nom);
	}
	game.releaseDate = gameData.left(gameData.indexOf(releaseDatePost.toUtf8())).simplified();
}

void AbstractScraper::getCover(GameEntry &game)
{
	if (coverPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: coverPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: coverPre)
	{
		nomNom(nom);
	}
	QString coverUrl = gameData.left(gameData.indexOf(coverPost.toUtf8())).replace("&amp;", "&");
	if (coverUrl.left(4) != "http")
	{
		coverUrl.prepend(baseUrl + (coverUrl.left(1) == "/" ? "" : "/"));
	}
	netComm->request(coverUrl);
	q.exec();
	QImage image;
	if (netComm->getError() == QNetworkReply::NoError &&
	    image.loadFromData(netComm->getData()))
	{
		game.coverData = netComm->getData();
	}
}

void AbstractScraper::getScreenshot(GameEntry &game)
{
	if (screenshotPre.isEmpty())
	{
		return;
	}
	// Check that we have enough screenshots
	int screens = gameData.count(screenshotCounter.toUtf8());
	if (screens >= 1)
	{
		for (int a = 0; a < screens - (screens / 2); a++)
		{
			for (const auto &nom: screenshotPre)
			{
				nomNom(nom);
			}
		}
		QString screenshotUrl = gameData.left(gameData.indexOf(screenshotPost.toUtf8())).replace("&amp;", "&");
		if (screenshotUrl.left(4) != "http")
		{
			screenshotUrl.prepend(baseUrl + (screenshotUrl.left(1) == "/" ? "" : "/"));
		}
		netComm->request(screenshotUrl);
		q.exec();
		QImage image;
		if (netComm->getError() == QNetworkReply::NoError &&
		    image.loadFromData(netComm->getData()))
		{
			game.screenshotData = netComm->getData();
		}
	}
}

void AbstractScraper::getLogo(GameEntry &game)
{
	if (wheelPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: wheelPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: wheelPre)
	{
		nomNom(nom);
	}
	QString logoUrl = gameData.left(gameData.indexOf(wheelPost.toUtf8())).replace("&amp;", "&");
	if (logoUrl.left(4) != "http")
	{
		logoUrl.prepend(baseUrl + (logoUrl.left(1) == "/" ? "" : "/"));
	}
	netComm->request(logoUrl);
	q.exec();
	QImage image;
	if (netComm->getError() == QNetworkReply::NoError &&
	    image.loadFromData(netComm->getData()))
	{
		game.logoData = netComm->getData();
	}
}

void AbstractScraper::getMarquee(GameEntry &game)
{
	if (marqueePre.isEmpty())
	{
		return;
	}
	for (const auto &nom: marqueePre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: marqueePre)
	{
		nomNom(nom);
	}
	QString marqueeUrl = gameData.left(gameData.indexOf(marqueePost.toUtf8())).replace("&amp;", "&");
	if (marqueeUrl.left(4) != "http")
	{
		marqueeUrl.prepend(baseUrl + (marqueeUrl.left(1) == "/" ? "" : "/"));
	}
	netComm->request(marqueeUrl);
	q.exec();
	QImage image;
	if (netComm->getError() == QNetworkReply::NoError &&
	    image.loadFromData(netComm->getData()))
	{
		game.marqueeData = netComm->getData();
	}
}

void AbstractScraper::getSteamgrid(GameEntry &game)
{
	if (steamgridPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: steamgridPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: steamgridPre)
	{
		nomNom(nom);
	}
	QString steamgridUrl = gameData.left(gameData.indexOf(steamgridPost.toUtf8())).replace("&amp;", "&");
	if (steamgridUrl.left(4) != "http")
	{
		steamgridUrl.prepend(baseUrl + (steamgridUrl.left(1) == "/" ? "" : "/"));
	}
	netComm->request(steamgridUrl);
	q.exec();
	QImage image;
	if (netComm->getError() == QNetworkReply::NoError &&
	    image.loadFromData(netComm->getData()))
	{
		game.steamgridData = netComm->getData();
	}
}

void AbstractScraper::getIcon(GameEntry &game)
{
	if (iconPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: iconPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: iconPre)
	{
		nomNom(nom);
	}
	QString iconUrl = systemData.left(systemData.indexOf(iconPost.toUtf8())).replace("&amp;", "&");
	if (iconUrl.left(4) != "http")
	{
		iconUrl.prepend(baseUrl + (iconUrl.left(1) == "/" ? "" : "/"));
	}
	netComm->request(iconUrl);
	q.exec();
	QImage image;
	if (netComm->getError() == QNetworkReply::NoError &&
	    image.loadFromData(netComm->getData()))
	{
		game.iconData = netComm->getData();
	}
}

void AbstractScraper::getHero(GameEntry &game)
{
	if (heroPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: heroPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: heroPre)
	{
		nomNom(nom);
	}
	QString heroUrl = gameData.left(gameData.indexOf(heroPost.toUtf8())).replace("&amp;", "&");
	if (heroUrl.left(4) != "http")
	{
		heroUrl.prepend(baseUrl + (heroUrl.left(1) == "/" ? "" : "/"));
	}
	netComm->request(heroUrl);
	q.exec();
	QImage image;
	if (netComm->getError() == QNetworkReply::NoError &&
	    image.loadFromData(netComm->getData()))
	{
		game.heroData = netComm->getData();
	}
}

void AbstractScraper::getVideo(GameEntry &game)
{
	if (videoPre.isEmpty())
	{
		return;
	}
	for (const auto &nom: videoPre)
	{
		if (!checkNom(nom))
		{
			return;
		}
	}
	for (const auto &nom: videoPre)
	{
		nomNom(nom);
	}
	QString videoUrl = gameData.left(gameData.indexOf(videoPost.toUtf8())).replace("&amp;", "&");
	if (videoUrl.left(4) != "http")
	{
		videoUrl.prepend(baseUrl + (videoUrl.left(1) == "/" ? "" : "/"));
	}
	netComm->request(videoUrl);
	q.exec();
	if (netComm->getError() == QNetworkReply::NoError)
	{
		game.videoData = netComm->getData();
		game.videoFormat = videoUrl.right(3);
	}
}

void AbstractScraper::nomNom(const QString nom, bool including)
{
	gameData.remove(0, gameData.indexOf(nom.toUtf8()) + (including ? nom.length() : 0));
}

bool AbstractScraper::checkNom(const QString nom)
{
	if (gameData.indexOf(nom.toUtf8()) != -1)
	{
		return true;
	}
	return false;
}

QList<QString> AbstractScraper::getSearchNames(const QFileInfo &info)
{
	QString baseName = info.completeBaseName();

	if (config->scraper != "import")
	{
		if (!config->aliasMap[baseName].isEmpty())
		{
			baseName = config->aliasMap[baseName];
		}
		else if (info.suffix() == "lha")
		{
			QString nameWithSpaces = config->whdLoadMap[baseName].first;
			if (nameWithSpaces.isEmpty())
			{
				baseName = NameTools::getNameWithSpaces(baseName);
			}
			else
			{
				baseName = nameWithSpaces;
			}
		}
		else if (config->platform == "scummvm")
		{
			baseName = NameTools::getScummName(baseName, config->scummIni);
		}
		else if ((config->platform == "neogeo" ||
		          config->platform == "arcade" ||
		          config->platform == "mame-advmame" ||
		          config->platform == "mame-libretro" ||
		          config->platform == "mame-mame4all" ||
		          config->platform == "fba") &&
		         !config->mameMap[baseName].isEmpty())
		{
			baseName = config->mameMap[baseName];
		}
	}

	QList<QString> searchNames;

	if (baseName.isEmpty())
		return searchNames;

	searchNames.append(NameTools::getUrlQueryName(baseName));

	if (baseName.contains(":") || baseName.contains(" - "))
	{
		QString noSubtitle = baseName.left(baseName.indexOf(":")).simplified();
		noSubtitle = noSubtitle.left(noSubtitle.indexOf(" - ")).simplified();
		// Only add if longer than 3. We don't want to search for "the" for instance
		if (noSubtitle.length() > 3)
			searchNames.append(NameTools::getUrlQueryName(noSubtitle));
	}

	if (NameTools::hasRomanNumeral(baseName) || NameTools::hasIntegerNumeral(baseName))
	{
		if (NameTools::hasRomanNumeral(baseName))
		{
			baseName = NameTools::convertToIntegerNumeral(baseName);
		}
		else if (NameTools::hasIntegerNumeral(baseName))
		{
			baseName = NameTools::convertToRomanNumeral(baseName);
		}
		searchNames.append(NameTools::getUrlQueryName(baseName));

		if (baseName.contains(":") || baseName.contains(" - "))
		{
			QString noSubtitle = baseName.left(baseName.indexOf(":")).simplified();
			noSubtitle = noSubtitle.left(noSubtitle.indexOf(" - ")).simplified();
			// Only add if longer than 3. We don't want to search for "the" for instance
			if (noSubtitle.length() > 3)
				searchNames.append(NameTools::getUrlQueryName(noSubtitle));
		}
	}

	return searchNames;
}

QString AbstractScraper::getCompareTitle(QFileInfo info)
{
	QString baseName = info.completeBaseName();

	if (config->scraper != "import")
	{
		if (!config->aliasMap[baseName].isEmpty())
		{
			baseName = config->aliasMap[baseName];
		}
		else if (info.suffix() == "lha")
		{
			QString nameWithSpaces = config->whdLoadMap[baseName].first;
			if (nameWithSpaces.isEmpty())
			{
				baseName = NameTools::getNameWithSpaces(baseName);
			}
			else
			{
				baseName = nameWithSpaces;
			}
		}
		else if (config->platform == "scummvm")
		{
			baseName = NameTools::getScummName(baseName, config->scummIni);
		}
		else if ((config->platform == "neogeo" ||
		          config->platform == "arcade" ||
		          config->platform == "mame-advmame" ||
		          config->platform == "mame-libretro" ||
		          config->platform == "mame-mame4all" ||
		          config->platform == "fba") &&
		         !config->mameMap[baseName].isEmpty())
		{
			baseName = config->mameMap[baseName];
		}
	}

	// Now create actual compareTitle
	baseName = baseName.replace("_", " ").left(baseName.indexOf("(")).left(baseName.indexOf("[")).simplified();

	QRegularExpressionMatch match;

	// Always move ", The" to the beginning of the name
	match = QRegularExpression(", [Tt]he").match(baseName);
	if (match.hasMatch())
	{
		baseName = baseName.replace(match.captured(0), "").prepend(match.captured(0).right(3) + " ");
	}

	// Remove "vX.XXX" versioning string if one is found
	match = QRegularExpression(" v[.]{0,1}([0-9]{1}[0-9]{0,2}[.]{0,1}[0-9]{1,4}|[IVX]{1,5})$").match(baseName);
	if (match.hasMatch() && match.capturedStart(0) != -1)
	{
		baseName = baseName.left(match.capturedStart(0)).simplified();
	}

	return baseName;
}

void AbstractScraper::runPasses(QList<GameEntry> &gameEntries, const QFileInfo &info, QString &output, QString &debug)
{
	// Reset region priorities to original list from Settings
	regionPrios = config->regionPrios;
	// Autodetect region and append to region priorities
	if (info.fileName().indexOf("(") != -1 && config->region.isEmpty())
	{
		QString regionString = info.fileName().toLower().mid(info.fileName().indexOf("("), info.fileName().length());
		if (regionString.contains("europe") || regionString.contains("(e)"))
		{
			regionPrios.prepend("eu");
		}
		if (regionString.contains("usa") || regionString.contains("(u)"))
		{
			regionPrios.prepend("us");
		}
		if (regionString.contains("world"))
		{
			regionPrios.prepend("wor");
		}
		if (regionString.contains("japan") || regionString.contains("(j)"))
		{
			regionPrios.prepend("jp");
		}
		if (regionString.contains("brazil"))
		{
			regionPrios.prepend("br");
		}
		if (regionString.contains("korea"))
		{
			regionPrios.prepend("kr");
		}
		if (regionString.contains("taiwan"))
		{
			regionPrios.prepend("tw");
		}
		if (regionString.contains("france"))
		{
			regionPrios.prepend("fr");
		}
		if (regionString.contains("germany"))
		{
			regionPrios.prepend("de");
		}
		if (regionString.contains("italy"))
		{
			regionPrios.prepend("it");
		}
		if (regionString.contains("spain"))
		{
			regionPrios.prepend("sp");
		}
		if (regionString.contains("china"))
		{
			regionPrios.prepend("cn");
		}
		if (regionString.contains("australia"))
		{
			regionPrios.prepend("au");
		}
		if (regionString.contains("sweden"))
		{
			regionPrios.prepend("se");
		}
		if (regionString.contains("canada"))
		{
			regionPrios.prepend("ca");
		}
		if (regionString.contains("netherlands"))
		{
			regionPrios.prepend("nl");
		}
		if (regionString.contains("denmark"))
		{
			regionPrios.prepend("dk");
		}
		if (regionString.contains("asia"))
		{
			regionPrios.prepend("asi");
		}
	}

	QList<QString> searchNames;
	if (config->searchName.isEmpty())
	{
		searchNames = getSearchNames(info);
	}
	else
	{
		// Add the string provided by "--query"
		searchNames.append(config->searchName);
	}

	if (searchNames.isEmpty())
	{
		return;
	}

	for (int pass = 1; pass <= searchNames.size(); ++pass)
	{
		output.append("\033[1;35mPass " + QString::number(pass) + "\033[0m ");
		getSearchResults(gameEntries, searchNames.at(pass - 1), config->platform);
		debug.append("Tried with: '" + searchNames.at(pass - 1) + "'\n");
		debug.append("Platform: " + config->platform + "\n");
		if (!gameEntries.isEmpty())
		{
			break;
		}
	}
}

bool AbstractScraper::platformMatch(QString found, QString platform)
{
	for (const auto &p: Platform::getAliases(platform))
	{
		if (found.toLower() == p)
		{
			return true;
		}
	}
	return false;
}

QString AbstractScraper::getPlatformId(const QString)
{
	return "na";
}
