// provide Discord Rich Presence functionality for Vita by using PSVita kernel "VitaPresence" by Electry
// Copyright (C) 2020 TheMightyV

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <string.h>
#include <time.h>
#include <iostream>
#include <thread>
#include <string>
#include <algorithm>
#include <csignal>

#include "tacopie/tacopie"
#include "discord_rpc.h"
#include "inih/cpp/INIReader.h"


struct UserData{
    std::string name;
    std::string id;
    std::string discriminator;
}user;

struct Game{
    unsigned int magic;
    int index;
    char titleid[10];
    char title[128];
}game;


bool work = true;
bool vitaOk = false;


static void handleDiscordReady(const DiscordUser* connectedUser){
    user.name = connectedUser->username;
    user.id = connectedUser->userId;
    user.discriminator = connectedUser->discriminator;
}

//process packet from VitaPresence plugin with game data
void on_newVitaMessage(tacopie::tcp_client& client,
                       const tacopie::tcp_client::read_result& res){
    if (res.success) {
        client.disconnect();
        vitaOk = true;
        game = *((Game*)res.buffer.data());
        if (game.magic != 0xCAFECAFE){ return; }
        if (game.title[0] == '\0'){
            strcpy(game.title, "LiveArea\0");
            strcpy(game.titleid, "livearea\0");
        }
    }
    else {
        std::cout << "Client disconnected" << std::endl;
        vitaOk = false;
    }
}

//asynchronously asks status from VitaPresence plugin
void runVita(const std::string &vitaHostname, const int vitaPort,
             const int vitaTimeout, const int updateTimer){

    const int vitaMsgSize = 600;
    tacopie::tcp_client vitaClient;

    while (work){
        try{
            vitaClient.connect(vitaHostname, vitaPort, vitaTimeout);
            vitaOk = true;
        }
        catch (...){
            vitaOk = false;
            continue;
        }

        vitaClient.async_read( {vitaMsgSize,
                                std::bind(&on_newVitaMessage,
                                std::ref(vitaClient),
                                std::placeholders::_1)});

        std::this_thread::sleep_for(std::chrono::milliseconds(updateTimer));
    }
}

//funtion handling Discord RPC
void runDiscord(const std::string &appID, int updateTimer){
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));

    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));

    std::string prevGame;
    time_t gameStartTime;

    handlers.ready = handleDiscordReady;

    Discord_Initialize(appID.c_str(), &handlers, 1, NULL);
    Discord_RunCallbacks();

    while (work){
        if (vitaOk){
            discordPresence.state = game.title;

            if (prevGame != game.title){
                gameStartTime = time(0);
            }
            discordPresence.startTimestamp = gameStartTime;

            std::string imageName = game.titleid;
            std::transform(imageName.begin(), imageName.end(),
                           imageName.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            discordPresence.largeImageKey = imageName.c_str();

            discordPresence.instance = 0;
            Discord_UpdatePresence(&discordPresence);

            prevGame = game.title;
        }
        else{
            Discord_ClearPresence();
        }

        Discord_RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(updateTimer));
    }

    Discord_ClearPresence();
    Discord_RunCallbacks();
    Discord_Shutdown();
}

void signalHandler(int signum ){
    std::cerr << "Caught signal " << signum << ", exiting gracefully...\n";
    work = false;
}

void console_clear_screen() {
  #ifdef _WIN32
  system("cls");
  #elif __unix
  system("clear");
  #endif
}

int main(int argc, char* argv[])
{
	#ifdef _WIN32
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0){
		std::cerr << "WASStartup() failure" << std::endl;
		return 1;
	}
	#endif //_WIN32

    // register signal SIGINT and signal handler for graceful shutdown
    signal(SIGINT, signalHandler);

    // reading parameters
    INIReader ini("vita-presence-the-server.ini");
    if (ini.ParseError() < 0){
        std::cerr << "Failed to find ini file" << std::endl;
        return 1;

    }

    int vitaPort = 0xCAFE;
    std::string vitaHostname = ini.GetString("General", "vitaIP", "NULL");
    std::string appID = ini.GetString("General", "appID", "NULL");
    int updateTimer = ini.GetInteger("General", "updateTimer", 5*1000); //msec
    int vitaTimeout = ini.GetInteger("General", "vitaTimeout", 5*1000); //msec

    // starting worker threads
    std::thread discordThread(runDiscord, appID, updateTimer);
    std::thread vitaThread(runVita, vitaHostname, vitaPort,
                           vitaTimeout, updateTimer);

    // display current status to terminal
    while (work){
        console_clear_screen();

        std::cout << "Discord ID: " << appID << std::endl;
        std::cout << "Discord user: ";
        if (!user.name.empty()){
            std::cout << user.name << "(#" << user.discriminator << ')'
                      <<  ' ' << user.id << std::endl;
        }
        else{
            std::cout << "not connected" << std::endl;
        }

        std::cout << "PS Vita: ";
        if (vitaOk){
            std::cout << "connected to " << vitaHostname << ':'
                      << vitaPort << std::endl;

            if (game.title[0] != '\0'){
                std::cout << "Playing: " << game.title << " (" << game.titleid
                          << ')' << std::endl;
            }
            else{
                std::cout << "At LiveArea" << std::endl;
            }
        }
        else{
            std::cout << "connecting to "  << vitaHostname << ':' <<
                         vitaPort << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(updateTimer));
    }

    std::cerr << "Cleaning up..." << std::endl;

    discordThread.join();
    vitaThread.join();

    return 0;
}
