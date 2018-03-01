#include "common.h"
#include "sim.h"
#include "trace.h" 
#include "memory.h" 
#include "knob.h"
#include "all_knobs.h"

#include <stdlib.h>
#include "vmem.h"   //wendy3

/*******************************************************************/
/* Memory Related frame */ 
/*******************************************************************/
/* You do not need to change the code in this part                 */ 

extern UINT64 dram_row_buffer_hit_count;			//wendy
extern UINT64 dram_row_buffer_miss_count;

extern uint64_t vmem_page_size;          //wendy3
extern tlb *dtlb;                     //wendy3

extern TLB_MISS_ENUM tlb_miss;//wendyxiaoyu,bool place by broadcast() and mem_stage
					//0 no tlb miss, 1 tlb miss without mshr insert, 2 tbl miss with successful mshr insert
					// 3 pte retrieved from dram



void memory_c::dprint_queues(void) 
{
  // traverse the queue and print out all the values inside the queue 

  /* print out mshr entries */ 
  
  list<m_mshr_entry_s *>::const_iterator cii; 
  
  /* do not modify print MSHR print statement here. These will be used for grading */ 

  cout <<"** cycle_count: " << cycle_count << endl; 
  cout <<"***MSHR entry print outs: mshr_size: " <<  m_mshr.size() << endl; 

   for (cii= m_mshr.begin() ; cii != m_mshr.end(); cii++) {
    
    int jj = 0; 
    cout << "mshr_entry_num: " << jj;
    jj++; 

    m_mshr_entry_s* entry = (*cii);
    
    if (entry->valid) {
      mem_req_s *req = entry->m_mem_req; 
      
      cout <<" mem_req_id: " << req->m_id; 
      cout <<" mem_addr: " << req->m_addr; 
      cout <<" size: " << (int)req->m_size; 
      cout <<" insert_time: " << entry->insert_time;
      cout <<" rdy_cycle: " << req->m_rdy_cycle; 
      cout <<" dirty: " << req->m_dirty; 
      cout <<" done: " << req->m_done; 
      cout <<" state: " << req->m_state; 
      cout <<" type:  " << req->m_type; 
      cout <<" ops_size: " << entry->req_ops.size(); 
      
      if (entry->req_ops.size()) {
	list<Op *>::const_iterator cii; 
	int kk = 0; 
	for (cii = entry->req_ops.begin() ; cii != entry->req_ops.end(); cii++) {
	  
	  Op *m_op = (*cii);
	  
	  if(m_op->mem_type == MEM_LD) {
	    printf("op[%d]:LD id: %lu ", kk, (uint64_t)m_op->inst_id);
	  }
	  else 
	    printf("op[%d]:ST id: %lu", kk, (uint64_t)m_op->inst_id);
	  	  
	}
	kk++;

      }
     
      cout << endl; 
    }
  }
  
  // print queues 

  
  cout <<"***DRAM_IN_QUEUE entry print outs ****" << endl; 
  
  list<mem_req_s *>::const_iterator cqii; 
  for (cqii = dram_in_queue.begin() ; cqii != dram_in_queue.end(); cqii++) {
    mem_req_s *req = *cqii; 
    
    cout << " req_id: " << req->m_id; 
    cout << " addr: " << req->m_addr; 
    cout << " rdy_cycle: " << req->m_rdy_cycle; 
    cout <<" state: " << req->m_state; 
    cout <<" type:  " << req->m_type; 
    cout << " ||| " ; 
  }
  
  cout << endl; 
  // end printing dram in queue 
  

  // print dram scheduler queues 
  
  list<mem_req_s *>::const_iterator clii; 
  
  cout <<"***DRAM_SCH_QUEUE entry print outs ****" << endl; 

  for (int bb = 0; bb < m_dram_bank_num; bb++) { 
    cout <<"***DRAM_SCH_QUEUE BANK[" << bb << "]" << endl; 
    
    for (clii = dram_bank_sch_queue[bb].begin(); 
	 clii != dram_bank_sch_queue[bb].end(); 
	 clii++) { 
      
      mem_req_s *req = *clii; 
      
      cout << " req_id: " << req->m_id; 
      cout << " addr: " << req->m_addr; 
      cout << " rdy_cycle: " << req->m_rdy_cycle; 
      cout <<" state: " << req->m_state; 
      cout <<" type:  " << req->m_type; 
      cout << " ||| " ; 
    
    }
    cout << endl; 
  }
  cout << endl; 
  // ending print dram scheduler 

  
  // print dram out queue 
  
  cout <<"***DRAM_OUT_QUEUE entry print outs ****" << endl; 
  for (cqii= dram_out_queue.begin() ; cqii != dram_out_queue.end(); cqii++) {
    mem_req_s *req = *cqii; 
    
    cout << " req_id: " << req->m_id; 
    cout << " addr: " << req->m_addr; 
    cout << " rdy_cycle: " << req->m_rdy_cycle; 
    cout <<" state: " << req->m_state; 
    cout <<" type:  " << req->m_type; 
    cout << " ||| " ; 
      
  }
  
  cout << endl; 
  
  
}

/*******************************************************************/
/* print dram status. debugging help function 
/*******************************************************************/

void memory_c::dprint_dram_banks()
{
  
  cout << " DRAM_BANK_STATUS cycle_count "<< cycle_count << endl; 

  for (int ii = 0; ii <m_dram_bank_num; ii++) {
    printf("bank_num[%d]: row_id:%d rdy_cycle:%lu \n",
	   ii, (int64_t)dram_bank_open_row[ii], (uint64_t)dram_bank_rdy_cycle[ii]); 
  }
  
}


/*******************************************************************/
/* Initialize the memory related data structures                   */
/*******************************************************************/

void memory_c::init_mem()
{
  /* For Lab #2, you do not need to modify this code */

  /* you can add other code here if you want */ 
  

  /* init mshr */ 
  m_mshr_size = KNOB(KNOB_MSHR_SIZE)->getValue(); 
  m_dram_bank_num = KNOB(KNOB_DRAM_BANK_NUM)->getValue(); 
  m_block_size = KNOB(KNOB_BLOCK_SIZE)->getValue();

  
  for (int ii = 0 ; ii < m_mshr_size; ii++) 
  {

    m_mshr_entry_s* entry = new m_mshr_entry_s; 
    entry->m_mem_req = new mem_req_s;  // create a memory rquest data structure here 
    
    entry->valid = false; 
    entry->insert_time = 0; 
    m_mshr_free_list.push_back(entry); 
  }
  
  /* init DRAM scheduler queues */ 

  dram_in_queue.clear();
  dram_out_queue.clear();
  
  dram_bank_sch_queue = new list<mem_req_s*>[m_dram_bank_num]; 
  dram_bank_open_row = new int64_t[m_dram_bank_num];
  dram_bank_rdy_cycle = new uint64_t [m_dram_bank_num];


  for(int ii=0;ii<m_dram_bank_num;ii++)
  {
	dram_bank_rdy_cycle[ii]=0;
    dram_bank_open_row[ii]=-1;
  }

  
}

/*******************************************************************/
/* free MSHR entry (init the corresponding mshr entry  */ 
/*******************************************************************/

void memory_c::free_mshr_entry(m_mshr_entry_s *entry)
{
  entry->valid = false; 
  entry->insert_time = 0; 
  m_mshr_free_list.push_back(entry); 

}

/*******************************************************************/
/* This function is called every cycle                             */ 
/*******************************************************************/

void memory_c::run_a_cycle() 
{

	//cout << "print!!!" << endl;
  if (KNOB(KNOB_PRINT_MEM_DEBUG)->getValue()) {
    dprint_queues();
    dprint_dram_banks();
  }
  //cout << "print!!!" << endl;
  /* This function is called from run_a_cycle() every cycle */ 
  /* You do not add new code here */ 
  /* insert D-cache/I-cache (D-cache for only Lab #2) and wakes up instructions */ 
  fill_queue(); 
  //cout << "print!!!" << endl;
  /* move memory requests from dram to cache and MSHR*/ /* out queue */ 
  send_bus_out_queue(); 
  //cout << "print!!!" << endl;
  /* memory requests are scheduled */ 
  dram_schedule(); 

  /* memory requests are moved from bus_queue to DRAM scheduler */
  push_dram_sch_queue();
  
  /* new memory requests send from MSRH to in_bus_queue */ 
  send_bus_in_queue(); 

}

/*******************************************************************/
/* get a new mshr entry 
/*******************************************************************/

m_mshr_entry_s* memory_c::allocate_new_mshr_entry(void)
{
  
  if (m_mshr_free_list.empty())
    return NULL; 
  
  m_mshr_entry_s* entry = m_mshr_free_list.back(); 
  m_mshr_free_list.pop_back(); 
  m_mshr.push_back(entry); 
  
  return entry; 
  
 }






/* insert memory request into the MSHR, if there is no space return false */ 
/*******************************************************************/
/* memory_c::insert_mshr
/*******************************************************************/
bool memory_c::insert_mshr(Op *mem_op)
{

  bool insert=false; 
 
  //  step 1. create a new memory request 
  m_mshr_entry_s * entry = allocate_new_mshr_entry();
 
  //  step 2. no free entry means no space return ; 
  if (!entry) return false; // cannot insert a request into the mshr; 

  // insert op to into the mshr 
  entry->req_ops.push_back(mem_op);

    // step 3. initialize the memory request here 
  mem_req_s *t_mem_req = entry->m_mem_req; 
  
  if(mem_op->opcode==OP_DUMMY){

  		t_mem_req->m_type = MRT_DUMMY;
  		t_mem_req->m_addr = mem_op->ld_vaddr;
  		t_mem_req->m_size = mem_op->mem_read_size;
  	}

  else if (mem_op->mem_type == MEM_LD ) {
    t_mem_req->m_type = MRT_DFETCH; 
    t_mem_req->m_addr = mem_op->ld_vaddr; 
    t_mem_req->m_size = mem_op->mem_read_size;
  }
  else {
    t_mem_req->m_type = MRT_DSTORE; 
    t_mem_req->m_addr = mem_op->st_vaddr; 
    t_mem_req->m_size = mem_op->mem_write_size;
  }
    t_mem_req->m_rdy_cycle = 0; 
    t_mem_req->m_dirty = false; 
    t_mem_req->m_done = false; 
    t_mem_req->m_id = ++m_unique_m_count; 
    t_mem_req->m_state = MEM_NEW; 
    
    entry->valid  = true; 
    entry->insert_time = cycle_count; 
    
    return true; 
}


/*******************************************************************/
/* searching for matching mshr entry using memory address 
/*******************************************************************/


m_mshr_entry_s * memory_c::search_matching_mshr(ADDRINT addr) {
  
  list<m_mshr_entry_s *>::const_iterator cii; 
  
  for (cii= m_mshr.begin() ; cii != m_mshr.end(); cii++) {
    
    m_mshr_entry_s* entry = (*cii);
    if (entry->valid) {
      mem_req_s *req = entry->m_mem_req; 
      if (req) {
	if ((req->m_addr)/m_block_size == (addr/m_block_size)) 
	  return entry; 
      }
    }
  }
  return NULL;
}

/*******************************************************************/
/* searching for matching mshr entry using memory address and return the iterator 
/*******************************************************************/

list<m_mshr_entry_s*>::iterator memory_c::search_matching_mshr_itr(ADDRINT addr) {
  
  list<m_mshr_entry_s *>::iterator cii; 
  
  for (cii= m_mshr.begin() ; cii != m_mshr.end(); cii++) {
    
    m_mshr_entry_s* entry = (*cii);
    if (entry->valid) {
      mem_req_s *req = entry->m_mem_req; 
      if (req) {
	if ((req->m_addr)/m_block_size == (addr/m_block_size)) 
	  return cii; 
      }
    }
  }
  return cii;
}



/*******************************************************************/
/*  search MSHR entries and find a matching MSHR entry and piggyback 
/*******************************************************************/

bool memory_c::check_piggyback(Op *mem_op)
{
  bool match = false; 
  ADDRINT addr;
  if (mem_op->mem_type == MEM_LD) 
    addr = mem_op->ld_vaddr;
  else 
    addr = mem_op->st_vaddr; 
  
  m_mshr_entry_s *entry = search_matching_mshr(addr);
  
  if(!entry)
  {
    return false;
  } else 
  {
    entry->req_ops.push_back(mem_op); 
    return true;
  }
}



/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*****    COMPLETE the following functions               ***********/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/




/*******************************************************************/
/* send a request from mshr into in_queue 
/*******************************************************************/

void memory_c::send_bus_in_queue() {
 
   /* For Lab #2, you need to fill out this function */ 
	list<m_mshr_entry_s *>::iterator it;

		if (!m_mshr.empty()){

		    for(it=m_mshr.begin();it!=m_mshr.end();++it){
				if((*it)->valid){

				    if(MEM_NEW==(*it)->m_mem_req->m_state){
					    (*it)->m_mem_req->m_state=MEM_DRAM_IN;
					    dram_in_queue.push_back((*it)->m_mem_req);
					    break;                                     //can only send one addr every time
				    }
				}
			}
		}
  // *END**** This is for the TA*/

}





/*******************************************************************/
/*  push_dram_sch_queue()
/*******************************************************************/

void memory_c::push_dram_sch_queue()
{


  /* For Lab #2, you need to fill out this function */

  mem_req_s * temp=NULL;
  int addr;
  if (!dram_in_queue.empty()){
		addr=get_dram_bank_id(dram_in_queue.front()->m_addr);
		temp=dram_in_queue.front();
		dram_bank_sch_queue[addr].push_back(temp);
		dram_in_queue.pop_front();                                //

  }

}


/*******************************************************************/
/* search from dram_schedule queue and scheule a request 
/*******************************************************************/

 void memory_c::dram_schedule() {

   /* For Lab #2, you need to fill out this function */ 
  int i;
  mem_req_s * temp=NULL;
	 for(i=0;i<m_dram_bank_num;i++){
	    if(!dram_bank_sch_queue[i].empty()) {
	 		                            //search for not empty dram bank for row need to be process
	 		temp=dram_bank_sch_queue[i].front();
	 		if(dram_bank_rdy_cycle[i]<cycle_count){   //bank available
	 			if(dram_bank_open_row[i]==get_dram_row_id(temp->m_addr)){
	 				dram_row_buffer_hit_count++;
	 				temp->m_rdy_cycle=cycle_count+KNOB(KNOB_MEM_LATENCY_ROW_HIT)->getValue();

	 			}
	 			else{
	 				dram_row_buffer_miss_count++;
	 				temp->m_rdy_cycle=cycle_count+KNOB(KNOB_MEM_LATENCY_ROW_MISS)->getValue();
	 				dram_bank_open_row[i]=get_dram_row_id(temp->m_addr);

	 			}
	 			dram_bank_rdy_cycle[i]=temp->m_rdy_cycle;
	 			temp->m_state=MEM_DRAM_SCH;
	 		}

	       }

	 	}
   
}

/*******************************************************************/
/* send bus_out_queue 
/*******************************************************************/

void memory_c::send_bus_out_queue() 
{
	//cout << "print!!!" << endl;
   /* For Lab #2, you need to fill out this function */ 

    int i=0;
	mem_req_s * temp=NULL;
	for(i=0;i<m_dram_bank_num;i++){
			if(dram_bank_rdy_cycle[i]<cycle_count){    //addr complete process and needs to be send out
				//cout << "print!!!" << endl;
			    if(!dram_bank_sch_queue[i].empty() && dram_bank_sch_queue[i].front()->m_state==MEM_DRAM_SCH){  //have addr needs to be take out
			       temp=dram_bank_sch_queue[i].front();
			       temp->m_state=MEM_DRAM_OUT;
			       dram_out_queue.push_back(temp);
			       dram_bank_sch_queue[i].pop_front();

			    }

			}


		}


}


/*******************************************************************/
/*  fill_queue 
/*******************************************************************/

void memory_c::fill_queue() 
{

   /* For Lab #2, you need to fill out this function */ 
  /* CAUTION!: This function is not completed. Please complete this function */ 
 //cout << "print!!!" << endl;
  if (dram_out_queue.empty()) return;

    mem_req_s *req = dram_out_queue.front(); 
    dram_out_queue.pop_front(); 
    
    req->m_state=MEM_DRAM_DONE;     //wendy

    /* search for matching mshr  entry */ 
    m_mshr_entry_s *entry = search_matching_mshr(req->m_addr); 
    
    dcache_insert(entry->m_mem_req->m_addr);  //wendy
    //cout<<"!!!!TLB install in fill queue!!!"<<endl;
    list<m_mshr_entry_s *>::iterator mii = search_matching_mshr_itr(req->m_addr);
    if(req->m_type==MRT_DUMMY){                                    //wendy3
    		uint64_t base;
    		uint64_t pfn;

    		base=(*mii)->req_ops.front()->instruction_addr/vmem_page_size;
    		pfn=vmem_vpn_to_pfn(base,0);
    		tlb_install(dtlb,base,0,pfn);
    		Op *t_op=(*mii)->req_ops.front();
    		(*mii)->req_ops.pop_front();
    		free(t_op);
           //cout<<"!!!!TLB install in fill queue!!!"<<endl;
    		m_mshr.erase(mii);

    		free_mshr_entry(entry);
    		tlb_miss=MSHR_SERVICED;   // NOT USE NOW




    		return;
    	}                                          //wendy3




    while(entry->req_ops.size()) {
      Op *w_op = entry->req_ops.front();
      broadcast_rdy_op(w_op); 
      entry->req_ops.pop_front(); 
    }
    

    /* The following code will free mshr entry */ 
    //list<m_mshr_entry_s *>::iterator mii = search_matching_mshr_itr(req->m_addr);
    m_mshr.erase(mii);
  
  free_mshr_entry(entry); 

}





/*******************************************************************/
/*  store-load forwarind features, cache addresses, cache load/store types 
/*******************************************************************/


bool memory_c::store_load_forwarding(Op *mem_op)
{

  /*For Lab #2, you need to fill out this function */

       if (MEM_LD != mem_op->mem_type)
			return FALSE;

		list<m_mshr_entry_s *>::iterator iter = search_matching_mshr_itr(mem_op->ld_vaddr);
		if (iter==m_mshr.end())
			return FALSE;

		mem_req_s *req_s = (*iter)->m_mem_req;
		if (MRT_DSTORE == req_s->m_type && req_s->m_addr <= mem_op->ld_vaddr
				&& req_s->m_addr + req_s->m_size
						>= mem_op->ld_vaddr + mem_op->mem_read_size) {
			return TRUE;
		}
		return FALSE;


}
