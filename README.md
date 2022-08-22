# zeno-helpers
A suite of high-level helper functions for Age of Mythology custom mapmaking. All the cool files are in the **zeno-helpers/helpers** folder.
* **zshared.c**: This file contains a ton of useful helper functions for AoM mapmaking. Recommended for any advanced Random Map Scripting. Make sure this is the first file parsed by your rmsify.py or xmlify.py.
* **dataLoadBackend.c**: This file contains necessary code for running data loading. Only include if you plan to use the dataLoad feature. NOTE: If you want to use this feature for Multiplayer, it only works for Random Map Scripts! Scenarios cannot do multiplayer data load! (But they can still do singleplayer data load)
* **dataLoadModifyMe.c**: This file contains information that should be modified by the user if they intend to use the dataLoad feature.

## xDatabase Functions
These functions are powerful database functions that allow you to group, store, and iterate over data in an incredibly fluid manner. It utilizes a Linked List implementation to carry chunks of data, which are defined to have certain values. (If you know c programming, imagine this as a linked list of structs).

### Step 1: Initialize
Before you can use a database, you must initialize it. To do so, run **xInitDatabase()**. This function accepts two parameters:
* **name**: This is a string representing the name of the database. This is unused except for debugging purposes.
* **count**: This is the initial amount of spare blocks to be allocated for your database. This is an optional parameter, as more blocks are created when needed.

This function returns an integer, which is the unique **ID** of the database. You must save this value somewhere so that you can reference the database later. I generally declare a global integer and then assign this value to it. For example, if I globally declare an integer named dPlayerUnits, I can use it to store a new database index in a trigger like so:
``` 
    dPlayerUnits = xInitDatabase("playerUnits", 8);
```

After you've initialized a database, it is time to declare variables. Variables have five types: **int, float, string, vector,** and **bool**. Each one has its own corresponding initialize function, as follows:
```
    xUnitName = xInitAddInt(dPlayerUnits, "name");
    xUnitArmor = xInitAddFloat(dPlayerUnits, "armor", 0.5);
    xUnitProto = xInitAddString(dPlayerUnits, "proto", "Hawk");
    xUnitPos = xInitAddVector(dPlayerUnits, "position");
    xUnitActive = xInitAddBool(dPlayerUnits, "active", true);
```
Note that the parameters are the same for all of them: 
* **Database ID**: This is the unique ID of the database you want to add this variable to.
* **name**: This is a string representing the name of the variable. This is unused except for debugging purposes.
* **default value**: This optional parameter is the default value of the variable. When a new database block is created, its value for this variable will be initialized to whatever you set for this parameter.

Each one returns an integer which is the unique ID of the variable. This must be stored in an integer for use later. In the example above, you can see that I've stored the return values in integers named xUnitName, xUnitArmor, xUnitProto, xUnitPos, and xUnitActive.

Let's say that all of the above code was run. This will do the following:
1. Initialized a new database named "playerUnits" with a unique ID stored in the dPlayerUnits variable.
2. Added five variables to the dPlayerUnits database, each with their own uniqiue ID's, and some with custom default values.

Now, whenever you create a dPlayerUnits block, it will come with its own xUnitName, xUnitArmor, xUnitProto, xUnitPos, and xUnitActive variables.

### Step 2: Add a database block
You have just defined your database. However, there's nothing in it! To add something to your database, simply run the **xAddDatabaseBlock()** function. It accepts two parameters:
* **Database ID**: This is the unique ID of the database you are adding a block for
* **Look At New**: If set to true, this will set the pointer of the database to look at the newly created database block. Useful if you want to set its values after adding it to the database.

This will also return an integer which represents the index of the block you just added.
```
    xAddDatabaseBlock(dPlayerUnits);
```
If we run the above command, it will create a new block in the dPlayerUnits database. It will have its own xUnitName integer, xUnitArmor float, xUnitProto string, xUnitPos vector, and xUnitActive bool.

### Step 3: Get and set variables
Now that you have a block in your database, you can manipulate its contents however you wish, with Get and Set functions. For example:
```
    int x = xGetInt(dPlayerUnits, xUnitName);
```
This function will acquire the value of xUnitName for the currently selected database block. We store it in integer **x**.
```
    xSetFloat(dPlayerUnits, xUnitArmor, 0.6);
```
This function will set the xUnitArmor of the currently selected database block to 0.6.

### Database Iteration
I may have set the armor of one block in the database, but what if I want to do so for all blocks in the database? This is where the iteration comes in handy:
```
    xDatabaseNext(dPlayerUnits);
```
The **xDatabaseNext()** function will move the pointer down the database by one entry. It has the following parameters:
* **Database ID**: The unique database ID to run this function on
* **Reverse**: If set to true, this will move the pointer backwards instead of forwards.
```
  for(i=xGetDatabaseCount(dPlayerUnits); >0) {
    xDatabaseNext(dPlayerUnits);
    trChatSend(0, "Armor: " + xGetFloat(dPlayerUnits, xUnitArmor));
  }
```
The above code will iterate over the entire dPlayerUnits database and send a chat displaying each unit's armor value. Maybe you don't want to run this functionality for the entire database. Maybe there are a hundred blocks in the dPlayerUnits database, and running this code for each one would be too slow. You can easily vary the amount of computation as follows:
```
  for(i=3; >0) {
    xDatabaseNext(dPlayerUnits);
    trChatSend(0, "Armor: " + xGetFloat(dPlayerUnits, xUnitArmor));
  }
```
Now, instead of looking through the entire database, we only look at the next three blocks in the database! This is useful if computation is expensive and you don't need high fidelity. For example, let's say you want all your units in a database with unique bounty values. When a unit dies, it should award the players with gold. However, you don't want to check the entire database every trigger loop. So instead, you only check five or so units at a time. This will loop through the database five units per trigger loop until it eventually finds your dead unit and awards the players with gold. There might be a slight delay between when a unit dies and when the players are awarded their gold, but the runtime is significantly faster since you don't have to check EVERY unit in the database on EVERY trigger loop.

### Database Pointers
If you want to look at a specific block in the database, you can do so with **xSetPointer()**, which accepts two parameters:
* **Database ID**: The unique ID of the database to run this function on.
* **Index**: The index to set the pointer to.

This will make the database look at the block at the specified index, if it is currently in the database. If the block at that index is not currently in the database, this function will return false. You can think of this like **trUnitSelect()** but for database blocks instead of units. For example:
```
    if (xSetPointer(dPlayerUnits, 2)) {
      xSetFloat(dPlayerUnits, xUnitArmor, 0.99);
    }
```
The above code will attempt to look at database block 2. If it exists in the database, it will then set that block's armor value to 0.99.

But where did this number 2 come from? Most of the time, we won't be working with raw values. Instead, you can grab a database's index with **xGetPointer()**, which returns the current index that the database is looking at. This is most useful when you're searching through the database for something, such as the closest unit to a point. For example, you can iterate through an entire database and check each unit's distance to a point. Each time you find one that's closer than the last, you can use **xGetPointer()** to store the current index in a variable. Once you've looped through the whole database, you can use **xSetPointer()** to return to the closest unit found.

## Data Load
This is the secret sauce behind Ascension, made in an incredibly easy to use format for anyone. To start, please include the **zshared.c, dataLoadBackend.c,** and **dataLoadModifyMe.c** files in your rmsify.py file list. These three files don't have to be directly contiguous, but make sure they are in this sequential order (You can include other files of your own in between them, as long as zshared.c is before dataLoadBackend.c and dataLoadBackend.c is before dataLoadModifyMe.c).

Next, open **dataLoadModifyMe.c** and follow instructions in the comments to add your desired data.

With this, the game will load the data at the start of the map. In multiplayer, it will go through the whole loading bar. In singleplayer, the data load is instantaneous.

### Saving
To save your data, simply call the **saveAllData()** function, which will save all the values back into the player's profile. This can be called at any time during play, even if the game is still running, although I would recommend only saving at sparse checkpoints or the end of the map.

### Data Types
All the data is stored as integers. Floats and other such things will not work. There is also a distinction between the data:
* **Database Data**: This is data that is unique for each player. For example, each player has their own level in Ascension. This is data that is loaded at the start of the map and communicated to other player machines.
* **Local Data**: This is data that isn't important for multiplayer. For example, only in singleplayer does the map care about your collection of relics, or what levels your unused classes are at. Therefore, this data is NOT transferred to other players when playing the map in multiplayer.

It is VERY IMPORTANT to make sure that Local Data and Database Data are in separate slots. If you have a bit of both in the same slot, it will BREAK your data.
