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


# Playing

Make sure all the .dll files and the [data](https://github.com/0xCryden/Dungeon-Siege-Online/tree/master/server/data) folder from the [bin](https://github.com/0xCryden/Dungeon-Siege-Online/tree/master/bin) folder are in the same directory as the [dsmmorpg.exe](https://github.com/0xCryden/Dungeon-Siege-Online/blob/master/bin/dsmmorpg.exe)

Launch the Server ([dsmmorpg.exe](https://github.com/0xCryden/Dungeon-Siege-Online/blob/master/bin/dsmmorpg.exe))

Make sure to accept any Firewall prompts

Launch the game

Again make sure accept any new Firewall prompts

Log in as any of the existing [accounts](https://github.com/0xCryden/Dungeon-Siege-Online/blob/master/server/data/dynamic/accounts.xml) (eg. Username "aanderse" and password "a") (Username is case sensitive)


# Current Challenges

Saving Spellbooks spells positions / Saving item containers items

(currently spells from your spellbook get moved to your inventory after relogging)

Changing the world map

(currently when changing the map in the client it still tries to place the characters on the old map)

Item non-redundance

(When picking up an item and the server restarts the item will exist twice, once in inventory and once on floor)
