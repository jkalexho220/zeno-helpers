/*
README

A slot can hold an integer value up to 1 billion. Make sure you're not using up all this space.
To calculate how much space you are using in a slot, multiply together all the various sizes of
data that are stored in that slot.

In Age of Mythology, we have 16 slots to work with, numbered 0-15.

Make sure that there are no units at the start of the map. If you want to do any
map initialization suff, make sure it is done in or after the NEXT_TRIGGER_NAME trigger.
This is to ensure that the unit names are contiguous and starting from 0, which is
important for the data load algorithm.

Also make sure that the bottom corner of the map is passable terrain at the very start. (It can be changed after data loading is done)

Also, using this will modify the cost of swordsman hero to 0 and also make swordsmen flying units. 
Be wary of this side effect. You can modify it back to what you want later.

There are two functions you can use to add data:

////////////////////
// addSavedDataQV //
////////////////////
Use this when you want to save the data in a quest var.
NOTE: This quest var value will be LOCAL to the player, meaning it may have a
different value for each player.

addSavedDataQV(<QV name>, <slot number>, <data range>);

Example: addSavedDataQV("cow", 0, 10);
This will tell the game to keep track of the QV "cow". Its value is limited to the range 0-9 and it is stored in slot 0.

////////////////////
// addSavedDataDB //
////////////////////
Use this when you want to save the data in a database containing player info. This data
will be synced with other players at the start of the map.

NOTE: The database must be large enough to have an entry for each player. When data is
loaded, it will be stored at the database <destDB> at the variable <destVar> at the index <p>,
where p is the player. Also, the database must be declared BEFORE this function is ever
called. You can do so by initializing the database in a highFrequency trigger that is
active at the very start before this trigger is run.

addSavedDataDB(<DB name>, <Var name>, <slot number>, <data range>);

Example: addSavedDataDB(dPlayerData, xPlayerHealth, 3, 10);
This will tell the game to keep track of the xPlayerHealth value for each player in the dPlayerData database. 
Its value ranges from 0-9 and it is stored in slot 3.
*/

rule setup_local_data
inactive
highFrequency
{
	NEXT_TRIGGER_NAME = "did_it_work"; // the next trigger to run after data load is complete
	/*
	Add data to slots here
	
	EXAMPLE
	name | slot | maximum value
	addSavedDataQV("cow", 1, 10);
	addSavedDataDB(dPlayerData, xPlayerGold, 0, 1000);
	*/

	/*
	Slot 0
	Total size: 0
	*/
	addSavedDataQV("example", 0, 13); // the QV "example" can have an integer value from 0-12. It is stored in the first slot

	/*
	Slot 1
	Total size: 1000
	*/
	addSavedDataDB(dPlayerData, xPlayerHealth, 1, 1000);

	/*
	Slot 2
	Total size: 1,000,000
	*/
	addSavedDataDB(dPlayerData, xPlayerGold, 2, 1000);
	addSavedDataDB(dPlayerData, xPlayerFood, 2, 1000);

	/*
	Slot 3
	Total size: 1000
	*/
	addSavedDataDB(dPlayerData, xPlayerWood, 3, 1000);

	/*
	Slot 4
	Total size: 0
	*/

	/*
	Slot 5
	Total size: 0
	*/

	/*
	Slot 6
	Total size: 0
	*/

	/*
	Slot 7
	Total size: 0
	*/

	/*
	Slot 8
	Total size: 0
	*/

	/*
	Slot 9
	Total size: 0
	*/

	/*
	Slot 10
	Total size: 0
	*/

	/*
	Slot 11
	Total size: 0
	*/

	/*
	Slot 12
	Total size: 0
	*/

	/*
	Slot 13
	Total size: 0
	*/

	/*
	Slot 14
	Total size: 0
	*/

	/*
	Slot 15
	Total size: 0
	*/
	xsDisableSelf();
}