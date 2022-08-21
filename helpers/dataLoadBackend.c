int dLocalData = 0;
int xLocalDataSize = 0;
int xLocalDataSlot = 0;
int xLocalDataName = 0;
int xLocalDataDB = 0;
int xLocalDataVar = 0;

int loadNumHumans = 0;

int localDataArray = 0; // an array to temporarily store the slot data in memory
int swordsmanCountArray = 0; // how many swordsmen passes are needed here?
int playerDataArray = 0;

int loadProgress = 0;
int totalLoad = 0;

string NEXT_TRIGGER_NAME = "";


// keep track of the amount of space used in this slot
void recordTotalSize(int slot = 0, int size = 0) {
	if (zGetInt(swordsmanCountArray, slot) == 0) {
		zSetInt(swordsmanCountArray, slot, size);
	} else {
		zSetInt(swordsmanCountArray, slot, size * zGetInt(swordsmanCountArray, slot));
	}
}


void addSavedDataQV(string qvName = "", int slot = 0, int size = 1) {
	xAddDatabaseBlock(dLocalData, true);
	xSetInt(dLocalData, xLocalDataSize, size);
	xSetInt(dLocalData, xLocalDataSlot, slot);

	xSetString(dLocalData, xLocalDataName, qvName);

	recordTotalSize(slot, size);
}

void addSavedDataDB(int destDB = 0, int destVar = 0, int slot = 0, int size = 1) {
	xAddDatabaseBlock(dLocalData, true);
	xSetInt(dLocalData, xLocalDataSize, size);
	xSetInt(dLocalData, xLocalDataSlot, slot);

	xSetInt(dLocalData, xLocalDataDB, destDB);
	xSetInt(dLocalData, xLocalDataVar, destVar);

	recordTotalSize(slot, size);
}

void showLoadProgress() {
	trSoundPlayFN("default","1",-1,"Loading Data:"+100 * loadProgress / totalLoad,"icons\god power reverse time icons 64");
}

void saveAllData() {
	xsSetContextPlayer(0);
	int currentdata = 0;
	int slot = 0;
	int p = trCurrentPlayer();

	// save the data
	for(i=0; < 16) {
		// make all data 0 in preparation for updates
		zSetInt(localDataArray, i, 0);
	}
	// read data from the various quest vars in backwards order
	xSetPointer(dLocalData, 1);
	for(i=xGetDatabaseCount(dLocalData); >0) {
		xDatabaseNext(dLocalData, true); // database search is backwards this time
		// read the data in the quest var
		slot = xGetInt(dLocalData, xLocalDataSlot);
		if (xGetInt(dLocalData, xLocalDataDB) == 0) {
			currentdata = trQuestVarGet(xGetString(dLocalData, xLocalDataName));
		} else {
			currentdata = xGetInt(xGetInt(dLocalData, xLocalDataDB), xGetInt(dLocalData, xLocalDataVar), p);
		}
		
		// floor and ceiling the data so it fits in the data range
		currentdata = xsMax(0, currentdata);
		currentdata = xsMin(currentdata, xGetInt(dLocalData, xLocalDataSize) - 1);
		
		// shift the slot data over and insert our data
		zSetInt(localDataArray, slot, zGetInt(localDataArray, slot) * xGetInt(dLocalData, xLocalDataSize) + currentdata);
	}
	// save all the data into the slots
	for(i=0; < 16) {
		trSetCurrentScenarioUserData(i, zGetInt(localDataArray, i));
	}
}

void loadRawData() {
	xsSetContextPlayer(0);
	for(i=0; < 16) {
		// load all the raw data into the array
		zSetInt(localDataArray, i, trGetScenarioUserData(i));
	}
}

void loadDataFromArray(int arrNum = 0, int p = 1) {
	int slot = xGetInt(dLocalData, xLocalDataSlot);
	int currentdata = zGetInt(arrNum, slot); 
	int val = iModulo(xGetInt(dLocalData, xLocalDataSize), currentdata);
	zSetInt(arrNum, slot, currentdata / xGetInt(dLocalData, xLocalDataSize));

	if (xGetInt(dLocalData, xLocalDataDB) == 0) {
		// if it's a qv
		if (trCurrentPlayer() == p) {
			trQuestVarSet(xGetString(dLocalData, xLocalDataName), val);
		}
	} else {
		// if it's a db entry
		xSetInt(xGetInt(dLocalData, xLocalDataDB), xGetInt(dLocalData, xLocalDataVar), val, p);
	}
}

// Reads data into the local array
void loadAllDataMultiplayer() {
	xsSetContextPlayer(0);

	xSetPointer(dLocalData, 1);
	// turn all the data into vars by traversing forwards
	for(i=xGetDatabaseCount(dLocalData); >0) {
		// read the data segment
		for(p=1; < cNumberPlayers) {
			loadDataFromArray(zGetInt(playerDataArray, p), p);
		}
		xDatabaseNext(dLocalData);
	}
}

// Reads data into the local array
void loadAllDataSingleplayer() {
	xsSetContextPlayer(0);

	xSetPointer(dLocalData, 1);
	// turn all the data into vars by traversing forwards
	for(i=xGetDatabaseCount(dLocalData); >0) {
		// read the data segment
		loadDataFromArray(localDataArray);
		xDatabaseNext(dLocalData);
	}
}


rule data_load_00
active
highFrequency
{
	xsDisableSelf();
	xsEnableRule("setup_local_data");
	trDelayedRuleActivation("data_load_01");

	trLetterBox(true);
	trUIFadeToColor(0,0,0,0,0,true);

	/*
	The dLocalData database contains every requested piece of data. This is how
	the data will be automatically loaded for us.
	*/
	dLocalData = xInitDatabase("localDataSegments");
	xLocalDataSize = xInitAddInt(dLocalData, "size");
	xLocalDataSlot = xInitAddInt(dLocalData, "slot");
	xLocalDataName = xInitAddString(dLocalData, "name");
	xLocalDataDB = xInitAddInt(dLocalData, "database");
	xLocalDataVar = xInitAddInt(dLocalData, "variable");

	localDataArray = zNewArray(mInt, 16, "localData"); // data for the local player

	if (aiIsMultiplayer()) {
		playerDataArray = zNewArray(mInt, cNumberPlayers, "playerData"); // a 3-dimensional array holding player data
		swordsmanCountArray = zNewArray(mInt, 16, "swordsmanCount");
		for(i=0; < 16) {
			zSetInt(swordsmanCountArray, i, 0);
		}
		for(p=1; < cNumberPlayers) {
			zSetInt(playerDataArray, p, zNewArray(mInt, 16, "p"+p+"data"));

			trModifyProtounit("Swordsman", p, 55, 4);
			trModifyProtounit("Swordsman", p, 2, 9999999999999999999.0);
			trModifyProtounit("Swordsman", p, 2, -9999999999999999999.0);
			trModifyProtounit("Swordsman", p, 2, 0);
			trModifyProtounit("Swordsman Hero", p, 2, 9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 2, -9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 2, 0);
			trModifyProtounit("Swordsman Hero", p, 6, -100);
			trModifyProtounit("Swordsman Hero", p, 16, 9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 17, 9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 18, 9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 19, 9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 16, -9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 17, -9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 18, -9999999999999999999.0);
			trModifyProtounit("Swordsman Hero", p, 19, -9999999999999999999.0);
			trArmyDispatch(""+p+",0","Swordsman", 32, 1 + 2 * p,0,1,0,true);

			if (kbIsPlayerHuman(p)) {
				loadNumHumans = 1 + loadNumHumans;
			}
		}

		for(p=1; < cNumberPlayers) {
			trArmyDispatch(""+p+",0","Victory Marker", 1, 1, 0, 1, 0, true);
		}
	}
}

int currentSwordsmanSlot = 0;
int currentSwordsmanMultiplier = 1;
int currentSwordsmanData = 0;

rule data_load_01
inactive
highFrequency
{
	xsDisableSelf();
	loadRawData();
	if (aiIsMultiplayer()) {
		xsEnableRule("data_load_02_send_data");
		xsEnableRule("data_load_03_receive_data");
		for(i=0; < 16) {
			// how many swordsmen are needed to transfer the data?
			int swordsmen = 0;
			for(j=0; < 6) {
				if (zGetInt(swordsmanCountArray, i) > 0) {
					zSetInt(swordsmanCountArray, i, zGetInt(swordsmanCountArray, i) / 32);
					swordsmen = swordsmen + 1;
				} else {
					break;
				}
			}
			zSetInt(swordsmanCountArray, i, swordsmen);
			totalLoad = totalLoad + swordsmen;

			for(p=0; < cNumberPlayers) {
				zSetInt(zGetInt(playerDataArray, p), i, 0);
			}
		}
	} else {
		loadAllDataSingleplayer();
		xsEnableRule(NEXT_TRIGGER_NAME);
	}
}



rule data_load_02_send_data
inactive
highFrequency
{
	for(i=currentSwordsmanSlot; < 16) {
		if (zGetInt(swordsmanCountArray, i) > 0) {
			zSetInt(swordsmanCountArray, i, zGetInt(swordsmanCountArray, i) - 1);
			currentSwordsmanData = iModulo(32, zGetInt(localDataArray, i));
			zSetInt(localDataArray, i, zGetInt(localDataArray, i) / 32);

			currentSwordsmanSlot = i;
			break;
		} else {
			currentSwordsmanMultiplier = 1;
		}
	}

	trLetterBox(false);
	trBlockAllSounds(true);
	trUnitSelectClear();
	trUnitSelectByID(currentSwordsmanData + 32 * (trCurrentPlayer() - 1));
	for(i=32; >0) {
		if (trUnitIsSelected() == false) {
			uiFindType("Swordsman");
		} else {
			break;
		}
	}
	uiTransformSelectedUnit("Swordsman Hero");
	trForceNonCinematicModels(true);
	
	trLetterBox(true);
	
	showLoadProgress();
	xsDisableSelf();
}

rule data_load_03_receive_data
inactive
highFrequency
{
	int swordsmen = 0;
	int currentdata = 0;
	for(p=1; < cNumberPlayers) {
		swordsmen = swordsmen + trPlayerUnitCountSpecific(p, "Swordsman Hero");
	}
	if (swordsmen == loadNumHumans) {
		for(p=1; < cNumberPlayers) {
			swordsmen = 32 * (p - 1);
			for(x=0; < 32) {
				if (kbGetUnitBaseTypeID(x + swordsmen) == kbGetProtoUnitID("Swordsman Hero")) {
					/* read the data */
					currentdata = zGetInt(zGetInt(playerDataArray, p), currentSwordsmanSlot) + currentSwordsmanMultiplier * x;

					zSetInt(zGetInt(playerDataArray, p), currentSwordsmanSlot, currentdata);
					
					trUnitSelectClear();
					trUnitSelectByID(x + swordsmen);
					trMutateSelected(kbGetProtoUnitID("Swordsman"));
					break;
				}
			}
		}

		loadProgress = loadProgress + 1;
		currentSwordsmanMultiplier = currentSwordsmanMultiplier * 32;
		if (loadProgress == totalLoad) {
			xsEnableRule("data_load_04_parse_data");
			xsDisableSelf();
		} else {
			xsEnableRule("data_load_02_send_data");
		}
		showLoadProgress();
	}
}

rule data_load_04_parse_data
inactive
highFrequency
{
	xsDisableSelf();
	int slot = 0;
	int currentdata = 0;

	for(i=0; < 32 * (cNumberPlayers - 1)) {
		trUnitSelectClear();
		trUnitSelectByID(i);
		trUnitDestroy();
	}

	loadAllDataMultiplayer();

	trSoundPlayFN("favordump.wav","1",-1,"","");

	xsEnableRule(NEXT_TRIGGER_NAME);
}