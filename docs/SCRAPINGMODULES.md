## Scraping module overview
Steamscraper supports several online and local sources when scraping data for your roms. This makes Steamscraper a hugely versatile tool since it also caches any resources that are gathered from any of the modules. The cached data can then be used to generate a game list and composite artwork later.

Choosing a scraping module is as simply as setting the `-s <MODULE>` option when running Steamscraper on the command line. It also requires a platform to be set with `-p <PLATFORM>`. If you leave out the `-s` option Steamscraper goes into *game list generation* mode and combines your cached data into a game list for the chosen platform and frontend. Read more about the resource cache [here](CACHE.md).

For scraping modules that support or require user credentials you have the option of either setting it on commandline with `-u <USER:PASSWD>` or `-u <KEY>` or better yet, by adding it permanently to the Steamscraper configuration at `/home/USER/.steamscraper/config.ini` as described [here](https://github.com/withertech/steamscraper/blob/master/docs/CONFIGINI.md#usercredscredentials-or-key)

Below follows a description of all scraping modules.

#### ScreenScraper
* Shortname: *`screenscraper`*
* Type: *Online*
* Website: *[www.screenscraper.fr](https://www.screenscraper.fr)*
* Type: *Rom checksum based, Exact file name based*
* User credential support: *Yes, and strongly recommended, but not required*
* API request limit: *20k per day for registered users*
* Thread limit: *1 or more depending on user credentials*
* Platform support: *[Check list under "Systémes"](https://www.screenscraper.fr)*
* Media support: *`cover`, `screenshot`, `logo`, `marquee`, `steamgrid`, `icon`, `video`*
* Example use: `Steamscraper -p snes -s screenscraper`

ScreenScraper is probably the most versatile and complete retro gaming database out there. It searches for games using either the checksums of the files or by comparing the *exact* file name to entries in their database.

It can be used for gathering data for pretty much all platforms, but it does have issues with platforms that are ISO based. Still, even for those platforms, it does locate some games.

It has the best support for the `logo` and `marquee` artwork types of the databases, and also contains videos for a lot of the games.

I strongly recommend supporting them by contributing data to the database, or by supporting them with a bit of money. This can also give you more threads to scrape with.

Note! *Exact* file name matching does not work well for the `arcade` derived platforms in cases where a data checksum doesn't match. The reason being that `arcade` and other arcade-like platforms are made up of several subplatforms. Each of those subplatforms have a high chance of containing the same file name entry. In those cases ScreenScraper can't determine a unique game and will return an empty result.

#### EmulationStation game list
* Shortname: *`esgamelist`*
* Type: *Local*
* Website: *[https://emulationstation.org](https://emulationstation.org)*
* Type: *Exact file name match*
* User credential support: *None required*
* API request limit: *None*
* Thread limit: *None*
* Platform support: *All*
* Media support: *`screenshot`, `marquee`, `video`*
* Example use: `Steamscraper -p snes -s esgamelist`

This module allows you to import data from an existing EmulationStation game list into the Steamscraper cache. This is useful if you already have a lot of data and artwork in a gamelist.xml file and you wish to use it with Steamscraper.

Steamscraper will search for the `gamelist.xml` file at `<INPUT FOLDER>/gamelist.xml` which by default is `/home/<USER>/RetroPie/roms/<PLATFORM>/gamelist.xml`. If that file isn't found it will look for it at `/home/USER/.steamscraper/import/<PLATFORM>/gamelist.xml`.

#### Custom resource import scraper
* Shortname: *`import`*
* Type: *Local*
* Website: *[Documentation@github](IMPORT.md)*
* Type: *Exact file name match*
* User credential support: *None required*
* API request limit: *None*
* Thread limit: *None*
* Platform support: *All*
* Media support: *`cover`, `screenshot`, `logo`, `marquee`, `steamgrid`, `icon`, `hero`, `video`*
* Example use: `Steamscraper -p snes -s import`

Read a thorough description of this module [here](IMPORT.md).
