rule lure
active
highFrequency
{
	for(p = 1; <= 2){
		if (trPlayerUnitCountSpecific(p, "Animal Attractor") == 1) {
			yFindLatestReverse("p"+p+"LureObject", "Animal Attractor", p);
			trVectorSetUnitPos("V1", "p"+p+"LureObject", true);
			trMutateSelected(kbGetProtoUnitID("Osiris SFX"));
			trArmyDispatch(""+p+",0", "Ajax", 1, trVectorQuestVarGetX("V1"), trVectorQuestVarGetY("V1"),
			trVectorQuestVarGetZ("V1"), 0, true);
			trVectorQuestVarEcho("V1");
		}
		else {
			trQuestVarSet("p"+p+"LureObject", trGetNextUnitScenarioNameNumber() - 1);
		}
	}
}