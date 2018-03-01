#include "common.h"
#include "sim.h"
#include "trace.h" 
#include "cache.h"  /**** NEW-LAB2*/ 
#include "memory.h" // NEW-LAB2 
#include <stdlib.h>
#include <ctype.h> /* Library for useful character operations */
//new lab3
#include "bpred.h"
#include "vmem.h"


/*******************************************************************/
/* Simulator frame */ 
/*******************************************************************/

bool run_a_cycle(memory_c *main_memory); // please modify run_a_cycle function argument  /** NEW-LAB2 */
void init_structures(memory_c *main_memory); // please modify init_structures function argument  /** NEW-LAB2 */



/* uop_pool related variables */ 

uint32_t free_op_num;
uint32_t active_op_num; 
Op *op_pool; 
Op *op_pool_free_head = NULL; 

/* simulator related local functions */ 

bool icache_access(ADDRINT addr); /** please change uint32_t to ADDRINT NEW-LAB2 */
bool dcache_access(ADDRINT addr); /** please change uint32_t to ADDRINT NEW-LAB2 */
void  init_latches(void);

#include "knob.h"
#include "all_knobs.h"

// knob variables
KnobsContainer *g_knobsContainer; /* < knob container > */
all_knobs_c    *g_knobs; /* < all knob variables > */

gzFile g_stream;



void init_knobs(int argc, char** argv)
{
  // Create the knob managing class
  g_knobsContainer = new KnobsContainer();

  // Get a reference to the actual knobs for this component instance
  g_knobs = g_knobsContainer->getAllKnobs();

  // apply the supplied command line switches
  char* pInvalidArgument = NULL;
  g_knobsContainer->applyComandLineArguments(argc, argv, &pInvalidArgument);

  g_knobs->display();
}

void read_trace_file(void)
{
  g_stream = gzopen((KNOB(KNOB_TRACE_FILE)->getValue()).c_str(), "r");
}

// simulator main function is called from outside of this file 

void simulator_main(int argc, char** argv) 
{
  init_knobs(argc, argv);

  // trace driven simulation 
  read_trace_file();
   /** NEW-LAB2 */ /* just note: passing main memory pointers is hack to mix up c++ objects and c-style code */  /* Not recommended at all */ 
  memory_c *main_memory = new memory_c();  // /** NEW-LAB2 */ 


  init_structures(main_memory);  // please modify run_a_cycle function argument  /** NEW-LAB2 */ 
  run_a_cycle(main_memory); // please modify run_a_cycle function argument  /** NEW-LAB2 */ 

}
int op_latency[NUM_OP_TYPE]; 

void init_op_latency(void)
{
  op_latency[OP_INV]   = 1; 
  op_latency[OP_NOP]   = 1; 
  op_latency[OP_CF]    = 1; 
  op_latency[OP_CMOV]  = 1; 
  op_latency[OP_LDA]   = 1;
  op_latency[OP_LD]    = 1; 
  op_latency[OP_ST]    = 1; 
  op_latency[OP_IADD]  = 1; 
  op_latency[OP_IMUL]  = 2; 
  op_latency[OP_IDIV]  = 4; 
  op_latency[OP_ICMP]  = 2; 
  op_latency[OP_LOGIC] = 1; 
  op_latency[OP_SHIFT] = 2; 
  op_latency[OP_BYTE]  = 1; 
  op_latency[OP_MM]    = 2; 
  op_latency[OP_FMEM]  = 2; 
  op_latency[OP_FCF]   = 1; 
  op_latency[OP_FCVT]  = 4; 
  op_latency[OP_FADD]  = 2; 
  op_latency[OP_FMUL]  = 4; 
  op_latency[OP_FDIV]  = 16; 
  op_latency[OP_FCMP]  = 2; 
  op_latency[OP_FBIT]  = 2; 
  op_latency[OP_FCMP]  = 2; 
}

void init_op(Op *op)
{
  op->num_src               = 0; 
  op->src[0]                = -1; 
  op->src[1]                = -1;
  op->dst                   = -1; 
  op->opcode                = 0; 
  op->is_fp                 = false;
  op->cf_type               = NOT_CF;
  op->mem_type              = NOT_MEM;
  op->write_flag             = 0;
  op->inst_size             = 0;
  op->ld_vaddr              = 0;
  op->st_vaddr              = 0;
  op->instruction_addr      = 0;
  op->branch_target         = 0;
  op->actually_taken        = 0;
  op->mem_read_size         = 0;
  op->mem_write_size        = 0;
  op->valid                 = FALSE; 
  /* you might add more features here */ 
}


void init_op_pool(void)
{
  /* initialize op pool */ 
  op_pool = new Op [1024];
  free_op_num = 1024; 
  active_op_num = 0; 
  uint32_t op_pool_entries = 0; 
  int ii;
  for (ii = 0; ii < 1023; ii++) {

    op_pool[ii].op_pool_next = &op_pool[ii+1]; 
    op_pool[ii].op_pool_id   = op_pool_entries++; 
    init_op(&op_pool[ii]); 
  }
  op_pool[ii].op_pool_next = op_pool_free_head; 
  op_pool[ii].op_pool_id   = op_pool_entries++;
  init_op(&op_pool[ii]); 
  op_pool_free_head = &op_pool[0]; 
}


Op *get_free_op(void)
{
  /* return a free op from op pool */ 

  if (op_pool_free_head == NULL || (free_op_num == 1)) {
    std::cout <<"ERROR! OP_POOL SIZE is too small!! " << endl; 
    std::cout <<"please check free_op function " << endl; 
    assert(1); 
    exit(1);
  }

  free_op_num--;
  assert(free_op_num); 

  Op *new_op = op_pool_free_head; 
  op_pool_free_head = new_op->op_pool_next; 
  assert(!new_op->valid); 
  init_op(new_op);
  active_op_num++; 
  return new_op; 
}

void free_op(Op *op)
{
  free_op_num++;
  active_op_num--; 
  op->valid = FALSE; 
  op->op_pool_next = op_pool_free_head;
  op_pool_free_head = op; 
}



/*******************************************************************/
/*  Data structure */
/*******************************************************************/

typedef struct pipeline_latch_struct {
  Op *op; /* you must update this data structure. */
  bool op_valid; 
   /* you might add more data structures. But you should complete the above data elements */ 
}pipeline_latch; 


typedef struct Reg_element_struct{
  bool valid;     //wendy
  uint64_t inst_id;  //wendy3
  // data is not needed 
  /* you might add more data structures. But you should complete the above data elements */ 
}REG_element; 

REG_element register_file[NUM_REG];


/*******************************************************************/
/* These are the functions you'll have to write.  */ 
/*******************************************************************/

void FE_stage(memory_c *main_memory);
void ID_stage();
void EX_stage(); 
void MEM_stage(memory_c *main_memory); // please modify MEM_stage function argument  /** NEW-LAB2 */
void WB_stage(); 

void init_registers() {			//wendy
	int i ;
	for (i=0; i < NUM_REG; i++) {
		register_file[i].valid = true;
		register_file[i].inst_id=0;      //wendy3
	}
}

/*******************************************************************/
/*  These are the variables you'll have to write.  */ 
/*******************************************************************/

bool sim_end_condition = FALSE;     /* please complete the condition. */ 
UINT64 retired_instruction = 0;    /* number of retired instruction. (only correct instructions) */ 
UINT64 cycle_count = 0;            /* total number of cycles */ 
UINT64 data_hazard_count = 0;  
UINT64 control_hazard_count = 0; 
UINT64 icache_miss_count = 0;      /* total number of icache misses. for Lab #2 and Lab #3 */ 
UINT64 dcache_hit_count = 0;      /* total number of dcache  misses. for Lab #2 and Lab #3 */    //L2ADD
UINT64 dcache_miss_count = 0;      /* total number of dcache  misses. for Lab #2 and Lab #3 */ 
UINT64 l2_cache_miss_count = 0;    /* total number of L2 cache  misses. for Lab #2 and Lab #3 */  
UINT64 dram_row_buffer_hit_count = 0; /* total number of dram row buffer hit. for Lab #2 and Lab #3 */   // NEW-LAB2
UINT64 dram_row_buffer_miss_count = 0; /* total number of dram row buffer hit. for Lab #2 and Lab #3 */   // NEW-LAB2
UINT64 store_load_forwarding_count = 0;  /* total number of store load forwarding for Lab #2 and Lab #3 */  // NEW-LAB2

uint64_t bpred_mispred_count = 0;  /* total number of branch mispredictions */  // NEW-LAB3
uint64_t bpred_okpred_count = 0;   /* total number of correct branch predictions */  // NEW-LAB3
uint64_t dtlb_hit_count = 0;       /* total number of DTLB hits */ // NEW-LAB3
uint64_t dtlb_miss_count = 0;      /* total number of DTLB miss */ // NEW-LAB3



//void init_register();    //L2MINUTES

list<Op*> WBLIST;  //wendy


typedef struct{              //wendy3
	bool valid;
	uint64_t inst_id;
}STRUCT_CF_STALL;
STRUCT_CF_STALL cf_stall;    //wendy3

TLB_MISS_ENUM tlb_miss=NO_TLB_MISS;//wendyxiaoyu, bool place here by broadcast() and mem_stage
					//0 no tlb miss, 1 tlb miss without mshr insert, 2 tbl miss with successful mshr insert
					// 3 pte retrieved from dram

bool perfect_dcache;
int dcache_latency;
bool enable_vmem;
uint64_t vmem_page_size;
bool use_bpred;                 //wendy3

//static int  dummy_count ;     //wendynotsure

pipeline_latch *MEM_latch;  
pipeline_latch *EX_latch;
pipeline_latch *ID_latch;
pipeline_latch *FE_latch;
UINT64 ld_st_buffer[LD_ST_BUFFER_SIZE]; /* this structure is deprecated. do not use */ 
UINT64 next_pc; 

Cache *data_cache;  // NEW-LAB2 

bpred *branchpred; // NEW-LAB3 (student need to initialize)
tlb *dtlb;        // NEW-LAB3 (student need to intialize)

/*******************************************************************/
/*  Print messages  */
/*******************************************************************/
void print_stats() {
  std::ofstream out((KNOB(KNOB_OUTPUT_FILE)->getValue()).c_str());
  /* Do not modify this function. This messages will be used for grading */ 
  out << "Total instruction: " << retired_instruction << endl; 
  out << "Total cycles: " << cycle_count << endl; 
  float ipc = (cycle_count ? ((float)retired_instruction/(float)cycle_count): 0 );
  out << "IPC: " << ipc << endl; 
  out << "Total I-cache miss: " << icache_miss_count << endl; 
  out << "Total D-cache miss: " << dcache_miss_count << endl; 
  out << "Total D-cache hit: " << dcache_hit_count << endl;
  out << "Total data hazard: " << data_hazard_count << endl;
  out << "Total control hazard : " << control_hazard_count << endl;   
  out << "Total DRAM ROW BUFFER Hit: " << dram_row_buffer_hit_count << endl; 
  out << "Total DRAM ROW BUFFER Miss: "<< dram_row_buffer_miss_count << endl; 
  out <<" Total Store-load forwarding: " << store_load_forwarding_count << endl; 

  // new for LAB3
  out << "Total Branch Predictor Mispredictions: " << bpred_mispred_count << endl;   
  out << "Total Branch Predictor OK predictions: " << bpred_okpred_count << endl;   
  out << "Total DTLB Hit: " << dtlb_hit_count << endl; 
  out << "Total DTLB Miss: " << dtlb_miss_count << endl; 

  out.close();
}

/*******************************************************************/
/*  Support Functions  */ 
/*******************************************************************/

bool get_op(Op *op)
{
  static UINT64 unique_count = 0; 
  Trace_op trace_op; 
  bool success = FALSE; 
  // read trace 
  // fill out op info 
  // return FALSE if the end of trace 


  //success = (gzread(g_stream, &trace_op, sizeof(Trace_op)) >0 );
	success = (gzread(g_stream, &trace_op, sizeof(Trace_op)) == sizeof(Trace_op)); //wendy from piazza say should change

  if (KNOB(KNOB_PRINT_INST)->getValue()) dprint_trace(&trace_op); 

  /* copy trace structure to op */ 
  if (success) { 
    copy_trace_op(&trace_op, op); 

    op->inst_id  = unique_count++;
    op->valid    = TRUE; 
  }
  return success; 
}
/* return op execution cycle latency */ 

int get_op_latency (Op *op) 
{
  assert (op->opcode < NUM_OP_TYPE); 
  return op_latency[op->opcode];
}

/* Print out all the register values */ 
void dump_reg() {
  for (int ii = 0; ii < NUM_REG; ii++) {
    std::cout << cycle_count << ":register[" << ii  << "]: V:" << register_file[ii].valid << endl; 
  }
}

void print_pipeline() {
  std::cout << "--------------------------------------------" << endl; 
  std::cout <<"cycle count : " << dec << cycle_count << " retired_instruction : " << retired_instruction << endl; 
  std::cout << (int)cycle_count << " FE: " ;
  if (FE_latch->op_valid) {
    Op *op = FE_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }
  std::cout << " ID: " ;
  if (ID_latch->op_valid) {
    Op *op = ID_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }
  std::cout << " EX: " ;
  if (EX_latch->op_valid) {
    Op *op = EX_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }

  std::cout << " MEM: " ;
  if (MEM_latch->op_valid) {
    Op *op = MEM_latch->op; 
    cout << (int)op->inst_id ;
  }
  else {
    cout <<"####";
  }
  cout << endl; 
  //  dump_reg();   
  std::cout << "--------------------------------------------" << endl; 
}

void print_heartbeat()
{
  static uint64_t last_cycle ;
  static uint64_t last_inst_count; 
  float temp_ipc = float(retired_instruction - last_inst_count) /(float)(cycle_count-last_cycle) ;
  float ipc = float(retired_instruction) /(float)(cycle_count) ;
  /* Do not modify this function. This messages will be used for grading */ 
  cout <<"**Heartbeat** cycle_count: " << cycle_count << " inst:" << retired_instruction << " IPC: " << temp_ipc << " Overall IPC: " << ipc << endl; 
  last_cycle = cycle_count;
  last_inst_count = retired_instruction; 
}
/*******************************************************************/
/*                                                                 */
/*******************************************************************/

bool run_a_cycle(memory_c *main_memory){    // please modify run_a_cycle function argument  /** NEW-LAB2 */

  for (;;) { 

	// cout << "print!!!" << endl;
    if (((KNOB(KNOB_MAX_SIM_COUNT)->getValue() && (cycle_count >= KNOB(KNOB_MAX_SIM_COUNT)->getValue())) || 
      (KNOB(KNOB_MAX_INST_COUNT)->getValue() && (retired_instruction >= KNOB(KNOB_MAX_INST_COUNT)->getValue())) ||  (sim_end_condition&&0==active_op_num))) {
        // please complete sim_end_condition 
        // finish the simulation 
        print_heartbeat(); 

        print_stats();
        return TRUE; 
    }
    cycle_count++; 
    if (!(cycle_count%5000)) {
      print_heartbeat(); 
    }




	main_memory->run_a_cycle();          // *NEW-LAB2   //can not go in!!!!!!

    WB_stage();
    MEM_stage(main_memory);  // please modify MEM_stage function argument  /** NEW-LAB2 */
    EX_stage();
    ID_stage();
    FE_stage(main_memory);
    if (KNOB(KNOB_PRINT_PIPE_FREQ)->getValue() && !(cycle_count%KNOB(KNOB_PRINT_PIPE_FREQ)->getValue())) print_pipeline();
  }
  return TRUE; 
}


/*******************************************************************/
/* Complete the following fuctions.  */
/* You can add new data structures and also new elements to Op, Pipeline_latch data structure */ 
/*******************************************************************/

void init_structures(memory_c * main_memory) // please modify init_structures function argument  /** NEW-LAB2 */
{
  init_op_pool(); 
  init_op_latency();
  /* please initialize other data stucturs */ 
  /* you must complete the function */

  perfect_dcache=KNOB(KNOB_PERFECT_DCACHE)->getValue();    //wendy3
  dcache_latency=KNOB(KNOB_DCACHE_LATENCY)->getValue();
  enable_vmem=KNOB(KNOB_ENABLE_VMEM)->getValue();
  vmem_page_size=KNOB(KNOB_VMEM_PAGE_SIZE)->getValue();
  use_bpred=KNOB(KNOB_USE_BPRED)->getValue();

  branchpred=bpred_new(bpred_type(KNOB(KNOB_BPRED_TYPE)->getValue()),KNOB(KNOB_BPRED_HIST_LEN)->getValue()); //wendy3
  dtlb=tlb_new(KNOB(KNOB_TLB_ENTRIES)->getValue());

  //dummy_count=-1;  //wendynotsure

  init_latches();
  init_registers();				//wendy
  main_memory->init_mem();			//wendy
  data_cache=new Cache();   //wendy

  cf_stall.valid=FALSE;   //wendy3
  cf_stall.inst_id=0;

  cache_init(data_cache,KNOB(KNOB_DCACHE_SIZE)->getValue(),KNOB(KNOB_BLOCK_SIZE)->getValue(),KNOB(KNOB_DCACHE_WAY)->getValue(),"dcache"); //wendy
  //init_register();
}

void WB_stage()
{
  /* You MUST call free_op function here after an op is retired */
  /* you must complete the function */
	/* if (MEM_latch->op_valid==TRUE){
	    	if (MEM_latch->op->dst>-1){
	    		register_file[MEM_latch->op->dst].valid=TRUE;
	    	}
	    	free_op(MEM_latch->op);
	    	MEM_latch->op_valid=FALSE;
	    	MEM_latch->op=NULL;
	    	retired_instruction++;

			}
	*/

	list <Op *>::iterator it;
	   if (MEM_latch->op_valid==TRUE){
			 WBLIST.push_back(MEM_latch->op);   //stop here
		     MEM_latch->op_valid=FALSE;
		     MEM_latch->op=NULL;
	   }
	  // cout << "print!!!" << endl;
	if(!WBLIST.empty()){
		// cout << "print!!!" << endl;    //   no print here
	  for(it=WBLIST.begin();it!=WBLIST.end();it++){

		   if ((*it)->dst>-1){

			   if(!register_file[(*it)->dst].valid){
			         if(register_file[(*it)->dst].inst_id==(*it)->inst_id)
				          register_file[(*it)->dst].valid=true;
			   }
		   }
		   //cout << "print!!!" << endl;
		   if((*it)->cf_type!=NOT_CF){
		   		if(!use_bpred)
		   			cf_stall.valid=FALSE;
		   		else if((*it)->cf_type==CF_CBR){
		   			if(cf_stall.valid==TRUE){
		   				if((*it)->inst_id==cf_stall.inst_id)
		   				   cf_stall.valid=false;

		   			}
		   		}
		   	}
//cout<<"print!!!"<<endl;
		   free_op(*it);

		   it=WBLIST.erase(it);
	                                                       //less one print here
		   retired_instruction++;

	  }

	 }

}

void MEM_stage(memory_c *main_memory)  // please modify MEM_stage function argument  /** NEW-LAB2 */
{
    static int count = -1;

	static bool hit = FALSE;
	static bool Mr_pipeline_stall = FALSE;

	//static bool tlb_cause_stall = false ;// pipeline stall caused by tlb miss
	static uint64_t addr,vpn,page_offset,pfn,pte;
	//static int  dummy_count = -1;    //count for TLB related instruction, -1 for already get physical address; others for caceh cycle
	//cout<<"dummy_count init="<<dummy_count<<endl;
	static Op * dummy_op=NULL;

	//cout << "print" << endl;
	if (count== -1  && EX_latch->op_valid ) {
		  //previous instruction is done new instr fetch
		   //cout<<"could process"<<endl;

		   if (EX_latch->op->mem_type==NOT_MEM){
               // cout<<"not mem op"<<endl;
				MEM_latch->op=EX_latch->op;
				MEM_latch->op_valid=TRUE;
				EX_latch->op_valid=FALSE;
				EX_latch->op=NULL;
				count=-1;

				return;

			}



			else {						//MEM access delay obtained from knob
				count = KNOB(KNOB_DCACHE_LATENCY)->getValue();

			}
	//	cout << "print" << endl;



		   if(enable_vmem && NOT_MEM != EX_latch->op->mem_type){
		   			if(EX_latch->op->mem_type==MEM_ST)
		   				addr=EX_latch->op->st_vaddr;
		   			else if(EX_latch->op->mem_type==MEM_LD)
		   				addr=EX_latch->op->ld_vaddr;

		   			vpn=addr/vmem_page_size;
		   			page_offset=addr%vmem_page_size;
		   			//start to translate address
		   			if(tlb_access(dtlb,vpn,0,&pfn)){
		   				//successful translation, substitute with physical address
		   				if(EX_latch->op->mem_type==MEM_ST){
		   					EX_latch->op->st_vaddr=pfn*vmem_page_size|page_offset;
		   				}
		   				else if(EX_latch->op->mem_type==MEM_LD){
		   					EX_latch->op->ld_vaddr=pfn*vmem_page_size|page_offset;
		   				}

		   				dtlb_hit_count++;
		   				tlb_miss=NO_TLB_MISS;
		   			}
		   			else{
		   				//translation failed, look up in dcache
		   				dtlb_miss_count++;
		   				tlb_miss=DCACHE_COUNTDOWN;
		   			}
		   		}
		   	else{
		   			tlb_miss=NO_TLB_MISS;
		   		}
		   	}


		   	switch(tlb_miss){
		   	case NO_TLB_MISS:
		   		break;
		   	case DCACHE_COUNTDOWN:
		   		if(count!=0){
		   			count--;
		   		}
		   		if(count==0){
		   			pte=vmem_get_pteaddr(vpn,0);
		   			if(dcache_access(pte)){
		   				pfn=vmem_vpn_to_pfn(vpn,0);
		   				tlb_install(dtlb,vpn,0,pfn);
		   				if(EX_latch->op->mem_type==MEM_ST)
		   					EX_latch->op->st_vaddr=pfn*vmem_page_size|page_offset;
		   				else if(EX_latch->op->mem_type==MEM_LD)
		   					EX_latch->op->ld_vaddr=pfn*vmem_page_size|page_offset;

		   				if(perfect_dcache)
		   					count=0;
		   				else
		   					count=KNOB(KNOB_DCACHE_LATENCY)->getValue();
		   				tlb_miss=NO_TLB_MISS;
		   				dcache_hit_count++;
		   				return;
		   			}
		   			else{
		   				dummy_op=(Op *)calloc(1,sizeof(Op));
		   				dummy_op->valid=1;
		   				dummy_op->opcode=OP_DUMMY;
		   				dummy_op->mem_type=MEM_LD;
		   				dummy_op->mem_read_size=1;
		   				dummy_op->instruction_addr=addr;
		   				dummy_op->ld_vaddr=pte;
		   				if(main_memory->insert_mshr(dummy_op)){
		   					tlb_miss=SUCCESSFUL_MSHR_INSERT;
		   				}
		   				else{
		   					free(dummy_op);
		   					tlb_miss=FAILED_MSHR_INSERT;
		   				}
		   				dcache_miss_count++;

		   				return;
		   			}
		   		}
		   		else
		   			return;
		   		break;
		   	case FAILED_MSHR_INSERT:
		   		dummy_op=(Op *)calloc(1,sizeof(Op));
		   		dummy_op->valid=1;
		   		dummy_op->opcode=OP_DUMMY;
		   		dummy_op->mem_type=MEM_LD;
		   		dummy_op->mem_read_size=1;
		   		dummy_op->instruction_addr=addr;
		   		dummy_op->ld_vaddr=pte;
		   		if(main_memory->insert_mshr(dummy_op)){
		   			tlb_miss=SUCCESSFUL_MSHR_INSERT;
		   		}
		   		else{
		   			free(dummy_op);


		   		}
		   		return;
		   	case SUCCESSFUL_MSHR_INSERT:
		   		return;
		   	case MSHR_SERVICED:
		   		tlb_miss=TLB_SERVICED;
		   		return;
		   	case TLB_SERVICED:
		   		pfn=vmem_vpn_to_pfn(vpn,0);
		   		if(EX_latch->op->mem_type==MEM_ST)
		   			EX_latch->op->st_vaddr=pfn*vmem_page_size|page_offset;
		   		else if(EX_latch->op->mem_type==MEM_LD)
		   			EX_latch->op->ld_vaddr=pfn*vmem_page_size|page_offset;
		   		else

		   		if(perfect_dcache)
		   			count=0;
		   		else
		   			count=KNOB(KNOB_DCACHE_LATENCY)->getValue();
		   		tlb_miss=NO_TLB_MISS;
		   		break;
		   	default:

		   		break;
		   	}
		 //  	cout << "print" << endl;
/*
	if(count!=-1){        //memory op enable vmem
			if(enable_vmem && NOT_MEM != EX_latch->op->mem_type){   //memory op and use vmen
				if(EX_latch->op->mem_type==MEM_ST)
					addr=EX_latch->op->st_vaddr;
				else if(EX_latch->op->mem_type==MEM_LD)
					addr=EX_latch->op->ld_vaddr;
				vpn=addr/vmem_page_size;
				page_offset=addr%vmem_page_size;
						//start to translate address
				//dummy_count = KNOB(KNOB_DCACHE_LATENCY)->getValue();        //mem op and use enable vmem
				if(tlb_access(dtlb,vpn,0,&pfn)){    //tlb hit, get physical address
					//cout<<"print!!"<<endl;  //never print!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
					dummy_count = -1;
					dtlb_hit_count++;
					if(EX_latch->op->mem_type==MEM_ST){
					        EX_latch->op->st_vaddr=pfn*vmem_page_size|page_offset;
					}
					else if(EX_latch->op->mem_type==MEM_LD){
							EX_latch->op->ld_vaddr=pfn*vmem_page_size|page_offset;
					}
				}

				else{                                    //tlb miss
					//tlb_cause_stall=true;
					//dtlb_miss_count++;
					//cout<<"tlb miss="<<dtlb_miss_count<<endl;
					if(dummy_count>0){
						dummy_count--;
						//cout<<"dummy_count="<<dummy_count<<endl;       //
					}
					if(dummy_count==0){
					  //if (tlb_cause_stall==false){
						//cout<<"0=dummycount"<<endl;  //show here how could 4 t
						dtlb_miss_count++;
						pte=vmem_get_pteaddr(vpn,0);
						if(dcache_access(pte)){  //dcache access success
							//cout<<"dcache TLB hit"<<endl;
							dcache_hit_count++;
							pfn=vmem_vpn_to_pfn(vpn,0);
							tlb_install(dtlb,vpn,0,pfn);
							if(EX_latch->op->mem_type==MEM_ST)
								EX_latch->op->st_vaddr=pfn*vmem_page_size|page_offset;
							else if(EX_latch->op->mem_type==MEM_LD)
								EX_latch->op->ld_vaddr=pfn*vmem_page_size|page_offset;
						}
					    else{     //dcache miss
						    dcache_miss_count++;  //problem is counting
						   // cout<<"dcache_miss_couttlb"<<dcache_miss_count<<endl;
						    dummy_op=(Op *)calloc(1,sizeof(Op));
						    dummy_op->valid=1;
						    dummy_op->opcode=OP_DUMMY;
						    dummy_op->mem_type=MEM_LD;
						    dummy_op->mem_read_size=1;
						    dummy_op->instruction_addr=addr;
						    dummy_op->ld_vaddr=pte;

						    //cout<<"!!pr!!!!"<<endl;                             //trap here

						    main_memory->insert_mshr(dummy_op);      //mshr insert
						    return;
                          //  cout<<"dummy_count"<<dummy_count<<endl;
					    }
				      //}  flage tlb_cause_stall
				    }

				}
		    }

  }//count!=-1*/




	//this is lab2 need add tlb_miss_stall flage here
	//then detect whether new request could be served.
	//if (dummy_count!=-1)
		//return;
  //if(dummy_count==-1){    //already get physical address
	//cout<<"in stage2"<<endl;
	//cout<<"stage2dummycount="<<dummy_count<<endl;
	  if (count!=-1) {	//instruction is still process
	  				if (count>0) {			//
	  					count--;
	  					//cout<<"count="<<count<<endl;
	  				}
	  				if (count==0) {				//delayed undemanded or finished

	  				  /* if (perfect_dcache) {
	  				    	MEM_latch->op=EX_latch->op;
	  				    	MEM_latch->op_valid=TRUE;
	  				    	EX_latch->op_valid=FALSE;
	  				    	EX_latch->op=NULL;
	  						count = -1;
	  						return;
	  						}*/
	  					if (Mr_pipeline_stall==FALSE) {	// no stall
	  						/*if (KNOB(KNOB_PERFECT_DCACHE)->getValue()) {	//NOT_MEM instruction needs no access delay
	  											MEM_latch->op=EX_latch->op;
	  											MEM_latch->op_valid=TRUE;
	  											EX_latch->op_valid=FALSE;
	  											EX_latch->op=NULL;
	  											count=-1;

	  											return;
	  						}*/
	  					 if (EX_latch->op->mem_type==MEM_ST) {		//cache hit
	  						    hit = dcache_access(EX_latch->op->st_vaddr);
	  						          if (hit==TRUE){
	  							         dcache_hit_count++;
	  							         MEM_latch->op=EX_latch->op;
	  							         MEM_latch->op_valid=TRUE;
	  							         EX_latch->op_valid=FALSE;
	  							       }
	  						          else
	  						          	dcache_miss_count++;
	  						     }
	  						else  if (EX_latch->op->mem_type==MEM_LD){   //cache hit
	  						        hit = dcache_access(EX_latch->op->ld_vaddr);
	  								if (hit==TRUE){
	  								      dcache_hit_count++;
	  									  MEM_latch->op=EX_latch->op;
	  									  MEM_latch->op_valid=TRUE;
	  									  EX_latch->op_valid=FALSE;
	  								  }
	  							else
	  								dcache_miss_count++;           //cache miss
	  						}


	  						if (hit==FALSE) {				// cache miss check st/ld forwarding and piggy back and mshr insert
	  							if (main_memory->store_load_forwarding(EX_latch->op)) {
	  								store_load_forwarding_count++;
	  								MEM_latch->op=EX_latch->op;
	  								MEM_latch->op_valid=TRUE;
	  								EX_latch->op_valid=FALSE;
	  								hit = TRUE;
	  							}
	  							else if (main_memory->check_piggyback(EX_latch->op)) {

	  								hit = TRUE;
	  							}
	  							else if (main_memory->insert_mshr(EX_latch->op)) {   //mishr could insert new entity
	  								hit = TRUE;
	  							}
	  							else {				//mshr full can not insert new entity
	  								hit = FALSE;
	  								Mr_pipeline_stall = TRUE;   //pipeline stall
	  							}
	  						}
	  						if (hit==TRUE) {
	  							EX_latch->op = NULL;
	  							EX_latch->op_valid = FALSE;
	  							//MEM_latch->op_valid=TRUE;
	  							count = -1;
	  							Mr_pipeline_stall = FALSE;
	  						}
	  					}
	  					else if (main_memory->insert_mshr(EX_latch->op)) {  //pipeline stall but now could insert new mshr entity
	  						EX_latch->op = NULL;
	  						EX_latch->op_valid = FALSE;
	  						Mr_pipeline_stall = FALSE;
	  						count = -1;
	  					}
	  					else
	  						Mr_pipeline_stall = TRUE;
	  				}
	  			}

//  }// for dummy_count

}


void EX_stage()
{
  /* you must complete the function */
	//cout<<"dummy_count_to_ex"<<dummy_count<<endl;
   static int EX_cycle_count=-1;  //count clock cycles the instruction needs in EX stage

    if(ID_latch->op_valid==TRUE){
    	if(EX_cycle_count == -1){
    		EX_cycle_count=get_op_latency(ID_latch->op);
    	}
    	EX_cycle_count--;
    	//cout << "print" << endl;
        if (EX_cycle_count==0){
        	//cout << "print" << endl;
        	if(EX_latch->op_valid==FALSE){
        		EX_latch->op=ID_latch->op;
        		EX_latch->op_valid=TRUE;
        		ID_latch->op_valid=FALSE;
        		ID_latch->op=NULL;

        	}
        	EX_cycle_count = -1;
        }

    }


}

void ID_stage()
{
  /* you must complete the function */


	//wendy
	  static Op* ID_op=NULL;
		if(!ID_op&&FE_latch->op_valid){
			ID_op=FE_latch->op;
		}
		if(ID_op){
           //printf("blah0\n");
			if(!ID_latch->op_valid){
				if(NOT_CF!=ID_op->cf_type){
					control_hazard_count++;
				}
				if((ID_op->src[0]!=-1&& !register_file[ID_op->src[0]].valid)||(ID_op->src[1]!=-1&&!register_file[ID_op->src[1]].valid)){
					data_hazard_count++;
           //printf("blah1\n");
				}
				else{
           //printf("blah2\n");
					if(ID_op->dst!=-1){
						register_file[ID_op->dst].inst_id=ID_op->inst_id;
						register_file[ID_op->dst].valid=FALSE;
					}
					ID_latch->op=ID_op;
					ID_latch->op_valid=TRUE;
					ID_op=NULL;
					FE_latch->op_valid=FALSE;
					FE_latch->op=NULL;
				}
			}
		}



}


void FE_stage(memory_c *main_memory)
{
  /* only part of FE_stage function is implemented */
  /* please complete the rest of FE_stage function */
	//if ((op->cf_type) >= CF_BR)

/*if(FE_latch->op_valid==TRUE&&FE_latch->op->cf_type>= CF_BR)  // stop fetching in case of br
			return;
if(ID_latch->op_valid==TRUE&&ID_latch->op->cf_type>= CF_BR)
			return;
if(EX_latch->op_valid==TRUE&&EX_latch->op->cf_type>= CF_BR)
			return;
if(MEM_latch->op_valid==TRUE&&MEM_latch->op->cf_type>= CF_BR)
			return;
*/


int predict_result;
if(cf_stall.valid==true) {
	return;
}

if (FE_latch->op_valid==FALSE){
	 FE_latch->op=get_free_op();
	 //if ((FE_latch->op->cf_type) >= CF_BR){
	//	 return;
	 //}

	 if (get_op(FE_latch->op)==TRUE){
		 FE_latch->op_valid=TRUE; //get op success
		 if(FE_latch->op->cf_type==CF_CBR){//deal with branch predict    //wendy3
		 				if(use_bpred){
		 					predict_result = bpred_access(branchpred,FE_latch->op->instruction_addr);
		 					bpred_update(branchpred,FE_latch->op->instruction_addr,predict_result,FE_latch->op->actually_taken);
		 					if(predict_result==FE_latch->op->actually_taken){
		 						bpred_okpred_count++;
		 						cf_stall.valid=FALSE;
		 					}
		 					else{
		 						bpred_mispred_count++;
		 						cf_stall.valid=TRUE;                         //mispredicted pipeline stall
		 						cf_stall.inst_id=FE_latch->op->inst_id;
		 					}
		 				}
		 				else
		 					cf_stall.valid=TRUE;     //always stall if do not use predict
		 }

	 }

	 else{
		 free_op (FE_latch->op);

		 FE_latch->op_valid=FALSE;
		 FE_latch->op=NULL;
		 if(!FE_latch->op_valid && !ID_latch->op_valid && !EX_latch->op_valid && !MEM_latch->op_valid && WBLIST.empty() && main_memory->m_mshr.empty()) { // vinson -- Add conditions for MSHR Too
		 		 		 sim_end_condition=TRUE;
		 }
	 }
  }
  //   next_pc = pc + op->inst_size;  // you need this code for building a branch predictor
//cout << "FE: op inst_id is: "<< FE_latch->op->mem_type<<endl;


}

/*void init_register()
{
	int i;
	for (i=0;i<NUM_REG;i++){
		register_file[i].valid=TRUE;
	}
}*/

void  init_latches()
{
  MEM_latch = new pipeline_latch();
  EX_latch = new pipeline_latch();
  ID_latch = new pipeline_latch();
  FE_latch = new pipeline_latch();

  MEM_latch->op = NULL;
  EX_latch->op = NULL;
  ID_latch->op = NULL;
  FE_latch->op = NULL;

  /* you must set valid value correctly  */ 
  MEM_latch->op_valid = false;
  EX_latch->op_valid = false;
  ID_latch->op_valid = false;
  FE_latch->op_valid = false;

}

bool icache_access(ADDRINT addr) {    /** please change uint32_t to ADDRINT NEW-LAB2 */

  /* For Lab #1, you assume that all I-cache hit */     
  bool hit = FALSE; 
  if (KNOB(KNOB_PERFECT_ICACHE)->getValue()) hit = TRUE; 
  return hit; 
}



bool dcache_access(ADDRINT addr) { /** please change uint32_t to ADDRINT NEW-LAB2 */
  /* For Lab #1, you assume that all D-cache hit */     
  /* For Lab #2, you need to connect cache here */   // NEW-LAB2 
  bool hit = FALSE;
  if (KNOB(KNOB_PERFECT_DCACHE)->getValue()) hit = TRUE; 
  else {						//wendy
  		hit = cache_read(data_cache, addr);
  	}
  return hit; 
  }

  
// NEW-LAB2 
void dcache_insert(ADDRINT addr)  // NEW-LAB2 
{                                 // NEW-LAB2 
  /* dcache insert function */   // NEW-LAB2 
  cache_insert(data_cache, addr) ;   // NEW-LAB2 
 
}                                       // NEW-LAB2 

void broadcast_rdy_op(Op* op)             // NEW-LAB2 
{                                          // NEW-LAB2 
  /* you must complete the function */     // NEW-LAB2 
  // mem ops are done.  move the op into WB stage   // NEW-LAB2 

	/*MEM_latch->op=op;
	MEM_latch->op_valid=TRUE;*/
	WBLIST.push_back(op);

	//cout << "print!!!" << endl;
}      // NEW-LAB2 



/* utility functions that you might want to implement */     // NEW-LAB2 
int64_t get_dram_row_id(ADDRINT addr)    // NEW-LAB2 
{  // NEW-LAB2 
 // NEW-LAB2 
/* utility functions that you might want to implement */     // NEW-LAB2 
/* if you want to use it, you should find the right math! */     // NEW-LAB2 
/* pleaes carefull with that DRAM_PAGE_SIZE UNIT !!! */     // NEW-LAB2 
  // addr >> 6;   // NEW-LAB2 
return addr / 1024 / KNOB(KNOB_DRAM_PAGE_SIZE)->getValue();
  //return 2;   // NEW-LAB2
}  // NEW-LAB2 

int get_dram_bank_id(ADDRINT addr)  // NEW-LAB2 
{  // NEW-LAB2 
 // NEW-LAB2 
/* utility functions that you might want to implement */     // NEW-LAB2 
/* if you want to use it, you should find the right math! */     // NEW-LAB2 

  // (addr >> 6);   // NEW-LAB2 
return (addr / 1024 / KNOB(KNOB_DRAM_PAGE_SIZE)->getValue())
				% KNOB(KNOB_DRAM_BANK_NUM)->getValue();
  //return 1;   // NEW-LAB2
}  // NEW-LAB2 
