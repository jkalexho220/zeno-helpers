const float PI = 3.141592;

/*
NOTE: In order for the database functionality to work, any database functions can only be called
while the context player is 0. If you need to switch context players (such as for getting a unit's current health or current attack target)
make sure to immediately switch back to context player 0 once you are done acquiring the information you need.

In addition, database functions do not work at the start of the game before AI have been initialized. Don't use them in any runImmediately triggers
*/

rule context_change_always
active
highFrequency
{
	/*
	For whatever reason, the context player is set to -1 at the start of
	every trigger loop, but only in random map scripts. So here we are.
	*/
	xsSetContextPlayer(0);
}

const int mInt = 0;
const int mFloat = 1;
const int mString = 2;
const int mVector = 3;
const int mBool = 4;

const int xMetadata = 0; // contains current pointer and database size and numVariables. However, malloc doesn't have metadata so they have to shift down 1
const int xDirtyBit = 1;
const int xNextBlock = 2;
const int xPrevBlock = 3; // for databases, xData is unused and xPrevBlock takes its place
const int xData = 3;
const int xVarNames = 4; // list of variable names
const int xVariables = 5;

/*
Metadata information
*/
const int mPointer = 0;
const int mCount = 1;
const int mNextFree = 2;
const int mNewestBlock = 3;
const int mCacheHead = 4; // the cache stores items that you want to temporarily remove
const int mCacheCount = 5;
const int mVariableTypes = 5;
/*
subsequent items in the metadata will determine the datatypes of extra variables for the database
*/

const int NEXTFREE = 0; // the very first block contains the next free pointer and nothing else

int MALLOC = 0;
int ARRAYS = 0;
int mNumArrays = 0;

void debugLog(string msg = "") {
	if (trCurrentPlayer() == 1) {
		trChatSend(0, "<color=1,0,0>" + msg);
	}
}


string datatypeName(int data = 0) {
	string name = "void";
	if (data >= 0 && data <= 4) {
		name = aiPlanGetUserVariableString(MALLOC,15,data);
	}
	return(name);
}

int zNewArray(int type = 0, int size = 1, string name = "") {
	int index = mNumArrays;
	mNumArrays = mNumArrays + 1;
	switch(type)
	{
		case mInt:
		{
			aiPlanAddUserVariableInt(ARRAYS,index,name,size);
		}
		case mFloat:
		{
			aiPlanAddUserVariableFloat(ARRAYS,index,name,size);
		}
		case mString:
		{
			aiPlanAddUserVariableString(ARRAYS,index,name,size);
		}
		case mVector:
		{
			aiPlanAddUserVariableVector(ARRAYS,index,name,size);
		}
		case mBool:
		{
			aiPlanAddUserVariableBool(ARRAYS,index,name,size);
		}
	}
	return(index);
}

/*
*/
bool free(int type = -1, int index = -1) {
	bool success = false;
	if (aiPlanGetUserVariableBool(MALLOC, type * 3 + xDirtyBit - 1, index)) {
		aiPlanSetUserVariableInt(MALLOC, type * 3 + xNextBlock - 1, index,
			aiPlanGetUserVariableInt(MALLOC, type * 3 + xNextBlock - 1, NEXTFREE));
		aiPlanSetUserVariableBool(MALLOC, type * 3 + xDirtyBit - 1, index, false);
		aiPlanSetUserVariableInt(MALLOC, type * 3 + xNextBlock - 1, NEXTFREE, index); // set next free to be the newly added block
		success = true;
	}
	return(success);
}

/*
*/
int malloc(int type = -1) {
	/*
	get next free User variable
	*/
	int next = aiPlanGetUserVariableInt(MALLOC, type * 3 + xNextBlock - 1, NEXTFREE);
	if (next == 0) {
		/*
		if no free buffers, create a new one
		*/
		next = aiPlanGetNumberUserVariableValues(MALLOC,type * 3 + xNextBlock - 1);
		for(i=type * 3; < type * 3 + 3) {
			aiPlanSetNumberUserVariableValues(MALLOC,i,next + 1, false); // resizing seems to be a little expensive?
		}
		aiPlanSetUserVariableInt(MALLOC,type * 3 + xNextBlock - 1, next, 0); // next free block is 0 for a newly created block
	} else {
		/*
		if a free buffer exists, we set our nextFree pointer to the next free buffer
		*/
		aiPlanSetUserVariableInt(MALLOC,type * 3 + xNextBlock - 1,NEXTFREE,
			aiPlanGetUserVariableInt(MALLOC,type * 3 + xNextBlock - 1,next));
	}
	aiPlanSetUserVariableBool(MALLOC,type * 3 + xDirtyBit - 1, next, true); // set dirty bit
	
	return(next);
}

bool mGetBool(int index = 0) {
	bool val = false;
	if (aiPlanGetUserVariableBool(MALLOC, mBool * 3 + xDirtyBit - 1, index)) {
		val = aiPlanGetUserVariableBool(MALLOC, mBool * 3 + xData - 1, index);
	}
	return(val);
}

bool mSetBool(int index = 0, bool val = false) {
	bool success = false;
	if (aiPlanGetUserVariableBool(MALLOC, mBool * 3 + xDirtyBit - 1, index)) {
		success = aiPlanSetUserVariableBool(MALLOC, mBool * 3 + xData - 1, index, val);
	}
	return(success);
}

int mNewBool(bool val = false) {
	int index = malloc(mBool);
	mSetBool(index, val);
	return(index);
}

bool mFreeBool(int index = 0) {
	return(free(mBool, index));
}


string mGetString(int index = 0) {
	string val = "";
	if (aiPlanGetUserVariableBool(MALLOC, mString * 3 + xDirtyBit - 1, index)) {
		val = aiPlanGetUserVariableString(MALLOC, mString * 3 + xData - 1, index);
	}
	return(val);
}

bool mSetString(int index = 0, string val = "") {
	bool success = false;
	if (aiPlanGetUserVariableBool(MALLOC, mString * 3 + xDirtyBit - 1, index)) {
		success = aiPlanSetUserVariableString(MALLOC, mString * 3 + xData - 1, index, val);
	}
	return(success);
}

int mNewString(string val = "") {
	int index = malloc(mString);
	mSetString(index, val);
	return(index);
}

bool mFreeString(int index = 0) {
	return(free(mString, index));
}

int mGetInt(int index = 0) {
	int val = -1;
	if (aiPlanGetUserVariableBool(MALLOC, mInt * 3 + xDirtyBit - 1, index)) {
		val = aiPlanGetUserVariableInt(MALLOC, mInt * 3 + xData - 1, index);
	}
	return(val);
}

bool mSetInt(int index = 0, int val = 0) {
	bool success = false;
	if (aiPlanGetUserVariableBool(MALLOC, mInt * 3 + xDirtyBit - 1, index)) {
		success = aiPlanSetUserVariableInt(MALLOC, mInt * 3 + xData - 1, index, val);
	}
	return(success);
}

int mNewInt(int val = 0) {
	int index = malloc(mInt);
	mSetInt(index, val);
	return(index);
}

bool mFreeInt(int index = 0) {
	return(free(mInt, index));
}

float mGetFloat(int index = 0) {
	float val = -1;
	if (aiPlanGetUserVariableBool(MALLOC, mFloat * 3 + xDirtyBit - 1, index)) {
		val = aiPlanGetUserVariableFloat(MALLOC, mFloat * 3 + xData - 1, index);
	}
	return(val);
}

bool mSetFloat(int index = 0, float val = 0) {
	bool success = false;
	if (aiPlanGetUserVariableBool(MALLOC, mFloat * 3 + xDirtyBit - 1, index)) {
		success = aiPlanSetUserVariableFloat(MALLOC, mFloat * 3 + xData - 1, index, val);
	}
	return(success);
}

int mNewFloat(float val = 0) {
	int index = malloc(mFloat);
	mSetFloat(index, val);
	return(index);
}

bool mFreeFloat(int index = 0) {
	return(free(mFloat, index));
}

vector mGetVector(int index = 0) {
	vector val = vector(-1,-1,-1);
	if (aiPlanGetUserVariableBool(MALLOC, mVector * 3 + xDirtyBit - 1, index)) {
		val = aiPlanGetUserVariableVector(MALLOC, mVector * 3 + xData - 1, index);
	}
	return(val);
}

bool mSetVector(int index = 0, vector val = vector(0,0,0)) {
	bool success = false;
	if (aiPlanGetUserVariableBool(MALLOC, mVector * 3 + xDirtyBit - 1, index)) {
		success = aiPlanSetUserVariableVector(MALLOC, mVector * 3 + xData - 1, index, val);
	}
	return(success);
}

int mNewVector(vector val = vector(0,0,0)) {
	int index = malloc(mVector);
	mSetVector(index, val);
	return(index);
}

bool mFreeVector(int index = 0) {
	return(free(mVector, index));
}

/*
Size is the starting size of the database, but databases can grow indefinitely
returns the identifier of the database. Use this identifier in other xDatabase triggers
*/
int xInitDatabase(string name = "", int size = 0) {
	int id = aiPlanCreate(name, 8);
	aiPlanAddUserVariableBool(id,xDirtyBit,"DirtyBit",size+1);
	aiPlanAddUserVariableInt(id,xNextBlock,"NextBlock",size+1);
	aiPlanAddUserVariableInt(id,xPrevBlock,"PrevBlock",size+1);
	aiPlanAddUserVariableInt(id,xMetadata,"Metadata",6);
	aiPlanSetUserVariableInt(id,xMetadata,mPointer,0);
	aiPlanSetUserVariableInt(id,xMetadata,mCount,0);
	aiPlanSetUserVariableInt(id,xMetadata,mCacheHead,0);
	aiPlanSetUserVariableInt(id,xMetadata,mCacheCount,0);
	
	aiPlanSetUserVariableInt(id,xMetadata,mNextFree,size);
	aiPlanSetUserVariableInt(id,xNextBlock,0,0);
	for(i=1; <= size) { // connect all the free buffers together
		aiPlanSetUserVariableBool(id,xDirtyBit,i,false);
		aiPlanSetUserVariableInt(id,xNextBlock,i,i-1);
	}
	aiPlanAddUserVariableString(id,xVarNames,"VarNames",1);
	aiPlanSetUserVariableString(id,xVarNames,0,"none");
	return(id);
}

/*
returns the index of the newly added variable
*/
int xInitAddVar(int id = 0, string name = "", int type = 0) {
	int count = aiPlanGetNumberUserVariableValues(id,xDirtyBit);
	/*
	first, add the type to our list of types in this struct
	*/
	int index = aiPlanGetNumberUserVariableValues(id,xMetadata);
	aiPlanSetNumberUserVariableValues(id,xMetadata,index + 1,false);
	aiPlanSetUserVariableInt(id,xMetadata,index,type);
	
	index = aiPlanGetNumberUserVariableValues(id,xVarNames);
	aiPlanSetNumberUserVariableValues(id,xVarNames,index+1,false);
	aiPlanSetUserVariableString(id,xVarNames,index,name);
	/*
	next, add a new array of the specified datatype to hold values
	*/
	index = xVarNames + index;
	switch(type)
	{
		case mInt:
		{
			aiPlanAddUserVariableInt(id,index,name,count);
		}
		case mFloat:
		{
			aiPlanAddUserVariableFloat(id,index,name,count);
		}
		case mString:
		{
			aiPlanAddUserVariableString(id,index,name,count);
		}
		case mVector:
		{
			aiPlanAddUserVariableVector(id,index,name,count);
		}
		case mBool:
		{
			aiPlanAddUserVariableBool(id,index,name,count);
		}
	}
	return(index);
}

/*
id is the integer name of the database
name is unused but required
defVal = default value for newly created database entries
*/
int xInitAddInt(int id = 0, string name = "", int defVal = 0) {
	int index = xInitAddVar(id,name,mInt);
	aiPlanSetUserVariableInt(id, index, 0, defVal);
	return(index);
}

int xInitAddFloat(int id = 0, string name = "", float defVal = 0) {
	int index = xInitAddVar(id, name, mFloat);
	aiPlanSetUserVariableFloat(id, index, 0, defVal);
	return(index);
}

int xInitAddString(int id = 0, string name = "", string defVal = "") {
	int index = xInitAddVar(id, name, mString);
	aiPlanSetUserVariableString(id, index, 0, defVal);
	return(index);
}

int xInitAddVector(int id = 0, string name = "", vector defVal = vector(0,0,0)) {
	int index = xInitAddVar(id, name, mVector);
	aiPlanSetUserVariableVector(id, index, 0, defVal);
	return(index);
}

int xInitAddBool(int id = 0, string name = "", bool defVal = false) {
	int index = xInitAddVar(id,name,mBool);
	aiPlanSetUserVariableBool(id, index, 0, defVal);
	return(index);
}

void xResetValues(int id = 0, int index = -1, int stopAt = -1) {
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	if (stopAt == -1) {
		stopAt = aiPlanGetNumberUserVariableValues(id, xVarNames);
	} else {
		stopAt = stopAt - mVariableTypes;
	}
	for(i = 1; < stopAt) {
		switch(aiPlanGetUserVariableInt(id,xMetadata,mVariableTypes + i))
		{
			case mInt:
			{
				aiPlanSetUserVariableInt(id,xVarNames + i,index,aiPlanGetUserVariableInt(id,xVarNames + i,0));
			}
			case mFloat:
			{
				aiPlanSetUserVariableFloat(id,xVarNames + i,index,aiPlanGetUserVariableFloat(id,xVarNames + i,0));
			}
			case mString:
			{
				aiPlanSetUserVariableString(id,xVarNames + i,index,aiPlanGetUserVariableString(id,xVarNames + i,0));
			}
			case mVector:
			{
				aiPlanSetUserVariableVector(id,xVarNames + i,index,aiPlanGetUserVariableVector(id,xVarNames + i,0));
			}
			case mBool:
			{
				aiPlanSetUserVariableBool(id,xVarNames + i,index,aiPlanGetUserVariableBool(id,xVarNames + i,0));
			}
		}
	}
}

bool xSetPointer(int id = 0, int index = 0) {
	bool success = false;
	if (aiPlanGetUserVariableBool(id,xDirtyBit,index)) {
		aiPlanSetUserVariableInt(id,xMetadata,mPointer,index);
		success = true;
	}
	return(success);
}

int xAddDatabaseBlock(int id = 0, bool setPointer = false) {
	int next = aiPlanGetUserVariableInt(id,xMetadata,mNextFree);
	if (next == 0) {
		/*
		if no available buffers, we extend the total sizes of the arrays
		*/
		next = aiPlanGetNumberUserVariableValues(id,xDirtyBit);
		/* increase lengths of variable arrays */
		for(i=aiPlanGetNumberUserVariableValues(id,xVarNames) - 1; > 0) {
			aiPlanSetNumberUserVariableValues(id,i + xVarNames,next+1,false);
		}
		/* increase lengths of metadata arrays */
		for(i=xPrevBlock; > xMetadata) {
			aiPlanSetNumberUserVariableValues(id,i,next+1,false);
		}
	} else {
		/*
		if a free buffer is available, we grab it and set next free to be the next of the buffer we grabbed
		*/
		aiPlanSetUserVariableInt(id,xMetadata,mNextFree,aiPlanGetUserVariableInt(id,xNextBlock,next));
	}
	aiPlanSetUserVariableBool(id,xDirtyBit,next,true);
	
	if (aiPlanGetUserVariableInt(id,xMetadata,mCount) == 0) {
		/*
		If it's the only thing in the db, point it to itself and also set the database pointer to the new thing
		*/
		aiPlanSetUserVariableInt(id,xNextBlock,next,next);
		aiPlanSetUserVariableInt(id,xPrevBlock,next,next);
		aiPlanSetUserVariableInt(id,xMetadata,mPointer,next);
	} else {
		/*
		otherwise, slide in between two links in the list at mPointer
		*/
		int before = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
		int after = aiPlanGetUserVariableInt(id,xNextBlock,before);
		
		aiPlanSetUserVariableInt(id,xNextBlock,next,after); // next of me is after
		aiPlanSetUserVariableInt(id,xPrevBlock,next,before); // prev of me is before
		aiPlanSetUserVariableInt(id,xNextBlock,before,next); // next of before is me
		aiPlanSetUserVariableInt(id,xPrevBlock,after,next); // prev of after is me
	}
	aiPlanSetUserVariableInt(id,xMetadata,mNewestBlock,next);
	aiPlanSetUserVariableInt(id,xMetadata,mCount, 1 + aiPlanGetUserVariableInt(id,xMetadata,mCount));
	/*
	finally, initialize all the variables of the struct to their default values (whatever's in index 0 of the array)
	*/
	xResetValues(id,next);
	if (setPointer) {
		xSetPointer(id, next);
	}
	return(next);
}


bool xFreeDatabaseBlock(int id = 0, int index = -1) {
	bool success = false;
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	if (aiPlanGetUserVariableBool(id,xDirtyBit,index)) {
		/* connect next with prev */
		int after = aiPlanGetUserVariableInt(id,xNextBlock,index);
		int before = aiPlanGetUserVariableInt(id,xPrevBlock,index);
		aiPlanSetUserVariableInt(id,xNextBlock,before,after); // next block of before is after
		aiPlanSetUserVariableInt(id,xPrevBlock,after,before); // prev block of after is before
		
		/* add myself to the top of the free stack */
		aiPlanSetUserVariableInt(id,xNextBlock,index,aiPlanGetUserVariableInt(id,xMetadata,mNextFree));
		aiPlanSetUserVariableInt(id,xMetadata,mNextFree,index);
		aiPlanSetUserVariableBool(id,xDirtyBit,index,false);
		
		/* set mPointer to my previous block and decrement count */
		if (index == aiPlanGetUserVariableInt(id,xMetadata,mPointer)) {
			aiPlanSetUserVariableInt(id,xMetadata,mPointer,aiPlanGetUserVariableInt(id,xPrevBlock,index));
		}
		aiPlanSetUserVariableInt(id,xMetadata,mCount, aiPlanGetUserVariableInt(id,xMetadata,mCount) - 1);
		success = true;
	}
	return(success);
}

// Detaches the block and saves it in the cache.
bool xDetachDatabaseBlock(int id = 0, int index = -1) {
	bool success = false;
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	if (aiPlanGetUserVariableBool(id,xDirtyBit,index)) {
		/* connect next with prev */
		int after = aiPlanGetUserVariableInt(id,xNextBlock,index);
		int before = aiPlanGetUserVariableInt(id,xPrevBlock,index);
		aiPlanSetUserVariableInt(id,xNextBlock,before,after); // next block of before is after
		aiPlanSetUserVariableInt(id,xPrevBlock,after,before); // prev block of after is before
		
		aiPlanSetUserVariableBool(id,xDirtyBit,index,false);
		
		/* set mPointer to my previous block and decrement count */
		if (index == aiPlanGetUserVariableInt(id,xMetadata,mPointer)) {
			aiPlanSetUserVariableInt(id,xMetadata,mPointer,before);
		}
		
		/* insert myself into the detach cache */
		if (aiPlanGetUserVariableInt(id,xMetadata,mCacheCount) == 0) {
			/*
			If it's the only thing in the db, point it to itself and also set the database pointer to the new thing
			*/
			aiPlanSetUserVariableInt(id,xNextBlock,index,index);
			aiPlanSetUserVariableInt(id,xPrevBlock,index,index);
			aiPlanSetUserVariableInt(id,xMetadata,mCacheHead,index);
		} else {
			/*
			otherwise, slide in between two links in the list at mCacheHead
			*/
			before = aiPlanGetUserVariableInt(id,xMetadata,mCacheHead);
			after = aiPlanGetUserVariableInt(id,xNextBlock,before);
			
			aiPlanSetUserVariableInt(id,xNextBlock,index,after); // next of me is after
			aiPlanSetUserVariableInt(id,xPrevBlock,index,before); // prev of me is before
			aiPlanSetUserVariableInt(id,xNextBlock,before,index); // next of before is me
			aiPlanSetUserVariableInt(id,xPrevBlock,after,index); // prev of after is me
		}
		
		aiPlanSetUserVariableInt(id,xMetadata,mCount, aiPlanGetUserVariableInt(id,xMetadata,mCount) - 1);
		aiPlanSetUserVariableInt(id,xMetadata,mCacheCount, aiPlanGetUserVariableInt(id,xMetadata,mCacheCount) + 1);
		success = true;
	}
	return(success);
}

bool xRestoreDatabaseBlock(int id = 0, int index = -1) {
	bool success = false;
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mCacheHead);
	}
	if (aiPlanGetUserVariableBool(id,xDirtyBit,index) == false) {
		/* connect next with prev */
		int after = aiPlanGetUserVariableInt(id,xNextBlock,index);
		int before = aiPlanGetUserVariableInt(id,xPrevBlock,index);
		aiPlanSetUserVariableInt(id,xNextBlock,before,after); // next block of before is after
		aiPlanSetUserVariableInt(id,xPrevBlock,after,before); // prev block of after is before
		
		aiPlanSetUserVariableBool(id,xDirtyBit,index,true);
		
		/* set mCacheHead to my previous block and decrement count */
		if (index == aiPlanGetUserVariableInt(id,xMetadata,mCacheHead)) {
			aiPlanSetUserVariableInt(id,xMetadata,mCacheHead,aiPlanGetUserVariableInt(id,xPrevBlock,index));
		}
		
		/* insert myself into the database */
		if (aiPlanGetUserVariableInt(id,xMetadata,mCount) == 0) {
			/*
			If it's the only thing in the db, point it to itself and also set the database pointer to the new thing
			*/
			aiPlanSetUserVariableInt(id,xNextBlock,index,index);
			aiPlanSetUserVariableInt(id,xPrevBlock,index,index);
			aiPlanSetUserVariableInt(id,xMetadata,mPointer,index);
		} else {
			/*
			otherwise, slide in between two links in the list at mPointer
			*/
			before = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
			after = aiPlanGetUserVariableInt(id,xNextBlock,before);
			
			aiPlanSetUserVariableInt(id,xNextBlock,index,after); // next of me is after
			aiPlanSetUserVariableInt(id,xPrevBlock,index,before); // prev of me is before
			aiPlanSetUserVariableInt(id,xNextBlock,before,index); // next of before is me
			aiPlanSetUserVariableInt(id,xPrevBlock,after,index); // prev of after is me
		}
		
		aiPlanSetUserVariableInt(id,xMetadata,mCount, aiPlanGetUserVariableInt(id,xMetadata,mCount) + 1);
		aiPlanSetUserVariableInt(id,xMetadata,mCacheCount, aiPlanGetUserVariableInt(id,xMetadata,mCacheCount) - 1);
		success = true;
	}
	
	return(success);
}

bool xRestoreCache(int id = 0) {
	bool success = false;
	if (aiPlanGetUserVariableInt(id,xMetadata,mCacheCount) > 0) {
		int pointer = aiPlanGetUserVariableInt(id,xMetadata,mCacheHead);
		for(i=aiPlanGetUserVariableInt(id,xMetadata,mCacheCount); >0) {
			aiPlanSetUserVariableBool(id,xDirtyBit,pointer,true);
			pointer = aiPlanGetUserVariableInt(id,xNextBlock,pointer);
		}
		/* insert the ends of the chain into the database */
		if (aiPlanGetUserVariableInt(id,xMetadata,mCount) == 0) {
			/*
			If it's the only thing in the db, pointer now points to the cacheHead
			*/
			aiPlanSetUserVariableInt(id,xMetadata,mPointer,aiPlanGetUserVariableInt(id,xMetadata,mCacheHead));
		} else {
			/*
			otherwise, slide in between two links in the list at mPointer
			*/
			int before = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
			int after = aiPlanGetUserVariableInt(id,xNextBlock,before);
			int index = aiPlanGetUserVariableInt(id,xMetadata,mCacheHead);
			int next = aiPlanGetUserVariableInt(id,xPrevBlock,index); // the next of this block will be the after block
			
			aiPlanSetUserVariableInt(id,xNextBlock,next,after); // next of next is after
			aiPlanSetUserVariableInt(id,xPrevBlock,after,next); // prev of after is next
			
			aiPlanSetUserVariableInt(id,xNextBlock,before,index); // next of before is me
			aiPlanSetUserVariableInt(id,xPrevBlock,index,before); // prev of me is before
		}
		aiPlanSetUserVariableInt(id,xMetadata,mCount,
			aiPlanGetUserVariableInt(id,xMetadata,mCount) + aiPlanGetUserVariableInt(id,xMetadata,mCacheCount));
		aiPlanSetUserVariableInt(id,xMetadata,mCacheHead,0);
		aiPlanSetUserVariableInt(id,xMetadata,mCacheCount,0);
		success = true;
	}
	return(success);
}

int xGetNewestPointer(int id = 0) {
	return(aiPlanGetUserVariableInt(id,xMetadata,mNewestBlock));
}

int xDatabaseNext(int id = 0, bool reverse = false) {
	int pointer = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	if (reverse) {
		pointer = aiPlanGetUserVariableInt(id,xPrevBlock,pointer);
	} else {
		pointer = aiPlanGetUserVariableInt(id,xNextBlock,pointer);
	}
	if (aiPlanGetUserVariableBool(id,xDirtyBit,pointer) && (aiPlanGetUserVariableInt(id,xMetadata,mCount) > 0)) {
		aiPlanSetUserVariableInt(id,xMetadata,mPointer,pointer);
	} else {
		pointer = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
		debugLog("xDatabaseNext: " + aiPlanGetName(id) + " pointer is incorrect!");
		debugLog("xNextBlock: " + aiPlanGetUserVariableInt(id,xNextBlock,pointer));
		debugLog("Me: " + pointer);
		debugLog("xPrevblock: " + aiPlanGetUserVariableInt(id,xPrevBlock,pointer));
	}
	return(pointer);
}

void xClearDatabase(int id = 0) {
	int next = aiPlanGetUserVariableInt(id,xMetadata,mNextFree);
	int pointer = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	aiPlanSetUserVariableInt(id,xMetadata,mNextFree,aiPlanGetUserVariableInt(id,xNextBlock,pointer));
	aiPlanSetUserVariableInt(id,xNextBlock,pointer,next);

	for(i=0; < aiPlanGetNumberUserVariableValues(id,xDirtyBit)) {
		aiPlanSetUserVariableBool(id,xDirtyBit,i,false);
	}
	
	aiPlanSetUserVariableInt(id,xMetadata,mCount,0);
	aiPlanSetUserVariableInt(id,xMetadata,mPointer,0);
}

void xResetDatabase(int id = 0) {
	int size = aiPlanGetNumberUserVariableValues(id,xDirtyBit);
	aiPlanSetUserVariableInt(id,xMetadata,mPointer,0);
	aiPlanSetUserVariableInt(id,xMetadata,mCount,0);
	aiPlanSetUserVariableInt(id,xMetadata,mCacheHead,0);
	aiPlanSetUserVariableInt(id,xMetadata,mCacheCount,0);
	
	aiPlanSetUserVariableInt(id,xMetadata,mNextFree,size - 1);
	aiPlanSetUserVariableInt(id,xNextBlock,0,0);
	for(i=1; < size) { // connect all the free buffers together
		aiPlanSetUserVariableBool(id,xDirtyBit,i,false);
		aiPlanSetUserVariableInt(id,xNextBlock,i,i-1);
	}
}

int xGetInt(int id = 0, int data = 0, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mInt) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xGetInt: " + aiPlanGetName(id) + " variable " + name + " is not an int! Type: " + type);
		return(-1); // if we are trying to get an int from the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanGetUserVariableInt(id,data,index));
}

bool xSetInt(int id = 0, int data = 0, int val = 0, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mInt) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xSetInt: " + aiPlanGetName(id) + " variable " + name + " is not an int! Type: " + type);
		return(false); // if we are trying to set the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	bool success = aiPlanSetUserVariableInt(id,data,index,val);
	if (success == false) {
		string err = ": Could not assign value: " + val;
		debugLog("xSetInt: " + aiPlanGetName(id) + aiPlanGetUserVariableString(id,xVarNames,data - xVarNames) + err);
	}
	return(success);
}


float xGetFloat(int id = 0, int data = 0, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mFloat) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xGetFloat: " + aiPlanGetName(id) + " variable " + name + " is not a float! Type: " + type);
		return(-1.0); // if we are trying to get an int from the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanGetUserVariableFloat(id,data,index));
}

bool xSetFloat(int id = 0, int data = 0, float val = 0, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mFloat) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xSetFloat: " + aiPlanGetName(id) + " variable " + name + " is not a float! Type: " + type);
		return(false); // if we are trying to set the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	bool success = aiPlanSetUserVariableFloat(id,data,index,val);
	if (success == false) {
		string err = ": Could not assign value: " + val;
		debugLog("xSetFloat: " + aiPlanGetName(id) + aiPlanGetUserVariableString(id,xVarNames,data - xVarNames) + err);
	}
	return(success);
}


string xGetString(int id = 0, int data = 0, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mString) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xGetString: " + aiPlanGetName(id) + " variable " + name + " is not a string! Type: " + type);
		return(""); // if we are trying to get an int from the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanGetUserVariableString(id,data,index));
}

bool xSetString(int id = 0, int data = 0, string val = "", int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mString) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xSetString: " + aiPlanGetName(id) + " variable " + name + " is not a string! Type: " + type);
		return(false); // if we are trying to set the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanSetUserVariableString(id,data,index,val));
}


vector xGetVector(int id = 0, int data = 0, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mVector) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xGetVector: " + aiPlanGetName(id) + " variable " + name + " is not a vector! Type: " + type);
		return(vector(0,0,0)); // if we are trying to get an int from the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanGetUserVariableVector(id,data,index));
}

bool xSetVector(int id = 0, int data = 0, vector val = vector(0,0,0), int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mVector) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xSetVector: " + aiPlanGetName(id) + " variable " + name + " is not a vector! Type: " + type);
		return(false); // if we are trying to set the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanSetUserVariableVector(id,data,index,val));
}


bool xGetBool(int id = 0, int data = 0, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mBool) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xGetBool: " + aiPlanGetName(id) + " variable " + name + " is not a bool! Type: " + type);
		return(false); // if we are trying to get an int from the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanGetUserVariableBool(id,data,index));
}

bool xSetBool(int id = 0, int data = 0, bool val = false, int index = -1) {
	if (aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes) != mBool) {
		string type = datatypeName(aiPlanGetUserVariableInt(id,xMetadata,data - xVarNames + mVariableTypes));
		string name = aiPlanGetUserVariableString(id,xVarNames,data - xVarNames);
		debugLog("xGetBool: " + aiPlanGetName(id) + " variable " + name + " is not a bool! Type: " + type);
		return(false); // if we are trying to set the wrong datatype, stop
	}
	if (index == -1) {
		index = aiPlanGetUserVariableInt(id,xMetadata,mPointer);
	}
	return(aiPlanSetUserVariableBool(id,data,index,val));
}

int xGetDatabaseCount(int id = 0) {
	return(aiPlanGetUserVariableInt(id,xMetadata,mCount));
}

int xGetPointer(int id = 0) {
	return(aiPlanGetUserVariableInt(id,xMetadata,mPointer));
}

void xPrintAll(int id = 0, int index = 0) {
	trChatSend(0, "<u>" + aiPlanGetName(id) + "</u>");
	trChatSend(0, "size: " + xGetDatabaseCount(id));
	trChatSend(0, "pointer: " + index);
	for(i=1; < aiPlanGetNumberUserVariableValues(id,xVarNames)) {
		string name = aiPlanGetUserVariableString(id,xVarNames,i);
		int type = aiPlanGetUserVariableInt(id,xMetadata,mVariableTypes + i);
		switch(type)
		{
			case mInt:
			{
				trChatSend(0, name + ": " + aiPlanGetUserVariableInt(id,xVarNames + i,index));
			}
			case mFloat:
			{
				trChatSend(0, name + ": " + aiPlanGetUserVariableFloat(id,xVarNames + i,index));
			}
			case mString:
			{
				trChatSend(0, name + ": " + aiPlanGetUserVariableString(id,xVarNames + i,index));
			}
			case mVector:
			{
				trChatSend(0, name + ": " + aiPlanGetUserVariableVector(id,xVarNames + i,index));
			}
			case mBool:
			{
				if (aiPlanGetUserVariableBool(id,xVarNames + i,index)) {
					trChatSend(0, name + ": true");
				} else {
					trChatSend(0, name + ": false");
				}
			}
		}
	}
}

void xUnitSelect(int id = 0, int varn = 0, bool reverse = true) {
	trUnitSelectClear();
	trUnitSelect(""+xGetInt(id,varn), reverse);
}

void xUnitSelectByID(int db = 0, int varn = 0) {
	trUnitSelectClear();
	trUnitSelectByID(xGetInt(db,varn));
}

rule mInitializeMemory
active
highFrequency
{
	xsDisableSelf();
	aiSet("NoAI", 0);
	MALLOC = aiPlanCreate("memory",8);
	ARRAYS = aiPlanCreate("arrays",8);
	for(i=0; < 5) {
		aiPlanAddUserVariableBool(MALLOC,i * 3 + xDirtyBit - 1,"DirtyBit"+i,1);
		aiPlanAddUserVariableInt(MALLOC,i * 3 + xNextBlock - 1,"NextBlock"+i,1);
		aiPlanSetUserVariableBool(MALLOC,i * 3 + xDirtyBit - 1, NEXTFREE, true);
		aiPlanSetUserVariableInt(MALLOC,i * 3 + xNextBlock - 1, NEXTFREE, 0);
	}
	aiPlanAddUserVariableInt(MALLOC,mInt * 3 + xData - 1, "intData",1);
	aiPlanAddUserVariableFloat(MALLOC,mFloat * 3 + xData - 1, "floatData",1);
	aiPlanAddUserVariableString(MALLOC,mString * 3 + xData - 1, "stringData",1);
	aiPlanAddUserVariableVector(MALLOC,mVector * 3 + xData - 1, "vectorData",1);
	aiPlanAddUserVariableBool(MALLOC,mBool * 3 + xData - 1, "boolData",1);
	
	aiPlanAddUserVariableString(MALLOC,15,"datatypes",5);
	aiPlanSetUserVariableString(MALLOC,15,mInt,"Integer");
	aiPlanSetUserVariableString(MALLOC,15,mFloat,"Float");
	aiPlanSetUserVariableString(MALLOC,15,mString,"String");
	aiPlanSetUserVariableString(MALLOC,15,mVector,"Vector");
	aiPlanSetUserVariableString(MALLOC,15,mBool,"Bool");
}


void trVectorQuestVarSet(string name = "", vector QVv = vector(-1,-1,-1)) {
	if (name == "") return;
	int old = xsGetContextPlayer();
	xsSetContextPlayer(0);
	if (trQuestVarGet("vector" + name) == 0) {
		trQuestVarSet("vector"+ name, mNewVector(QVv));
	} else {
		mSetVector(1*trQuestVarGet("vector"+name), QVv);
	}
	xsSetContextPlayer(old);
}

vector trVectorQuestVarGet(string name = "") {
	int old = xsGetContextPlayer();
	xsSetContextPlayer(0);
	vector ret = mGetVector(1*trQuestVarGet("vector"+name));
	xsSetContextPlayer(old);
	return(ret);
}

float trVectorQuestVarGetX(string name = "") {
	return(xsVectorGetX(trVectorQuestVarGet(name)));
}

float trVectorQuestVarGetY(string name = "") {
	return(xsVectorGetY(trVectorQuestVarGet(name)));
}

float trVectorQuestVarGetZ(string name = "") {
	return(xsVectorGetZ(trVectorQuestVarGet(name)));
}

void trVectorQuestVarEcho(string name = "") {
	if (name == "") return;
	trChatSend(0, ""+name+": "+trVectorQuestVarGet(name));
}


void trStringQuestVarSet(string name = "", string value = "") {
	int old = xsGetContextPlayer();
	xsSetContextPlayer(0);
	if (trQuestVarGet("string"+name) > 0) {
		mSetString(1*trQuestVarGet("string"+name), value);
	} else {
		trQuestVarSet("string"+name, mNewString(value));
	}
	xsSetContextPlayer(old);
}

string trStringQuestVarGet(string name="") {
	int old = xsGetContextPlayer();
	xsSetContextPlayer(0);
	string val = kbArmyGetName(1*trQuestVarGet("string"+name));
	xsSetContextPlayer(old);
	return(val);
}


bool playerIsPlaying(int p = 0) {
	return(kbIsPlayerHuman(p) == true && kbIsPlayerResigned(p) == false && trPlayerDefeated(p) == false);
}

bool playerIsNOTPlaying(int p = 0) {
	return(kbIsPlayerHuman(p) == false || kbIsPlayerResigned(p) == true || trPlayerDefeated(p) == true);
}


void trUnitTeleportToVector(string v = "") {
	vector pos = trVectorQuestVarGet(v);
	trUnitTeleport(xsVectorGetX(pos),xsVectorGetY(pos),xsVectorGetZ(pos));
}

void trUnitSelectByQV(string s = "", bool reverse = true) {
	trUnitSelectClear();
	trUnitSelect(""+1*trQuestVarGet(s), reverse);
}

/*
Given a quest var that stores a unit name, store
the unit's position in the vector.
*/
void trVectorSetUnitPos(string v = "", string db = "", bool reverse = true) {
	trVectorQuestVarSet(v, kbGetBlockPosition(""+1*trQuestVarGet(db), reverse));
}

void trVectorSetUnitPosInt(string v = "", int val = 0, bool reverse = true) {
	trVectorQuestVarSet(v, kbGetBlockPosition(""+val, reverse));
}


void trUnitMoveToVector(string v = "", bool attack = false) {
	vector pos = trVectorQuestVarGet(v);
	trUnitMoveToPoint(xsVectorGetX(pos),0,xsVectorGetZ(pos),-1,attack);
}

void trVectorScale(string db = "", float s = 1.0) {
	trVectorQuestVarSet(db, trVectorQuestVarGet(db) * s);
}


void vectorSnapToGrid(string qv = "") {
	vector pos = trVectorQuestVarGet(qv);
	int x = xsVectorGetX(pos) / 2;
	int z = xsVectorGetZ(pos) / 2;
	trVectorQuestVarSet(qv, xsVectorSet(x * 2 + 1, 0, z * 2 + 1));
}

int iModulo(int mod = 10, int val = 0) {
	return(val - val / mod * mod);
}

float fModulo(float mod = 0, float val = 0) {
	int c = 0;
	if (val > 0) {
		c = val / mod;
	} else {
		c = val / mod - 1;
	}
	return(0.0 + val - mod * c);
}

bool getBit(int bit = 0, int val = 0) {
	val = val / xsPow(2, bit);
	return((iModulo(2, val) == 1));
}

void zUnitHeading(float a = 0) {
	trSetUnitOrientation(xsVectorSet(xsSin(a),0,xsCos(a)), xsVectorSet(0,1,0), true);
}

void zInitProtoUnitStat(string r = "", int p = 0, int f = 0, float v = 0.0) {
	trQuestVarSet("p"+p+"pf"+kbGetProtoUnitID(r)+"f"+f, v);
}

void zSetProtoUnitStat(string r = "", int p = 0, int f = 0, float v = 0.0) {
for(zsps=0; >1){}
	zsps = kbGetProtoUnitID(r);
	trModifyProtounit(r, p, f, 0.0 + v - trQuestVarGet("p"+p+"pf"+zsps+"f"+f));
	trQuestVarSet("p"+p+"pf"+zsps+"f"+f, 0.0 + v);
}

void vectorToGrid(string from = "", string to = ""){
	int x = 0 + trQuestVarGet(from+"x") / 2;
	int z = 0 + trQuestVarGet(from+"z") / 2;
	trQuestVarSet(to+"x", x);
	trQuestVarSet(to+"z", z);
}

void gridToVector(string from = "", string to = "") {
	trQuestVarSet(to+"x", trQuestVarGet(from+"x") * 2 + 1);
	trQuestVarSet(to+"z", trQuestVarGet(from+"z") * 2 + 1);
}

float gridDistanceSquared(string from = "", string to = "") {
	float xdiff = trQuestVarGet(to+"x") - trQuestVarGet(from+"x");
	float zdiff = trQuestVarGet(to+"z") - trQuestVarGet(from+"z");
	return(xdiff * xdiff + zdiff * zdiff);
}

void zSquareVar(string qv = "") {
	trQuestVarSet(qv, xsPow(trQuestVarGet(qv), 2));
}

float zDistanceBetweenVectorsSquared(string start = "", string end = "") {
	float xdiff = trQuestVarGet(end + "X") - trQuestVarGet(start + "X");
	float zdiff = trQuestVarGet(end + "Z") - trQuestVarGet(start + "Z");
	float dist = xdiff * xdiff + zdiff * zdiff;
	return(dist);
}

bool vectorInRectangle(string pos = "", string bottom = "", string top = "") {
	if (trQuestVarGet(pos+"x") < trQuestVarGet(bottom+"x")) {
		return(false);
	}
	if (trQuestVarGet(pos+"x") > trQuestVarGet(top+"x")) {
		return(false);
	}
	if (trQuestVarGet(pos+"z") < trQuestVarGet(bottom+"z")) {
		return(false);
	}
	if (trQuestVarGet(pos+"z") > trQuestVarGet(top+"z")) {
		return(false);
	}
	return(true);
}

vector rotationMatrix(string v = "", float cosT = 0, float sinT = 0) {
	float x = trQuestVarGet(v+"x");
	float z = trQuestVarGet(v+"z");
	vector ret = xsVectorSet(x * cosT - z * sinT, 0, x * sinT + z * cosT);
	return(ret);
}

float zDistanceBetweenVectors(string start = "", string end = "") {
	float xdiff = trQuestVarGet(end + "X") - trQuestVarGet(start + "X");
	float zdiff = trQuestVarGet(end + "Z") - trQuestVarGet(start + "Z");
	float dist = xsSqrt(xdiff * xdiff + zdiff * zdiff);
	return(dist);
}

float zDistanceBetweenVectors3d(string start = "", string end = "") {
	float xdiff = trQuestVarGet(end + "X") - trQuestVarGet(start + "X");
	float ydiff = trQuestVarGet(end + "Y") - trQuestVarGet(start + "Y");
	float zdiff = trQuestVarGet(end + "Z") - trQuestVarGet(start + "Z");
	float dist = xsSqrt(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
	return(dist);
}

float zDistanceToVectorSquared(string qv = "", string v = "") {
	trVectorQuestVarSet("abcd", kbGetBlockPosition(""+1*trQuestVarGet(qv), true));
	return(zDistanceBetweenVectorsSquared("abcd", v));
}

/* For use in a ySearch */
float zDistanceToVector(string qv = "", string v = "") {
	trVectorQuestVarSet("abcd", kbGetBlockPosition(""+1*trQuestVarGet(qv), true));
	return(zDistanceBetweenVectors("abcd", v));
}

void trVectorSetFromAngle(string qv = "", float angle = 0) {
	trVectorQuestVarSet(qv,xsVectorSet(xsSin(angle), 0, xsCos(angle)));
}

float angleBetweenVectors(string from = "", string to = "") {
	float a = trQuestVarGet(to+"X")-trQuestVarGet(from+"X");
	a = a / (trQuestVarGet(to+"Z")-trQuestVarGet(from+"Z"));
	a = xsAtan(a);
	if (trVectorQuestVarGetZ(from) > trVectorQuestVarGetZ(to)) {
		if (trVectorQuestVarGetX(from) > trVectorQuestVarGetX(to)) {
			a = a - PI;
		} else {
			a = a + PI;
		}
	}
	return(a);
}

float angleOfVector(string dir = "") {
	float a = trQuestVarGet(dir+"X") / trQuestVarGet(dir+"Z");
	a = xsAtan(a);
	if (0.0 > trVectorQuestVarGetZ(dir)) {
		if (0.0 > trVectorQuestVarGetX(dir)) {
			a = a - PI;
		} else {
			a = a + PI;
		}
	}
	return(a);
}

vector zGetUnitVector(string start = "", string end = "", float mod = 1.0) {
	float xdiff = trQuestVarGet(end + "X") - trQuestVarGet(start + "X");
	float zdiff = trQuestVarGet(end + "Z") - trQuestVarGet(start + "Z");
	float dist = xsSqrt(xdiff * xdiff + zdiff * zdiff);
	vector ret = vector(1,0,0);
	if (dist > 0) {
		ret = xsVectorSet(xdiff / dist * mod, 0, zdiff / dist * mod);
	}
	return(ret);
}

vector zGetUnitVector3d(string start = "", string end = "", float mod = 1.0) {
	float xdiff = trQuestVarGet(end + "X") - trQuestVarGet(start + "X");
	float ydiff = trQuestVarGet(end + "Y") - trQuestVarGet(start + "Y");
	float zdiff = trQuestVarGet(end + "Z") - trQuestVarGet(start + "Z");
	float dist = xsSqrt(xdiff * xdiff + ydiff * ydiff + zdiff * zdiff);
	vector ret = xsVectorSet(xdiff / dist * mod, ydiff / dist * mod, zdiff / dist * mod);
	return(ret);
}

vector crossProduct(string a = "", string b = "") {
	float x = trQuestVarGet(a + "y") * trQuestVarGet(b + "z") - trQuestVarGet(a + "z") * trQuestVarGet(b + "y");
	float y = trQuestVarGet(a + "z") * trQuestVarGet(b + "x") - trQuestVarGet(a + "x") * trQuestVarGet(b + "z");
	float z = trQuestVarGet(a + "x") * trQuestVarGet(b + "y") - trQuestVarGet(a + "y") * trQuestVarGet(b + "x");
	vector ret = xsVectorSet(x, y, z);
	return(ret);
}

float dotProduct(string a = "", string b = "") {
	return(trQuestVarGet(a+"x") * trQuestVarGet(b+"x") + trQuestVarGet(a+"z") * trQuestVarGet(b+"z"));
}

bool terrainIsType(string qv = "", int type = 0, int subtype = 0) {
	bool isType = trGetTerrainType(trQuestVarGet(qv+"x"),trQuestVarGet(qv+"z")) == type;
	isType = trGetTerrainSubType(trQuestVarGet(qv+"x"),trQuestVarGet(qv+"z")) == subtype;
	return(isType);
}

/*
A shitty binary search algorithm to approximate the intersection of a line with
the circle specified by the center vector and radius. Behavior is undefined if start
vector is outside the circle.
Did this to avoid using trig as much as possible because trig is expensive.
*/
vector intersectionWithCircle(string start = "", string end = "", string center = "", float radius = 0) {
	trVectorQuestVarSet("iDir", zGetUnitVector(start, end));
	float x = 0;
	float z = 0;
	float dist = 0;
	float len = radius * 2;
	float lenmod = radius;
	for(i=8; >0) {
		x = trQuestVarGet(start+"x") + trQuestVarGet("iDirx") * len;
		z = trQuestVarGet(start+"z") + trQuestVarGet("iDirz") * len;
		dist = xsPow(x - trQuestVarGet(center+"x"), 2) + xsPow(z - trQuestVarGet(center+"z"), 2);
		if (dist > radius * radius) {
			len = len - lenmod;
		} else {
			len = len + lenmod;
		}
		lenmod = lenmod * 0.5;
	}
	return(xsVectorSet(x,0,z));
}

/* initializes a modular counter. */
void modularCounterInit(string name = "", int size = 0) {
	trQuestVarSet("counter" + name + "size", size);
	trQuestVarSet("counter" + name + "pointer", 1);
	trQuestVarSet(name, 1);
}

/* Progresses the modular counter by 1 and returns the value */
int modularCounterNext(string name = "") {
	trQuestVarSet("counter" + name + "pointer", 1 + trQuestVarGet("counter" + name + "pointer"));
	if (trQuestVarGet("counter" + name + "pointer") > trQuestVarGet("counter" + name + "size")) {
		trQuestVarSet("counter" + name + "pointer", 1);
	}
	trQuestVarSet(name, trQuestVarGet("counter"+name+"pointer"));
	return(0 + trQuestVarGet("counter" + name + "pointer"));
}

/* Peeks at the next value of the modular counter */
int peekModularCounterNext(string name = "") {
	trQuestVarSet("counter" + name + "fake", 1 + trQuestVarGet("counter" + name + "pointer"));
	if (trQuestVarGet("counter" + name + "fake") >= trQuestVarGet("counter" + name + "size")) {
		trQuestVarSet("counter" + name + "fake", 1);
	}
	return(0 + trQuestVarGet("counter" + name + "fake"));
}


float yGetVarAtIndex(string db = "", string var = "", int index = 0) {
	return(trQuestVarGet("xdata"+db+index+var));
}

float yGetVar(string db = "", string var = "") {
	int index = trQuestVarGet("xdata"+db+"pointer");
	return(trQuestVarGet("xdata"+db+index+var));
}

string yGetStringAtIndex(string db = "", string var = "", int index = 0) {
	return(trStringQuestVarGet("xdata"+db+index+var));
}

string yGetString(string db = "", string var = "") {
	int index = trQuestVarGet("xdata"+db+"pointer");
	return(trStringQuestVarGet("xdata"+db+index+var));
}

string yGetVarName(string db = "", string var = "") {
	int index = trQuestVarGet("xdata"+db+"pointer");
	return("xdata"+db+index+var);
}

void ySetVarAtIndex(string db = "", string var = "", float val = 0, int index = 0) {
	trQuestVarSet("xdata"+db+index+var, val);
}

void ySetVar(string db = "", string var = "", float val = 0) {
	int index = trQuestVarGet("xdata"+db+"pointer");
	ySetVarAtIndex(db, var, val, index);
}

void ySetStringAtIndex(string db = "", string var = "", string val = "", int index = 0) {
	trStringQuestVarSet("xdata"+db+index+var, val);
}

void ySetString(string db = "", string var = "", string val = "") {
	int index = trQuestVarGet("xdata"+db+"pointer");
	ySetStringAtIndex(db, var, val, index);
}

int yDatabaseNext(string db = "", bool select = false, bool reverse = false) {
	int index = yGetVar(db, "xNextBlock");
	if (reverse) {
		index = yGetVar(db, "xPrevBlock");
	}
	if (yGetVar(db, "xActive") == 0) {
		if (trCurrentPlayer() == 1) {
			trSoundPlayFN("attackwarning.wav","1",-1,"","");
			debugLog("<color=1,0,0>"+db+" is pointing to something wrong!");
		}
	}
	trQuestVarSet("xdata"+db+"pointer", index);
	int u = trQuestVarGet("xdata"+db+"index"+index);
	trQuestVarSet(db, u);
	if (select) {
		trUnitSelectClear();
		trUnitSelect(""+u, true);
		return(kbGetBlockID(""+u, true));
	} else {
		return(trQuestVarGet(db));
	}
}

void yRemoveFromDatabase(string db = "") {
	int index = trQuestVarGet("xdata"+db+"pointer");
	if (yGetVar(db, "xActive") == 1) {
		/* connect next with prev */
		ySetVarAtIndex(db, "xNextBlock", yGetVar(db, "xNextBlock"), 1*yGetVar(db, "xPrevBlock"));
		ySetVarAtIndex(db, "xPrevBlock", yGetVar(db, "xPrevBlock"), 1*yGetVar(db, "xNextBlock"));
		
		ySetVar(db, "xNextBlock", trQuestVarGet("xdata"+db+"nextFree"));
		ySetVar(db, "xActive", 0);
		trQuestVarSet("xdata"+db+"nextFree", index);
		
		trQuestVarSet("xdata"+db+"pointer", yGetVar(db, "xPrevBlock"));
		trQuestVarSet("xdata"+db+"count", trQuestVarGet("xdata"+db+"count") - 1);
	}
}

void yRemoveUpdateVar(string db = "", string attr = "") {
	int index = trQuestVarGet("xdata"+db+"nextFree");
	ySetVarAtIndex(db, attr, 0, index);
}

int yAddToDatabase(string db = "", string val = "") {
	int next = trQuestVarGet("xdata"+db+"nextFree");
	if (next == 0) {
		next = 1 + trQuestVarGet("xdata"+db+"total");
		trQuestVarSet("xdata"+db+"total", next);
	} else {
		trQuestVarSet("xdata"+db+"nextFree", yGetVarAtIndex(db, "xNextBlock", next));
	}
	trQuestVarSet("xdata"+db+"index"+next, trQuestVarGet(val));
	ySetVarAtIndex(db, "xActive", 1, next);
	if (trQuestVarGet("xdata"+db+"count") == 0) {
		ySetVarAtIndex(db, "xNextBlock", next, next);
		ySetVarAtIndex(db, "xPrevBlock", next, next);
		trQuestVarSet("xdata"+db+"pointer", next);
	} else {
		int index = trQuestVarGet("xdata"+db+"pointer");
		ySetVarAtIndex(db, "xNextBlock", yGetVarAtIndex(db, "xNextBlock", index), next);
		ySetVarAtIndex(db, "xPrevBlock", index, next);
		ySetVarAtIndex(db, "xNextBlock", next, index);
		ySetVarAtIndex(db, "xPrevBlock", next, 1*yGetVarAtIndex(db, "xNextBlock", next));
	}
	trQuestVarSet("xdata"+db+"newest", next);
	trQuestVarSet("xdata"+db+"count", trQuestVarGet("xdata"+db+"count") + 1);
	return(next);
}

string yGetNewestName(string db = "") {
	return("xdata"+db+"index"+1*trQuestVarGet("xdata"+db+"newest"));
}

int yGetNewestPointer(string db = "") {
	return(1*trQuestVarGet("xdata"+db+"newest"));
}

string yGetNewVarName(string db = "", string var = "") {
	int index = yGetNewestPointer(db);
	return("xdata"+db+index+var);
}

void yAddUpdateVar(string db = "", string var = "", float val = 0) {
	ySetVarAtIndex(db, var, val, 1*trQuestVarGet("xdata"+db+"newest"));
}

void yAddUpdateString(string db = "", string var = "", string val = "") {
	ySetStringAtIndex(db, var, val, 1*trQuestVarGet("xdata"+db+"newest"));
}

int yGetDatabaseCount(string db = "") {
	return(1*trQuestVarGet("xdata"+db+"count"));
}

int yGetUnitAtIndex(string db = "", int index = 0) {
	return(1*trQuestVarGet("xdata"+db+"index"+index));
}

void ySetUnitAtIndex(string db = "", int index = 0, int value = 0) {
	trQuestVarSet("xdata"+db+"index"+index, value);
}

void ySetUnit(string db = "", int value = 0) {
	int index = trQuestVarGet("xdata"+db+"pointer");
	ySetUnitAtIndex(db, index, value);
}

int yGetPointer(string db = "") {
	return(1*trQuestVarGet("xdata"+db+"pointer"));
}

bool ySetPointer(string db = "", int index = 0) {
	bool safe = (yGetVarAtIndex(db, "xActive", index) == 1);
	if (safe) {
		trQuestVarSet("xdata"+db+"pointer", index);
		trQuestVarSet(db, trQuestVarGet("xdata"+db+"index"+index));
	}
	return(safe);
}

void yClearDatabase(string db = "") {
	int index = trQuestVarGet("xdata"+db+"nextFree");
	trQuestVarSet("xdata"+db+"nextFree", yGetVar(db, "xNextBlock"));
	ySetVar(db, "xNextBlock", index);
	trQuestVarSet("xdata"+db+"count", 0);
	trQuestVarSet("xdata"+db+"pointer", 0);
}

void yVarToVector(string db = "", string v = "") {
	trQuestVarSet(v+"x", yGetVar(db, v + "x"));
	trQuestVarSet(v+"z", yGetVar(db, v + "z"));
}

void ySetVarFromVector(string db = "", string attr = "", string v = "") {
	ySetVar(db, attr+"x", trQuestVarGet(v+"x"));
	ySetVar(db, attr+"z", trQuestVarGet(v+"z"));
}

/*
Starting from NextUnitScenarioNameNumber and going backwards until the quest var 'qv',
looks for the specified protounit. If none found, returns -1. Otherwise, returns the
unit name.
*/
int yFindLatestReverse(string qv = "", string proto = "", int p = 0) {
	int id = kbGetProtoUnitID(proto);
	trUnitSelectClear();
	for(x=trGetNextUnitScenarioNameNumber(); >trQuestVarGet(qv)) {
		int i = kbGetBlockID(""+x, true);
		if (kbGetUnitBaseTypeID(i) == id) {
			trUnitSelectClear();
			trUnitSelectByID(i);
			if (trUnitIsOwnedBy(p)) {
				trQuestVarSet(qv, x);
				return(i);
			}
		}
	}
	return(-1);
}

/*
Starting from quest var 'qv' and going up until NextUnitScenarioNameNumber,
looks for the specified protounit. If none found, returns -1. Otherwise, returns the
unit name.
*/
int yFindLatest(string qv = "", string proto = "", int p = 0) {
	int id = kbGetProtoUnitID(proto);
	trUnitSelectClear();
	trQuestVarSet("next",trGetNextUnitScenarioNameNumber() - 1);
	while(trQuestVarGet("next") > trQuestVarGet(qv)) {
		trQuestVarSet(qv, 1 + trQuestVarGet(qv));
		int i = kbGetBlockID(""+1*trQuestVarGet(qv), true);
		if (kbGetUnitBaseTypeID(i) == id) {
			trUnitSelectClear();
			trUnitSelectByID(i);
			if (trUnitIsOwnedBy(p)) {
				return(i);
			}
		}
	}
	return(-1);
}

int yFindLatestAll(string qv = "", string proto = "") {
	int id = kbGetProtoUnitID(proto);
	trUnitSelectClear();
	trQuestVarSet("next",trGetNextUnitScenarioNameNumber() - 1);
	while(trQuestVarGet("next") > trQuestVarGet(qv)) {
		trQuestVarSet(qv, 1 + trQuestVarGet(qv));
		int i = kbGetBlockID(""+1*trQuestVarGet(qv), true);
		if (kbGetUnitBaseTypeID(i) == id) {
			trUnitSelectClear();
			trUnitSelectByID(i);
			return(i);
		}
	}
	return(-1);
}
