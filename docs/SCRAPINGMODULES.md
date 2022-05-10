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
* Media support: *`cover`, `screenshot`, `wheel`, `marquee`, `video`*
* Example use: `Steamscraper -p snes -s screenscraper`

ScreenScraper is probably the most versatile and complete retro gaming database out there. It searches for games using either the checksums of the files or by comparing the *exact* file name to entries in their database.

It can be used for gathering data for pretty much all platforms, but it does have issues with platforms that are ISO based. Still, even for those platforms, it does locate some games.

It has the best support for the `wheel` and `marquee` artwork types of any of the databases, and also contains videos for a lot of the games.

I strongly recommend supporting them by contributing data to the database, or by supporting them with a bit of money. This can also give you more threads to scrape with.

Note! *Exact* file name matching does not work well for the `arcade` derived platforms in cases where a data checksum doesn't match. The reason being that `arcade` and other arcade-like platforms are made up of several subplatforms. Each of those subplatforms have a high chance of containing the same file name entry. In those cases ScreenScraper can't determine a unique game and will return an empty result.

#### TheGamesDb (TGDB)
* Shortname: *`thegamesdb`*
* Type: *Online*
* Website: *[www.thegamesdb.net](http://www.thegamesdb.net)*
* Type: *File name search based*
* User credential support: *Not required*
* API request limit: *Limited to 3000 requests per IP per month*
* Thread limit: *None*
* Platform support: *[Link to list](https://thegamesdb.net/list_platforms.php)*
* Media support: *`cover`, `screenshot`, `wheel`, `marquee`*
* Example use: `Steamscraper -p snes -s thegamesdb`

For newer games there's no way around TheGamesDb. It recently had a huge redesign and their database remains one of the best out there. I would recommend scraping your roms with `screenscraper` first, and then use `thegamesdb` to fill out the gaps in your cache.

There's a small caveat to this module, as it has a monthly request limit (see above) per IP per month. But this should be plenty for most people.

Their API is based on a file name search. This means that the returned results do have a chance of being faulty. Steamscraper does a lot internally to make sure accepted data is for the correct game. But it is impossible to ensure 100% correct results, so do keep that in mind when using it. Consider using the `--flags interactive` command line flag if you want complete control of the accepted entries.

#### ArcadeDB by motoschifo
* Shortname: *`arcadedb`*
* Type: *Online*
* Website: *[adb.arcadeitalia.net](http://adb.arcadeitalia.net), [youtube](https://www.youtube.com/c/ArcadeDatabase)*
* Contact: *arcadedatabase@gmail.com*
* Type: *Mame file name id based*
* User credential support: *None required*
* API request limit: *None*
* Thread limit: *1*
* Platform support: *Exclusively arcade platforms using official MAME files*
* Media support: *`cover`, `screenshot`, `wheel`, `marquee`, `video`*
* Example use: `Steamscraper -p fba -s arcadedb`

Several Arcade databases using the MAME file name id's have existed throughout the years. Currently the best one, in my opinion, is the ArcadeDB made by motoschifo. It goes without saying that this module is best used for arcade platforms such as `fba`, `arcade` and any of the mame sub-platforms.

As it relies on the MAME file name id when searching, there's no use trying to use this module for any non-MAME files. It won't give you any results.

This module also supports videos for many games.

#### OpenRetro
* Shortname: *`openretro`*
* Type: *Online*
* Website: *[www.openretro.org](https://www.openretro.org)*
* Type: *WHDLoad uuid based, File name search based*
* User credential support: *None required*
* API request limit: *None*
* Thread limit: *1*
* Platform support: *Primarily Amiga, but supports others as well. [Check list to the right](https://openretro.org/browse/amiga/a)*
* Media support: *`cover`, `screenshot`, `marquee`*
* Example use: `Steamscraper -p amiga -s openretro`

If you're looking to scrape the Amiga RetroPlay LHA files, there's no better way to do this than using the `openretro` module. It is by far the best WHDLoad Amiga database on the internet when it comes to data scraping, and maybe even the best Amiga game info database overall.

The database also supports many non-Amiga platforms, but there's no doubt that Amiga is the strong point.

#### MobyGames
* Shortname: *`mobygames`*
* Type: *Online*
* Website: *[www.mobygames.com](https://www.mobygames.com)*
* Type: *File name search based*
* User credential support: *None required*
* API request limit: *1 request per 10 seconds*
* Rom limit per run: *35*
* Thread limit: *1*
* Platform support: *[List](https://www.mobygames.com/browse/games)*
* Media support: *`cover`, `screenshot`*
* Example use:
  * `Steamscraper -p fba -s mobygames <SINGLE FILE TO SCRAPE>`
  * `Steamscraper -p fba -s mobygames --startat <FILE TO START AT> --endat <FILE TO END AT>`

MobyGames. What can I say. If you haven't heard about this database before you've been missing out. It's one of the best and oldest games databases on the internet. You'll probably come across references to MobyGames on other sites when searching for retro games. There's a reason for that - it's that good.

There's a caveat to the module as it has quite strong restrictions for the number of requests that are allowed at any given time. This restriction is global for the entire Steamscraper user base, which means that it might quit on you if other users are currently scraping from it. For this reason it has been strongly limited inside of Steamscraper by forcing a maximum number of rom scrapings per run.

Please use this module sparingly. And only ever use it to scrape those last few roms you can't get any data for using any of the other sources.

#### IGDB
![IGDB logo](resources/igdb.png)
* Shortname: *`igdb`*
* Type: *Online*
* Website: *[www.igdb.com](https://www.igdb.com)*
* Type: *File name search based*
* User credential support: *Yes, free private API client-id and secret-key required! Read more below*
* API request limit: *A maximum of 4 requests per seconds is allowed*
* Thread limit: *4 (each being limited to 1 request per second)*
* Platform support: *[List](https://www.igdb.com/platforms)*
* Media support: *None*
* Example use:
  * `Steamscraper -p fba -s igdb <SINGLE FILE TO SCRAPE>`
  * `Steamscraper -p fba -s igdb --startat <FILE TO START AT> --endat <FILE TO END AT>`

IGDB is a relatively new database on the market. But absolutely not a bad one at that. It has a couple caveats though, as the database doesn't distinguish between platform versions of the same game when it comes to any artwork resources (they are working to implement this at some point). This makes it less usable in a retro game scraping context as many of the games differ drastically visually between the old platforms. For that reason alone, this module will only provide textual data for your roms for the time being.

It is *required* to register with the Twitch dev program (IGDB is owned by Twitch) and create a free client-id and secret-key pair for use with Steamscraper. The process of getting this free client-id and secret-key pair is quite easy. Just follow the following steps:
* Go [here](https://dev.twitch.tv/login) and sign up for an account
* [Enable](https://www.twitch.tv/settings/security) two-factor authentication (required)
* [Register](https://dev.twitch.tv/console/apps/create) an application (call it whatever you like)
* [Manage](https://dev.twitch.tv/console/apps) the application
* Add ```http://localhost``` as OAuth redirect URL
* Generate a secret-key by clicking ``New secret``
* Add your client-id and secret-key pair to the Steamscraper config ini (```/home/USER/.steamscraper/config.ini```):
```
[igdb]
userCreds="CLIENTID:SECRETKEY"
```
Substitute CLIENTID and SECRETKEY with your own details. And that's it, you should now be able to use the IGDB module.

#### World of Spectrum
* Shortname: *`worldofspectrum`*
* Type: *Online*
* Website: *[www.worldofspectrum.org](http://www.worldofspectrum.org)*
* Type: *File name search based*
* User credential support: *None required*
* API request limit: *None*
* Thread limit: *None*
* Platform support: *Exclusively ZX Spectrum games*
* Media support: *`cover`, `screenshot`*
* Example use: `Steamscraper -p zxspectrum -s worldofspectrum`

If you're looking specifically for ZX Spectrum data, this is the module to use. World of Spectrum is probably the most complete ZX Spectrum resource and information database in existence. I strongly recommend visiting the site if you have any interest in these little machines. It's a cornucopia of information on the platform.

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
* Media support: *`cover`, `screenshot`, `wheel`, `marquee`, `video`*
* Example use: `Steamscraper -p snes -s import`

Read a thorough description of this module [here](IMPORT.md).
