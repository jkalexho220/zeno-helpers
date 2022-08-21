/*
README

A slot can hold an integer value up to 1 billion. Make sure you're not using up all this space.
To calculate how much space you are using in a slot, multiply together all the various sizes of
data that are stored in that slot.

Make sure that there are no units at the start of the map. If you want to do any
map initialization suff, make sure it is done after the NEXT_TRIGGER_NAME trigger.
This is to ensure that the unit names are contiguous and starting from 0, which is
important for the data load algorithm.

Also make sure that the bottom corner of the map is passable terrain.

Also, using this will modify the cost of swordsman hero to 0. Be wary of this side effect.
You can modify it back to what you want later.

You should scroll down and modify ONLY the following:
- NEXT_TRIGGER_NAME (this is the name of the trigger that activates after data load is done)
- rule setup_local_data (this is where you define the data that shall be saved/loaded)
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
	addSavedDataQV("example", 0, 12); // the QV "example" can have an integer value from 0-9. It is stored in the first slot

	/*
	Slot 1
	Total size: 0
	*/
	addSavedDataDB(dPlayerData, xPlayerHealth, 1, 1000);

	/*
	Slot 2
	Total size: 0
	*/
	addSavedDataDB(dPlayerData, xPlayerGold, 2, 1000);
	addSavedDataDB(dPlayerData, xPlayerFood, 2, 1000);

	/*
	Slot 3
	Total size: 0
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