# vita-presence-the-server
This project reimplements desktop application required for Electry's VitaPresence plugin to work. Also adds support for Discord Rich Presence game thumbnails.

1. Install VitaPresence kernel plugin, see https://github.com/Electry/VitaPresence
2. Create application on Discord developer portal, get application ID
3. (Optional) Setup game thumbnails. In developer portal open Rich Presence -> Art Assets (on the left) and upload thumbnails. Make sure to name it exactly as the game's ID. Example: pcse00120 for Persona 4 Golden. Discord will automatically lowercase filenames. For LiveArea name it livearea.
4. Make sure vita-presence-the-server.ini is on the same level as the executable
5. Set parameters in vita-presence-the-server.ini, the keys are self-explanatory
6. Start the executable

If all goes well, your Discord nickname should be printed together with PS Vita game if you have it opened.

To stop the program, press Ctrl-C.

# Building
Compilation is tested under Linux and MinGW-w64
```
git clone https://github.com/TheMightyV/vita-presence-the-server
cd vita-presence-the-server
git submodule update --init
mkdir build && cd build
(if MinGW) cmake -G"MinGW Makefiles" ..
(if Linux) cmake ..
make
```

# Credits
1. Electry for VitaPresence https://github.com/Electry/VitaPresence
2. Ben Hoyt (benhoyt) for inih library https://github.com/benhoyt/inih
3. Cylix for tacopie TCP library (using my fork) https://github.com/Cylix/tacopie
