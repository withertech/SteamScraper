/***************************************************************************
 *            skyscraper.h
 *
 *  Wed Jun 7 12:00:00 CEST 2017
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

#ifndef SKYSCRAPER_H
#define SKYSCRAPER_H

#include "abstractfrontend.h"
#include "cache.h"
#include "netcomm.h"
#include "netmanager.h"
#include "platform.h"
#include "scraperworker.h"
#include "settings.h"

#include <QElapsedTimer>
#include <QFile>
#include <QObject>

#include <QCommandLineParser>

class Steamscraper : public QObject
{
	Q_OBJECT

public:
	Steamscraper(const QCommandLineParser &parser, const QString &currentDir);

	~Steamscraper();

	QSharedPointer<Queue> queue;
	QSharedPointer<NetManager> manager;
	int state = 0;

public slots:

	void run();

signals:

	void finished();

private slots:

	void entryReady(const GameEntry &entry, const QString &output, const QString &debug);

	void checkThreads();

private:
	Settings config;

	void loadConfig(const QCommandLineParser &parser);

	void copyFile(const QString &distro, const QString &current, bool overwrite = true);

	QString secsToString(const int &seconds);

	void checkForFolder(QDir &folder, bool create = true);

	void showHint();

	void doPrescrapeJobs();

	void loadAliasMap();

	void loadMameMap();

	void loadWhdLoadMap();

	void setRegionPrios();

	void setLangPrios();
	//void migrate(QString filename);

	AbstractFrontend *frontend;

	QSharedPointer<Cache> cache;

	QList<GameEntry> gameEntries;
	QList<QString> cliFiles;
	QMutex entryMutex;
	QMutex checkThreadMutex;
	QElapsedTimer timer;
	QString gameListFileString;
	QString skippedFileString;
	int doneThreads;
	int notFound;
	int found;
	int avgSearchMatch;
	int avgCompleteness;
	int currentFile;
	int totalFiles;
};

#endif// SKYSCRAPER_H
