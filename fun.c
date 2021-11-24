#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fun.h"
#define MAX_WORD_LENGTH 31

	
ErrorCode InitializeIndex(){

}

ErrorCode DestroyIndex(){

}

ErrorCode StartQuery(QueryID query_id,const char* query_str,MatchType match_type,unsigned int match_dist){

}

ErrorCode EndQuery(QueryID query_id){

}

ErrorCode MatchDocument(DocID doc_id,const char* doc_str){

}

ErrorCode GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids){

}
