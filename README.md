 Updated Windows version of [dsmmorpg](https://github.com/sbrkopac/dsmmorpg) by [brkopac](https://github.com/sbrkopac)  

 "This project replaces the multiplayer server for the commercial game Dungeon Siege. It is a standalone server which a modified version of the client Dungeon Siege can connect to in order to play large scale multiplayer games in a persistent setting."


# Installation

Use Dungeon Siege 1 Base game without LoA or it will not work. 

Download the [bin](https://github.com/0xCryden/Dungeon-Siege-Online/tree/master/bin) folder.

Put [dsmmorpg.dsdll](https://github.com/0xCryden/Dungeon-Siege-Online/blob/master/bin/dsmmorpg.dsdll) where your DungeonSiege.exe is located

Put [dsmmorpg.dsres](https://github.com/0xCryden/Dungeon-Siege-Online/blob/master/bin/dsmmorpg.dsres) inside Dungeon Sieges resource folder

To eliminate compatibility issues remove your other mods (.dsres files) except the base games (Logic, Objects, Sound, Terrain, Voices)

Edit your DungeonSiege.ini inside your documents' Dungeon Siege folder (eg. C:\Users\<USERNAME>\Documents\Dungeon Siege) and add
```
[server]
ip:0 = 127.0.0.1
```


# Current State

Can create new account & characters (settings are WIP)

Can join UP, walk around, fight with other players, level up, use & equip weapons & armor & spellbooks

All relevant data gets saved for server restarts

Server reads given .gas files but conversion to Gos is WIP

Server generates missing map information when needed during runtime


# Playing

Make sure all the .dll files and the [data](https://github.com/0xCryden/Dungeon-Siege-Online/tree/master/server/data) folder from the [bin](https://github.com/0xCryden/Dungeon-Siege-Online/tree/master/bin) folder are in the same directory as the [dsmmorpg.exe](https://github.com/0xCryden/Dungeon-Siege-Online/blob/master/bin/dsmmorpg.exe)

Launch the Server ([dsmmorpg.exe](https://github.com/0xCryden/Dungeon-Siege-Online/blob/master/bin/dsmmorpg.exe))

Make sure to accept any Firewall prompts

Launch the game

Again make sure accept any new Firewall prompts


# Current Challenges

Missing: spells, mobs, quests, shops, items/pcontent

If you unequip your unselected weapon and relog without overwriting the old empty slot you will have the weapon reequipped upon login

PvP toggle


# Other TODOs

Make server convert the read .gas files to Gos

Implement serversided network rate limit