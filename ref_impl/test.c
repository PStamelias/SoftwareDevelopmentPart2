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
   char* query_str="word1 word2 word3 word3";
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

TEST_LIST = {
   {"Hash_Put_Result",test_Hash_Put_Result},
   {"EditDistance",test_EditDistance},
   {"isPrime",test_isPrime},
   {"Initialize_Hash_Array",test_Initialize_Hash_Array},
   {"HammingDistance",test_HammingDistance},
   {"InitializeIndex",test_InitializeIndex},
   {"NextPrime",test_NextPrime},
   {"Deduplicate_Method",test_Deduplicate_Method},
   {"Free_Active_Queries",test_Free_Active_Queries},
   {"Put_On_Stack_Result",test_Put_On_Stack_Result},
   {"Delete_From_Stack",test_Delete_From_Stack},
   {"Put_query_on_Active_Queries",test_Put_query_on_Active_Queries},
   {"words_ofquery",test_words_ofquery},
   //{"free_Deduplication_Hash_Array",test_free_Deduplication_Hash_Array}, - cannot check
   {"Delete_Query_from_Active_Queries",test_Delete_Query_from_Active_Queries},
   { NULL,NULL }
};