int dPlayerData = 0;
int xPlayerHealth = 0;
int xPlayerFood = 0;
int xPlayerWood = 0;
int xPlayerGold = 0;

rule did_it_work
inactive
highFrequency
{
	xsDisableSelf();
	trUIFadeToColor(0,0,0,1000,1000,false);
	trLetterBox(false);
	for(i=xGetDatabaseCount(dPlayerData); >0) {
		xDatabaseNext(dPlayerData);
		xPrintAll(dPlayerData, xGetPointer(dPlayerData));
	}

	trChatSend(0, "Example: " + trQuestVarGet("example"));

	trQuestVarSet("end", trGetNextUnitScenarioNameNumber());
	trArmyDispatch("1,0","Pegasus",1,1,0,1,0,true);

	trArmyDispatch("0,0", "Gold Mine", 1, 11, 0, 15, 0, true);
	trArmyDispatch("0,0", "Berry Bush", 1, 15, 0, 15, 0, true);
	trArmyDispatch("0,0", "Palm", 1, 19, 0, 15, 0, true);

	trSetFogAndBlackmap(false, false);

	for(p=1; < cNumberPlayers) {
		trModifyProtounit("Villager Atlantean", p, 0, 840);
		trPlayerGrantResources(p, "food", xGetInt(dPlayerData, xPlayerFood, p) - trPlayerResourceCount(p, "food"));
		trPlayerGrantResources(p, "wood", xGetInt(dPlayerData, xPlayerWood, p) - trPlayerResourceCount(p, "wood"));
		trPlayerGrantResources(p, "gold", xGetInt(dPlayerData, xPlayerGold, p) - trPlayerResourceCount(p, "gold"));
		trQuestVarSet("p"+p+"villager", trGetNextUnitScenarioNameNumber());
		trArmyDispatch(""+p+",0","Villager Atlantean",1, 5 + 2 * p, 0, 3, 0, true);
		float health = xGetInt(dPlayerData, xPlayerHealth, p);
		if (health <= 0) {
			health = 999;
			xSetInt(dPlayerData, xPlayerHealth, p);
		}
		trArmySelect(""+p+",0");
		xsSetContextPlayer(p);
		trDamageUnit(kbUnitGetCurrentHitpoints(kbGetBlockID(""+1*trQuestVarGet("p"+p+"villager"))) - health);
		xsSetContextPlayer(0);
	}

	xsEnableRule("end_it_all");
}

rule end_it_all
inactive
highFrequency
{
	trUnitSelectClear();
	trUnitSelectByQV("end");
	if (trUnitAlive() == false) {
		int p = trCurrentPlayer();
		xSetPointer(dPlayerData, p);
		xSetInt(dPlayerData, xPlayerGold, trPlayerResourceCount(p, "gold"));
		xSetInt(dPlayerData, xPlayerWood, trPlayerResourceCount(p, "wood"));
		xSetInt(dPlayerData, xPlayerFood, trPlayerResourceCount(p, "food"));
		xsSetContextPlayer(p);
		float health = kbUnitGetCurrentHitpoints(kbGetBlockID(""+1*trQuestVarGet("p"+p+"villager")));
		xsSetContextPlayer(0);
		xSetInt(dPlayerData, xPlayerHealth, health);
		xPrintAll(dPlayerData, p);
		saveAllData();
		trSoundPlayFN("favordump.wav","1",-1,"","");
		xsDisableSelf();
	} else if (trUnitIsSelected() && (trTime() > trQuestVarGet("upNext"))) {
		trQuestVarSet("upNext", trTime());
		trQuestVarSet("example", 1 + trQuestVarGet("example"));
		trChatSend(0, "Example: " + trQuestVarGet("example"));
	}
}

rule init_db
active
highFrequency
{
	xsDisableSelf();
	int playerCount = cNumberPlayers - 1;

	dPlayerData = xInitDatabase("playerData", playerCount);
	xPlayerHealth = xInitAddInt(dPlayerData, "health");
	xPlayerFood = xInitAddInt(dPlayerData, "food");
	xPlayerWood = xInitAddInt(dPlayerData, "wood");
	xPlayerGold = xInitAddInt(dPlayerData, "gold");

	for(p=0; < playerCount) {
		xAddDatabaseBlock(dPlayerData);
	}
}