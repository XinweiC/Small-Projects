#include "predictor.h"


#define PHT_CTR_MAX  3
#define PHT_CTR_INIT 2

//gshare was 17
#define HIST_LEN   63


//Wendy
#define part1_budget 32768
//#define THRES 0

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
// Total storage budget: 32KB + 17 bits
// Total PHT counters: 2^17 
// Total PHT size = 2^17 * 2 bits/counter = 2^18 bits = 32KB
// GHR size: 17 bits
// Total Size = PHT size + GHR size
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

PREDICTOR::PREDICTOR(){
        ghr = 0;
        UINT32 i = 0;
        output = 0;
	    historyLength = HIST_LEN;//history;
		numberofEntries = part1_budget / (historyLength + 1);
		UINT32 colum = 0;
		//threshold = 0;
		threshold = 1.93 * historyLength + 14;     //function get from paper
		Entry = new INT32* [numberofEntries];
		for (i = 0; i< numberofEntries; i++)
		{
			Entry[i] = new INT32[historyLength + 1];

			for (colum = 0; colum < (historyLength + 1); colum++)
				Entry[i][colum] = 0;

		}
       // cout << numberofEntries << endl;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool   PREDICTOR::GetPrediction(UINT32 PC){
	    UINT64 i;
	    UINT32 Entryindex = PC % numberofEntries;
		INT32  prediction_result = Entry[Entryindex][0];

		for ( i = 1; i < (historyLength+1); i++)
		{
			//(ghr >> (i-1) % 2)
			//if ( (ghr & ( 1 << (i - 1))) == 0 ) {
			if (((ghr >> (i-1)) % 2) == 0) {
			    prediction_result = prediction_result- Entry[Entryindex][i];
			}
			else if (((ghr >> (i-1)) % 2) != 0) {
				prediction_result =  prediction_result + Entry[Entryindex][i];
			}
			else {
				cout << "error" << endl;
			}
		}
		output = prediction_result;

		if( prediction_result >=0 )
			return TAKEN;
		else
			return NOT_TAKEN;

  
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){
	UINT64 i;
	UINT32 Entryindex = PC % numberofEntries;
		INT32 t;
		INT32 x;

	    //UINT32 i = 1;
	    //UINT32 Entryindex = PC % numberofEntries;
/*		INT32  prediction_result = Entry[Entryindex][0];
		for ( i = 1; i < (historyLength+1); i++)
		{
			if ( (ghr & ( 1 << (i - 1))) == 0 )
				prediction_result = prediction_result- Entry[Entryindex][i];
			if ( (ghr & ( 1 << ( i - 1))) != 0 )
				prediction_result =  prediction_result + Entry[Entryindex][i];

		}
		output = prediction_result;
*/










		if (resolveDir == 1)
			t=1;
		if (resolveDir == 0)
			t=-1;

		if ((resolveDir != predDir)|| ((UINT32)abs(output) <= threshold))
		{
			for (i = 0; i < (historyLength + 1); i++)
			{
				if ( i == 0)
				{	x = 1;
				    if(t == 1) {//resolveDir == predDir) {
				    	if (Entry[Entryindex][i]<127)
				    	   Entry[Entryindex][i] += 1;
				    }
				    else {
				    	if (Entry[Entryindex][i]>-128)
				    	   Entry[Entryindex][i] -= 1;
				    }
				}

				else {
				   if (((ghr >> (i-1)) % 2) != 0){
					   x=1;
					  // Entry[Entryindex][i] += t * x;
				   }
				   else if (((ghr >> (i-1)) % 2) == 0){
					    x=-1;
					  //  Entry[Entryindex][i] += t * x;
				    }
				   else {
					   cout << "error" << endl;
				   }
				    if((t*x == 1) && Entry[Entryindex][i]<127)
				    	Entry[Entryindex][i] += t * x;
				    if((t*x == -1) && Entry[Entryindex][i]>-128)
				    	Entry[Entryindex][i] += t * x;
				}

//		        cout << Entry[Entryindex][i] << endl;
			}
		}

		ghr = (ghr <<1 | resolveDir);



}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget){

  // This function is called for instructions which are not
  // conditional branches, just in case someone decides to design
  // a predictor that uses information from such instructions.
  // We expect most contestants to leave this function untouched.

  return;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////



