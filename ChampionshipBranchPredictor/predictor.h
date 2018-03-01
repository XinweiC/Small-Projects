#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include "utils.h"
#include "tracer.h"


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  UINT32  ghr;           // global history register
  UINT32  *pht;          // pattern history table
  UINT32  historyLength; // history length
  UINT32  numPhtEntries; // entries in pht 

 public:

  // The interface to the four functions below CAN NOT be changed

  PREDICTOR(void);
  bool    GetPrediction(UINT32 PC);  
  bool  Predict(UINT32 PC);
  void UpdatePcHistory(UINT32 PC);
  void UpdateGHR( bool resolveDir);
  void Trianing(UINT32 PC, bool t, bool predDir, UINT32 branchTarget);
  void    UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget);
  void    TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget);
  UINT32  get_index(UINT32 cur_pc, UINT32 path_pc, UINT32 wt_size);
  // Contestants can define their own functions below

};



/***********************************************************/
#endif

