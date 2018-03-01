#include "predictor.h"

#define WT_SIZE 10 // number of entries in weight tables
#define Length 128	// length of global history for SGHR, GHR, history_pc and train_need
#define weight_length_1 5 // history length for the single 1024-entry table
#define weight_length_2 52 // history length for the single 512-entry table



short	   weight1[1024][weight_length_1]; // weight table no.1 1024 entries
short	   weight2[512][weight_length_2]; // weight table no.2 512 entries



bool train_need; // 1 means need train
bool *GHR;
UINT32 *history_pc;	// as path information


int32_t threshold;
short threshold_dynamic; // dymamic threshold counter


/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
// Total storage budget: 32KB + 17 bits
// Total PHT counters: 2^17 
// Total PHT size = 2^17 * 2 bits/counter = 2^18 bits = 32KB
// GHR size: 17 bits
// Total Size = PHT size + GHR size
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

PREDICTOR::PREDICTOR(void){


    GHR = new bool[Length];
    history_pc = new UINT32[Length];
    threshold = 107;
  
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

UINT32 PREDICTOR::get_index(UINT32 pc_current, UINT32 pc_history, UINT32 wt_size)     //get index use history pc and current pc
{
    pc_current = (pc_current ) ^ (pc_current / (1<<wt_size));
    pc_history = (pc_history) ^ (pc_history / (1<<wt_size));
    UINT32 index = pc_current ^ (pc_history);
    index = index % (1<<wt_size);
    return index;
}
int output = 0;
bool PREDICTOR::Predict(UINT32 PC)
{
	output = 0;
	bool pred;
			for(int j=0; j<weight_length_1; j++)               //weight table no.1
			{
			  UINT32 index = get_index(PC, history_pc[j], WT_SIZE);
		    	  if( GHR[j] == 1)
			    output += weight1[index][j];
		    	  else
			    output -= weight1[index][j];
			}

			for(int j=0; j<weight_length_2; j++)
			{
			  UINT32 index = get_index(PC, history_pc[weight_length_1+j], WT_SIZE-1); //  weight table no.2
		    	  if( GHR[weight_length_1+j] == 1)
			    output += weight2[index][j];
		    	  else
			    output -= weight2[index][j];
			}

			if (output<0)
				pred = false;   //not taken
			else
				pred = true; //taken




			if(output>-threshold && output<threshold)    {
			  train_need=1;
			}
			else{
			  train_need=0;
			}

			  return pred;
}
void PREDICTOR::UpdatePcHistory(UINT32 PC)    //shift pc register left and put new pc on the least right
{

	   for(int j=Length-1; j>0; j--)
  {
		history_pc[j] = history_pc[j-1];
	}
	history_pc[0]=PC;
}
bool   PREDICTOR::GetPrediction(UINT32 PC){
	bool result = Predict(PC);
	UpdatePcHistory( PC);
	return result;
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void PREDICTOR::UpdateGHR( bool resolveDir)
{

		        for(int j=Length-1; j>0; j--)
		        {
				GHR[j] = GHR[j-1];
		        }
		        GHR[0] = resolveDir?1:0;


}
void PREDICTOR::Trianing(UINT32 PC, bool t, bool predDir, UINT32 branchTarget)
{
if( (t != predDir) || (train_need == 1) )
{

	  for(int j = 0; j < weight_length_1; j++)	{                       //update weight table no.1
	    UINT32 index = get_index(PC, history_pc[j+1], WT_SIZE);
	      if(t==GHR[1+j])
	      { if(weight1[index][j]<127) weight1[index][j]+=1;}
	      else
	      { if(weight1[index][j]>-128) weight1[index][j]-=1;}
	  }

	  for(int j = 0; j < weight_length_2; j++)	{                     //update weight table no.2
	    UINT32 index = get_index(PC, history_pc[j+weight_length_1+1], WT_SIZE-1);
	      if(t==GHR[1+j+weight_length_1])
	      { if(weight2[index][j]<127) weight2[index][j]+=1;}
	      else
	      { if(weight2[index][j]>-128) weight2[index][j]-=1;}
	  }


if(t != predDir) {                             //update dynamic threshold
    threshold_dynamic++;
    if(threshold_dynamic==63) {
   	threshold_dynamic = 0;
	threshold++;
    }
}
else if(t==predDir && train_need == 1) {
    threshold_dynamic--;
    if(threshold_dynamic==-63) {
	threshold_dynamic = 0;
	threshold--;
    }
}
}
}
void  PREDICTOR::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){
		UpdateGHR(resolveDir);
		Trianing( PC, resolveDir,  predDir,  branchTarget);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget){

  // This function is called for instructions which are not
  // conditional branches, just in case someone decides to design
  // a predictor thistory_pct uses information from such instructions.
  // We expect most contestants to leave this function untouched.

  return;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
