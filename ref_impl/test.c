#include "acutest.h"
#include "../include/core.h"
void test_EditDistance(void){
   char word1[MAX_WORD_LENGTH];
   char word2[MAX_WORD_LENGTH];
   strcpy(word1,"armaggedon");
   strcpy(word2,"armada");
   int dist1=EditDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist1==5);
   memset(word1,0,MAX_WORD_LENGTH);
   memset(word2,0,MAX_WORD_LENGTH);
   strcpy(word1,"batman");
   strcpy(word2,"banana");
   int dist2=EditDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist2==3);
   memset(word1,0,MAX_WORD_LENGTH);
   memset(word2,0,MAX_WORD_LENGTH);
   strcpy(word1,"christmas");
   strcpy(word2,"hanuka");
   int dist3=EditDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist3==7);
   memset(word1,0,MAX_WORD_LENGTH);
   memset(word2,0,MAX_WORD_LENGTH);
   strcpy(word1,"alabama");
   strcpy(word2,"mama");
   int dist4=EditDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist4==4);
}
void test_HammingDistance(void){
   char word1[MAX_WORD_LENGTH];
   char word2[MAX_WORD_LENGTH];
   strcpy(word1,"armaggedon");
   strcpy(word2,"armada");
   int dist1=HammingDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist1==6);
   memset(word1,0,MAX_WORD_LENGTH);
   memset(word2,0,MAX_WORD_LENGTH);
   strcpy(word1,"batman");
   strcpy(word2,"banana");
   int dist2=HammingDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist2==4);
   memset(word1,0,MAX_WORD_LENGTH);
   memset(word2,0,MAX_WORD_LENGTH);
   strcpy(word1,"christmas");
   strcpy(word2,"hanuka");
   int dist3=HammingDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist3==9);
   memset(word1,0,MAX_WORD_LENGTH);
   memset(word2,0,MAX_WORD_LENGTH);
   strcpy(word1,"alabama");
   strcpy(word2,"mama");
   int dist4=HammingDistance(word1,strlen(word1),word2,strlen(word2));
   TEST_CHECK(dist4==7);
}

struct HammingDistanceStruct* HammingDistanceStructNode;
Index*  BKTreeIndexEdit;
struct Query_Info* ActiveQueries;
struct Exact_Root* HashTableExact;
int bucket_sizeofHashTableExact;
unsigned int active_queries;
struct Stack_result* StackArray;

void test_InitializeIndex(void){
   ErrorCode ret1=InitializeIndex();
   TEST_CHECK(ret1!=EC_FAIL);

}









void test_Put_query_on_Active_Queries(void){
   QueryID q=10;
   Put_query_on_Active_Queries(q,7);
   struct Query_Info* start=ActiveQueries;
   int found=0;
   while(1){
      if(start==NULL)
         break;
      if(start->query_id==q&&start->counter_of_distinct_words==7){
         found=1;
         break;
      }
      start=start->next;
   }
   TEST_CHECK(found==1);
}


void test_Free_Active_Queries(void){
   QueryID q=10;
   Put_query_on_Active_Queries(q,7);
   struct Query_Info* start=ActiveQueries;
   int found=0;
   while(1){
      if(start==NULL)
         break;
      if(start->query_id==q&&start->counter_of_distinct_words==7){
         found=1;
         break;
      }
      start=start->next;
   }
   Free_Active_Queries();
   TEST_CHECK(ActiveQueries==NULL);
}



void test_Delete_From_Stack(void){
   StackArray=malloc(sizeof(StackArray));
   StackArray->top=NULL;
   StackArray->first=NULL;
   StackArray->counter=0;
   int found1=0;
   int found2=0;
   QueryID* q=malloc(10*sizeof(QueryID));
   for(int i=0;i<10;i++)
      q[i]=i;
   DocID k=10;
   unsigned int num=10;
   Put_On_Stack_Result(k,num,q);
   free(q);
   q=malloc(20*sizeof(QueryID));
   for(int i=0;i<20;i++)
      q[i]=i;
   k=20;
   num=20;
   Put_On_Stack_Result(k,num,q);
   Delete_From_Stack();
   Delete_From_Stack();
   TEST_CHECK(StackArray->top==NULL);
}

void test_Put_On_Stack_Result(void){
   StackArray=malloc(sizeof(StackArray));
   StackArray->top=NULL;
   StackArray->first=NULL;
   StackArray->counter=0;
   int found1=0;
   int found2=0;
   QueryID* q=malloc(10*sizeof(QueryID));
   for(int i=0;i<10;i++)
      q[i]=i;
   DocID k=10;
   unsigned int num=10;
   Put_On_Stack_Result(k,num,q);
   free(q);
   q=malloc(20*sizeof(QueryID));
   for(int i=0;i<20;i++)
      q[i]=i;
   k=20;
   num=20;
   Put_On_Stack_Result(k,num,q);
   struct result* node=StackArray->top;
   while(1){
      if(node==NULL)
         break;
      if(node->doc_id==20&&node->result_counter==20)
         found2=1;
      if(node->doc_id==10&&node->result_counter==10)
         found1=1;
      node=node->next;;
   }
   free(q);
   TEST_CHECK(found2==1);
   TEST_CHECK(found1==1);
}

void test_Delete_Query_from_Active_Queries(void){
   QueryID q=10;
   Put_query_on_Active_Queries(q,7);
   Delete_Query_from_Active_Queries(q);
   struct Query_Info* start=ActiveQueries;
   int found=0;
   while(1){
      if(start==NULL)
         break;
      if(start->query_id==q&&start->counter_of_distinct_words==7){
         found=1;
         break;
      }
      start=start->next;
   }
   Free_Active_Queries();
   TEST_CHECK(found==0);
}

void test_NextPrime(void){
   int val=NextPrime(20);
   TEST_CHECK(val==23);
}

void test_isPrime(void){
   bool val=isPrime(20);
   TEST_CHECK(val==false);
}



void test_Hash_Put_Result(void){
   QueryID q=10;
   int found=0;
   char* w="word";
   struct Result_Hash_Node* rr1=NULL;
   Hash_Put_Result(q,w,&rr1);
   struct Result_Hash_Node* beg=rr1;
   while(1){
      if(beg==NULL)
         break;
      if(beg->query_id==q){
         if(!strcmp(beg->word_start->word,"word")){
            found=1;
            break;
         }
      }
      beg=beg->next;
   }
   TEST_CHECK(found==1);
}


void test_words_ofquery(void){
   char* query_str="word1 word2";
   int num=0;
   int found1=0;
   int found2=0;
   char** words=words_ofquery(query_str,&num);
   for(int i=0;i<num;i++){
      if(!strcmp(words[i],"word1"))
         found1=1;
      if(!strcmp(words[i],"word2"))
         found2=1;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
}



void test_Initialize_Hash_Array(void){
  struct Deduplicate_Hash_Array* Deduplication_Array=Initialize_Hash_Array(10);
  TEST_CHECK(Deduplication_Array!=NULL);
  TEST_CHECK(Deduplication_Array->array!=NULL);
}




/*void test_free_Deduplication_Hash_Array(void){
   struct Deduplicate_Hash_Array* Deduplication_Array=Initialize_Hash_Array(10);
   free_Deduplication_Hash_Array(Deduplication_Array,10);
   int found=0;
   for(int i=0;i<10;i++)
      if(Deduplication_Array->array[i]==NULL) found=1;
   TEST_CHECK(found==0);
}*/

void test_Deduplicate_Method(void){
   char* query_str="word1 word2 word2 word2 word1 word3 word3";
   int num=0;
   int found1=0;
   int found2=0;
   int found3=0;
   char** words=Deduplicate_Method(query_str,&num);
   for(int i=0;i<num;i++){
      if(!strcmp(words[i],"word1"))
         found1=1;
      if(!strcmp(words[i],"word2"))
         found2=1;
      if(!strcmp(words[i],"word3"))
         found3++;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found3==1);
   TEST_CHECK(found2==1);
}



void test_delete_specific_payload(void){
   struct Exact_Node* node=malloc(sizeof(struct Exact_Node));
   payload_node* p1=malloc(sizeof(payload_node));
   p1->query_id=10;
   p1->next=NULL;
   payload_node* p2=malloc(sizeof(payload_node));
   p2->query_id=2;
   p2->next=p1;
   node->beg=p2;
   delete_specific_payload(&node,10);
   payload_node* start1=node->beg;
   int found=0;
   while(start1!=NULL){
      if(start1->query_id==10){
         found=1;
         break;
      }
      start1=start1->next;
   }
   free(p2);
   free(node);
   TEST_CHECK(found==0);
}



void test_insert_HashTableExact(void){
   InitializeIndex();
   int found1=0;
   int found2=0;
   insert_HashTableExact("word1",3,10);
   insert_HashTableExact("word2",2,20);
   struct Exact_Node* start1=HashTableExact->array[3];
   while(1){
      if(!strcmp(start1->wd,"word1")){
         found1=1;
         break;
      }
      start1=start1->next;
      if(start1==NULL)
         break;
   }
   start1=HashTableExact->array[2];
   while(1){
      if(!strcmp(start1->wd,"word2")){
         found2=1;
         break;
      }
      start1=start1->next;
      if(start1==NULL)
         break;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
}


void test_insert_HashTableExact_V2(void){
   int found1=0;
   int found2=0;
   struct Exact_Root* HashTableExact=malloc(sizeof(struct Exact_Root));
   HashTableExact->array=malloc(10*sizeof(struct Exact_Node*));
   for(int i=0;i<10;i++)
      HashTableExact->array[i]=NULL;
   payload_node* p1=malloc(sizeof(payload_node));
   p1->query_id=2;
   p1->next=NULL;
   insert_HashTableExact_V2(HashTableExact,"word1",9,p1);
   insert_HashTableExact_V2(HashTableExact,"word2",5,p1);
   struct Exact_Node* start1=HashTableExact->array[9];
   while(1){
      if(!strcmp(start1->wd,"word1")){
         found1=1;
         break;
      }
      start1=start1->next;
      if(start1==NULL)
         break;
   }
   start1=HashTableExact->array[5];
   while(1){
      if(!strcmp(start1->wd,"word2")){
         found2=1;
         break;
      }
      start1=start1->next;
      if(start1==NULL)
         break;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
}




void test_Exact_Result(void){
   struct Match_Type_List* Final=malloc(sizeof(struct Match_Type_List));
   Final->start=NULL;
   Final->counter=0;
   Final->cur=NULL;
   InitializeIndex();
   int found1=0;
   int found2=0;
   insert_HashTableExact("word1",3,10);
   insert_HashTableExact("word2",2,20);
   struct Match_Type_List* Final1=Exact_Result("word1");
   Final->start=Final1->start;
   Final->cur=Final1->cur;
   struct Match_Type_List* Final2=Exact_Result("word2");
   Final->cur->next=Final2->start;
   Final->cur=Final2->cur;
   Entry* start=Final->start;
   while(1){
      if(!strcmp(start->my_word,"word1"))
         found1=1;
      if(!strcmp(start->my_word,"word2"))
         found2=1;
      start=start->next;
      if(start==NULL)
         break;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
}

void test_Check_Exact_Hash_Array(void){
   int found1=0;
   int found2=0;
   struct Exact_Root* HashTableExact=malloc(sizeof(struct Exact_Root));
   HashTableExact->array=malloc(10*sizeof(struct Exact_Node*));
   for(int i=0;i<10;i++)
      HashTableExact->array[i]=NULL;
   payload_node* p1=malloc(sizeof(payload_node));
   p1->query_id=2;
   p1->next=NULL;
   payload_node* p2=malloc(sizeof(payload_node));
   p2->query_id=3;
   p2->next=NULL;
   insert_HashTableExact_V2(HashTableExact,"word1",9,p1);
   insert_HashTableExact_V2(HashTableExact,"word2",5,p2);
   Check_Exact_Hash_Array(3);
   Check_Exact_Hash_Array(2);
   for(int i=0;i<10;i++){
      struct Exact_Node* start=HashTableExact->array[i];
      if(start==NULL)
         break;
      payload_node* e1=start->beg;
      while(1){
         if(e1->query_id==2)
            found1=1;
         if(e1->query_id==3)
            found2=1;
         e1=e1->next;
         if(e1==NULL)
            break;
      }
   }
   TEST_CHECK(found1==0);
   TEST_CHECK(found2==0);
}

void test_empty_of_payload_nodes(void){
   struct Exact_Node* node=malloc(sizeof(struct Exact_Node));
   node->beg=NULL;
   node->wd=NULL;
   bool val=empty_of_payload_nodes(node);
   TEST_CHECK(val==true);
}


void test_check_if_word_exists(void){
   InitializeIndex();
   bool found1=false;
   bool found2=false;
   bool found3=false;
   bool found4=false;
   struct Exact_Root* HashTableExact=malloc(sizeof(struct Exact_Root));
   HashTableExact->array=malloc(10*sizeof(struct Exact_Node*));
   for(int i=0;i<10;i++)
      HashTableExact->array[i]=NULL;
   payload_node* p1=malloc(sizeof(payload_node));
   p1->query_id=2;
   p1->next=NULL;
   payload_node* p2=malloc(sizeof(payload_node));
   p2->query_id=3;
   p2->next=NULL;
   insert_HashTableExact("word1",3,10);
   insert_HashTableExact("word2",2,20);
   insert_HashTableExact("word3",2,20);
   insert_HashTableExact("word4",2,20);
   found1=check_if_word_exists("word1",3,10);
   found2=check_if_word_exists("word2",2,20);
   found3=check_if_word_exists("word3",2,20);
   found4=check_if_word_exists("word4",2,20);
   TEST_CHECK(found1==true);
   TEST_CHECK(found2==true);
   TEST_CHECK(found3==true);
   TEST_CHECK(found4==true);

}

void test_EditPut_build_entry_index_Edit_destroy_Edit_index(void){
   char* query_str="word1 word2 word2 word2 word1 word3 word3";
   unsigned int match_dist=2;
   int found1=0;
   int found2=0;
   InitializeIndex();
   int found3=0;
   QueryID query_id=10;
   int num=0;
   char** words=Deduplicate_Method(query_str,&num);
   Edit_Put(words,num,query_id,match_dist);
   struct EditNode* start=BKTreeIndexEdit->root;
   while(1){
      if(!strcmp(start->wd,"word1"))
         found1=1;
      if(!strcmp(start->wd,"word2"))
         found2=1;
      if(!strcmp(start->wd,"word3"))
         found3=1;
      if(start->next==NULL)
         start=start->firstChild;
      else
         start=start->next;
      if(start==NULL)
         break;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
   TEST_CHECK(found3==1);
   destroy_Edit_index(BKTreeIndexEdit);
   TEST_CHECK(BKTreeIndexEdit->root==NULL);
   query_str="worda wordb worrr";
   match_dist=2;
   found1=0;
   found2=0;
   InitializeIndex();
   found3=0;
   query_id=10;
   num=0;
   words=Deduplicate_Method(query_str,&num);
   Edit_Put(words,num,query_id,match_dist);
   start=BKTreeIndexEdit->root;
   while(1){
      if(!strcmp(start->wd,"worda"))
         found1=1;
      if(!strcmp(start->wd,"wordb"))
         found2=1;
      if(!strcmp(start->wd,"worrr"))
         found3=1;
      if(start->next==NULL)
         start=start->firstChild;
      else
         start=start->next;
      if(start==NULL)
         break;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
   TEST_CHECK(found3==1);
}

void test_HammingPut_build_entry_index_Hamming(void){
   char* query_str="worda worddr worddp wordb wordc";
   unsigned int match_dist=2;
   int found1=0;
   int found2=0;
   InitializeIndex();
   int found3=0;
   int found4=0;
   int found5=0;
   QueryID query_id=10;
   int num=0;
   char** words=Deduplicate_Method(query_str,&num);
   Hamming_Put(words,num,query_id,match_dist);
   struct word_RootPtr* word_ptr1=&HammingDistanceStructNode->word_RootPtrArray[strlen("worda")];
   struct word_RootPtr* word_ptr2=&HammingDistanceStructNode->word_RootPtrArray[strlen("worddp")];
   /*struct HammingNode* H1_start=word_ptr1->HammingPtr->root;
   struct HammingNode* H2_start=word_ptr2->HammingPtr->root;
   while(1){
      if(!strcmp("worda",H1_start->wd))
         found1=1;
      if(!strcmp("wordb",H1_start->wd))
         found2=1;
      if(!strcmp("wordc",H1_start->wd))
         found3=1;
      if(H1_start->next==NULL)
         break;
      else  
         H1_start=H1_start->next;
      if(H1_start==NULL)
         break;
   }
   while(1){
      if(!strcmp("worddp",H2_start->wd))
         found4=1;
      if(!strcmp("worddr",H2_start->wd))
         found5=1;
      if(H2_start->next==NULL)
         H2_start=H2_start->firstChild;
      else  
         H2_start=H2_start->next;
      if(H2_start==NULL)
         break;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
   TEST_CHECK(found3==1);
   TEST_CHECK(found4==1);
   TEST_CHECK(found5==1);*/
}


void test_Edit_Result(void){
   char* query_str="worda wordb wordb wordb worda wordc wordc";
   unsigned int match_dist=3;
   int found1=0;
   int found2=0;
   InitializeIndex();
   int found3=0;
   QueryID query_id=10;
   int num=0;
   char** words=Deduplicate_Method(query_str,&num);
   Edit_Put(words,num,query_id,match_dist);
   struct Match_Type_List* el=Edit_Result("wordd");
   Entry* beg=el->start;
   while(1){
      if(beg==NULL)
         break;
      if(!strcmp(beg->my_word,"worda"))
         found1=1;
      if(!strcmp(beg->my_word,"wordb"))
         found2=1;
      if(!strcmp(beg->my_word,"wordc"))
         found3=1;
      beg=beg->next;
   }
   TEST_CHECK(found1==1);
   TEST_CHECK(found2==1);
   TEST_CHECK(found3==1);
}


void test_Delete_Query_from_Edit_Nodes(void){
   struct EditNode* node=malloc(sizeof(struct EditNode));
   node->wd=NULL;
   int found1=0;
   node->start_info=NULL;
   struct Info* p1=malloc(sizeof(struct Info));
   p1->query_id=10;
   p1->match_dist=2;
   p1->next=NULL;
   struct Info* p2=malloc(sizeof(struct Info));
   p2->query_id=20;
   p2->match_dist=3;
   p2->next=NULL;
   p1->next=p2;
   node->start_info=p1;
   Delete_Query_from_Edit_Nodes(node,10);
   struct Info* start=node->start_info;
   while(1){
      if(start==NULL)
         break;
      if(start->query_id==10)
         found1=1;
      start=start->next;
   }
   TEST_CHECK(found1==0);
}



void test_Delete_Result_List(void){
   /*struct Match_Type_List* en=malloc(sizeof(struct Match_Type_List));
   en->cur=NULL;
   en->counter=0;
   en->start=NULL;
   Entry* a=malloc(sizeof(Entry));
   Entry* b=malloc(sizeof(Entry));
   a->my_word=malloc((strlen("word1")+1)*sizeof(char));
   strcpy(a->my_word,"word1");
   payload_node* p1=malloc(sizeof(payload_node));
   p1->query_id=10;
   p1->next=NULL;
   a->payload=p1;
   a->next=NULL;
   b->my_word=malloc((strlen("word2")+1)*sizeof(char));
   strcpy(b->my_word,"word2");
   b->next=NULL;
   payload_node* p2=malloc(sizeof(payload_node));
   p2->query_id=20;
   p2->next=NULL;
   b->payload=p2;
   a->next=b;
   en->start=a;
   en->cur=b;
   en->counter=2;
   Delete_Result_List(en);
   TEST_CHECK(en==NULL);*/
}



void test_Put_data(void){
   int found=0;
   struct Exact_Node* node=malloc(sizeof(struct Exact_Node));
   node->wd=malloc((strlen("word")+1)*sizeof(char));
   strcpy(node->wd,"word");
   node->next=NULL;
   payload_node* p1=malloc(sizeof(payload_node));
   p1->query_id=10;
   p1->next=NULL;
   payload_node* p2=malloc(sizeof(payload_node));
   p2->query_id=20;
   p2->next=NULL;
   p1->next=p2;
   node->beg=p1;
   Entry* entry_node=Put_data(node);
   if(!strcmp(entry_node->my_word,"word"))
      found++;
   payload_node* p_start=entry_node->payload;
   while(1){
      if(p_start==NULL)
         break;
      if(p_start->query_id==10||p_start->query_id==20)
         found++;
      p_start=p_start->next;
   }
   TEST_CHECK(found==3);
}








TEST_LIST = {
   {"Delete_Result_List",test_Delete_Result_List},/*- cannot check why its impossible to check if memeory freed*/
   {"check_if_word_exists",test_check_if_word_exists},
   {"Check_Exact_Hash_Array",test_Check_Exact_Hash_Array},
   {"Hash_Put_Result",test_Hash_Put_Result},
   {"EditDistance",test_EditDistance},
   {"isPrime",test_isPrime},
   {"Initialize_Hash_Array",test_Initialize_Hash_Array},
   {"HammingDistance",test_HammingDistance},
   {"EditPut-build_entry_index_Edit-destroy_Edit_index",test_EditPut_build_entry_index_Edit_destroy_Edit_index},
   {"HammingPut-build_entry_index_Edit",test_HammingPut_build_entry_index_Hamming},
   {"InitializeIndex",test_InitializeIndex},
   {"NextPrime",test_NextPrime},
   {"insert_HashTableExact",test_insert_HashTableExact},
   {"insert_HashTableExact_V2",test_insert_HashTableExact_V2},
   {"Deduplicate_Method",test_Deduplicate_Method},
   {"Free_Active_Queries",test_Free_Active_Queries},
   {"delete_specific_payload",test_delete_specific_payload},
   {"Put_On_Stack_Result",test_Put_On_Stack_Result},
   {"Exact_Result",test_Exact_Result},
   {"Delete_From_Stack",test_Delete_From_Stack},
   {"Put_query_on_Active_Queries",test_Put_query_on_Active_Queries},
   {"words_ofquery",test_words_ofquery},
   {"Edit_Result",test_Edit_Result},
   {"Put_data",test_Put_data},
   {"Delete_Query_from_Edit_Nodes",test_Delete_Query_from_Edit_Nodes},
   {"empty_of_payload_nodes",test_empty_of_payload_nodes},
   //{"free_Deduplication_Hash_Array",test_free_Deduplication_Hash_Array}, - cannot check why its impossible to check if memeory freed
   {"Delete_Query_from_Active_Queries",test_Delete_Query_from_Active_Queries},
   { NULL,NULL }
};