typedef enum MatchType{
  MT_EXACT_MATCH,
  MT_HAMMING_DIST,
  MT_EDIT_DIST
}MatchType;			
typedef enum ErrorCode{
  EC_SUCCESS,
  EC_NO_AVAIL_RES,
  EC_FAIL
}ErrorCode;
typedef unsigned int QueryID;
typedef unsigned int DocID;
ErrorCode InitializeIndex();
ErrorCode DestroyIndex();
ErrorCode StartQuery(QueryID query_id,const char* query_str,MatchType match_type,unsigned int match_dist);
ErrorCode EndQuery(QueryID query_id);
ErrorCode MatchDocument(DocID doc_id,const char* doc_str);
ErrorCode GetNextAvailRes(DocID* p_doc_id,unsigned int* p_num_res,QueryID** p_query_ids);
