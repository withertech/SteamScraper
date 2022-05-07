#!/bin/bash
{
    # shellcheck disable=SC2006
    LATEST=$(wget -q -O - "https://api.github.com/repos/withertech/SteamScraper/releases/latest" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')

    if [ ! -f VERSION ]
    then
	echo "VERSION=0.0.0" > VERSION
    fi
    source VERSION

    handle_error () {
	local EXITCODE=$?
	local ACTION=$1
	rm --force VERSION
	echo "--- Failed to $ACTION SteamScraper v.${LATEST}, exiting with code $EXITCODE ---"
	exit $EXITCODE
    }

    if [ "$LATEST" != "$VERSION" ]
    then
	echo "--- Fetching SteamScraper v.$LATEST ---"
	wget -N https://github.com/withertech/SteamScraper/archive/"${LATEST}".tar.gz || handle_error "fetch"
	echo "--- Unpacking ---"
	tar xvzf "${LATEST}".tar.gz --strip-components 1 --overwrite || handle_error "unpack"
	rm "${LATEST}".tar.gz
	echo "--- Cleaning out old build if one exists ---"
	rm -rf build
	mkdir build; cd build || exit; cmake .. || handle_error "clean old"
	echo "--- Building SteamScraper v.$LATEST ---"
	make -j"$(nproc)" || handle_error "build"
	echo "--- Installing SteamScraper v.$LATEST ---"
	sudo make install || handle_error "install"
	echo "--- SteamScraper has been updated to v.$LATEST ---"
    else
	echo "--- SteamScraper is already the latest version, exiting ---"
	echo "You can force a reinstall by removing the VERSION file by running rm VERSION. Then rerun ./update_steamscraper.sh afterwards."
    fi
    exit
}
