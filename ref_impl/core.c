#include "../include/core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

//used in EditDistance below
int min(int a, int b, int c){
    int m = a;
    if(b<m){
        m = b;
    }
    if(c<m){
        m = c;
    }
    return m;
}

int EditDistance(char* a, int na, char* b, int nb)
{
	int table[na+1][nb+1];
    int i,j;
    for(i=0; i<=na; i++){
        for(j=0; j<=nb; j++){
            if(i == 0){
                table[i][j] = j;
            }
            else if(j == 0){
                table[i][j] = i;
            }
            else if(a[i-1] == b[j-1]){
                table[i][j] = table[i-1][j-1];
            }
            else{
                table[i][j] = 1 + min(table[i-1][j], table[i][j-1], table[i-1][j-1]);
            }
        }
    }
    return table[na][nb];
}

unsigned int HammingDistance(char* a, int na, char* b, int nb)
{
	int i;
	int distance = 0;
	for(i=0; i<na; i++){
		if(a[i] != b[i]){
			distance++;
		}
	}
	return distance;
}


struct HammingDistanceStruct* HammingDistanceStructNode;
Index*  BKTreeIndexEdit;
struct Query_Info* ActiveQueries;
struct Exact_Root* HashTableExact;
int bucket_sizeofHashTableExact;
unsigned int active_queries;
struct Stack_result* StackArray;

ErrorCode InitializeIndex(){
	active_queries=0;
	StackArray=malloc(sizeof(struct Stack_result));
	StackArray->counter=0;
	StackArray->first=NULL;
	StackArray->top=NULL;
	ActiveQueries=NULL;
	BKTreeIndexEdit=malloc(sizeof(Index));
	BKTreeIndexEdit->root=NULL;
	HashTableExact=NULL;
	bucket_sizeofHashTableExact=5;/*starting bucket size of hash array*/
	HashTableExact=malloc(sizeof(struct Exact_Root));
	HashTableExact->array=malloc(bucket_sizeofHashTableExact*sizeof(struct Exact_Node*));
	for(int i=0;i<bucket_sizeofHashTableExact;i++)
		HashTableExact->array[i]=NULL;
	/*Hamming struct initilization*/
	int HammingIndexSize=(MAX_WORD_LENGTH-MIN_WORD_LENGTH)+1;
	HammingDistanceStructNode=NULL;
	HammingDistanceStructNode=malloc(sizeof(struct HammingDistanceStruct));
	HammingDistanceStructNode->word_RootPtrArray=malloc(HammingIndexSize*sizeof(struct word_RootPtr));
	for(int i=0;i<HammingIndexSize;i++){
		HammingDistanceStructNode->word_RootPtrArray[i].HammingPtr=NULL;
		HammingDistanceStructNode->word_RootPtrArray[i].word_length=4+i;
	}
	/*******************************/
	if(BKTreeIndexEdit==NULL) return EC_FAIL;
	if(HashTableExact==NULL) return EC_FAIL;
	if(HammingDistanceStructNode==NULL) return EC_FAIL;
	return EC_SUCCESS;
}

ErrorCode DestroyIndex(){
	for(int i=0; i<bucket_sizeofHashTableExact; i++){
		if(HashTableExact->array[i] == NULL) continue;
		struct Exact_Node* start = HashTableExact->array[i];
		struct Exact_Node* start_next = start->next;
		while(1){
			struct payload_node* start1=start->beg;
			struct payload_node* start2=start1->next;
			while(1){
				free(start1);
				start1=start2;
				if(start1==NULL)
					break;
				start2=start2->next;
			}
			free(start->wd);
			free(start);
			start = start_next;
			if(start == NULL) break;
			start_next = start_next->next;
		}
	}
	free(HashTableExact->array);
	free(HashTableExact);
	//call destroy gia to BKTree */
	free(BKTreeIndexEdit);
	free(HammingDistanceStructNode->word_RootPtrArray);
	free(HammingDistanceStructNode);	
	return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	active_queries++;
	int words_num=0;
	char** query_words=words_ofquery(query_str,&words_num);
	Put_query_on_Active_Queries(query_id,words_num);
	if(match_type==0)
		Exact_Put(query_words,words_num,query_id);
	else if(match_type==1)
		Hamming_Put(query_words,words_num,query_id,match_dist);
	else if(match_type==2)
		Edit_Put(query_words,words_num,query_id,match_dist);
	for(int i=0;i<words_num;i++)
		free(query_words[i]);
	free(query_words);
	query_words=NULL;
	if(query_words!=NULL)
		return EC_FAIL;
	/*********************/
	return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id)
{	
	active_queries--;
	Delete_Query_from_Active_Queries(query_id);
	/*check if query exists on ExactHashTable*/
	Check_Exact_Hash_Array(query_id);
	/*check if query exists on EditBKTree*/
	Check_Edit_BKTree(query_id);
	/*check if query exists on HammingBKTrees*/
	Check_Hamming_BKTrees(query_id);
	return EC_SUCCESS;
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	printf("doc_id=%u\n",doc_id);
	int words_num=0;
	char** words_oftext=Deduplicate_Method(doc_str,&words_num);
	Entry* exact_list=NULL;
	Entry* edit_list=NULL;
	Entry* hamming_list=NULL;
	int num1=0;
	int num2=0;
	int num_result=0;
	int num3=0;
	for(int i=0;i<words_num;i++){
		exact_list=Exact_Result(words_oftext[i],&num1);
		edit_list=Edit_Result(words_oftext[i],&num2);
		hamming_list=Hamming_Result(words_oftext[i],&num3);
		if(edit_list!=NULL)
			printf("makis2\n");
		if(hamming_list!=NULL)
			printf("makis3\n");
	}
	/*if(exact_list==NULL)
		printf("null exact_list\n");
	if(edit_list==NULL)
		printf("null edit_list\n");
	if(hamming_list==NULL)
		printf("null hamming_list\n");*/
	QueryID* query_id_result=Put_On_Result_Hash_Array(exact_list,edit_list,hamming_list,num1,num2,num3,&num_result);
	Put_On_Stack_Result(doc_id,num_result,query_id_result);
	Delete_Result_List(exact_list);
	Delete_Result_List(edit_list);
	Delete_Result_List(hamming_list);
	for(int i=0;i<words_num;i++)
		free(words_oftext[i]);
	free(words_oftext);
	return EC_SUCCESS;
}


ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{	
	printf("GetNextAvailRes\n");
	DocID doc=StackArray->top->doc_id;
	*p_doc_id=doc;
	unsigned int counter=StackArray->top->result_counter;
	*p_num_res=counter;
	QueryID* curr=malloc(StackArray->top->result_counter*sizeof(QueryID));
	for(int i=0;i<StackArray->top->result_counter;i++)
		curr[i]=StackArray->top->query_id[i];
	*p_query_ids=curr;
	Delete_From_Stack();
	return EC_SUCCESS;
}




int NextPrime(int N)
{
    bool found = false;
    while(!found){
        found = isPrime(N);
        N++;
    }
    return (N - 1);
}


bool isPrime(int N){
    
    for(int i = 2; i <= (N/2); i++){
        if(N % i == 0) return false;
    }
    return true;
}



unsigned int hashing(const char *word) {
    unsigned int hash = 0, c;

    for (size_t i = 0; word[i] != '\0'; i++) {
        c = (unsigned char)word[i];
        hash = (hash << 3) + (hash >> (sizeof(hash) * CHAR_BIT - 3)) + c;
    }
    return hash;
}

float counting_load_factor(int n, int k){
	float re;
	re=(float)n/(float)k;
	return re;

}

char** Deduplicate_Method(const char* query_str,int* size){
	int BucketsHashTable=10;
	char** word_array=NULL;
	struct Deduplicate_Hash_Array* Deduplication_Array=Initialize_Hash_Array(BucketsHashTable);
	char word[MAX_WORD_LENGTH];
	int len=0;
	for(int i=0;i<strlen(query_str);i++){
		if(query_str[i]==' '){
			word[len]='\0';
			len=0;
			if(search_hash_array(Deduplication_Array,BucketsHashTable,word)==true){
				memset(word,0,MAX_WORD_LENGTH);
				continue;
			}
			float load_factor=counting_load_factor(Deduplication_Array->entries_counter+1,BucketsHashTable);
			if(load_factor>=0.8){
				int old_bucket_num=BucketsHashTable;
				BucketsHashTable=NextPrime(BucketsHashTable*2);
				struct Deduplicate_Hash_Array* new_Deduplicate_Hash_Array=Initialize_Hash_Array(BucketsHashTable);
				for(int j=0;j<old_bucket_num;j++){
					if(Deduplication_Array->array[j]==NULL) continue;
					struct Deduplicate_Node* start=Deduplication_Array->array[j];
					struct Deduplicate_Node* next_start=start->next;
					while(1){
						insert_hash_array(&new_Deduplicate_Hash_Array,BucketsHashTable,start->the_word);
						free(start->the_word);
						free(start);
						start=next_start;
						if(start==NULL) break;
						next_start=next_start->next;
					}
				}
				free(Deduplication_Array->array);
				free(Deduplication_Array);
				insert_hash_array(&new_Deduplicate_Hash_Array,BucketsHashTable,word);
				Deduplication_Array=new_Deduplicate_Hash_Array;
			}
			else insert_hash_array(&Deduplication_Array,BucketsHashTable,word);
			memset(word,0,MAX_WORD_LENGTH);
			continue;
		}
		if(i==strlen(query_str)-1){
			word[len++]=query_str[i];
			word[len]='\0';
			len=0;
			if(search_hash_array(Deduplication_Array,BucketsHashTable,word)==true){
				memset(word,0,MAX_WORD_LENGTH);
				continue;
			}
			float load_factor=counting_load_factor(Deduplication_Array->entries_counter+1,BucketsHashTable);
			if(load_factor>=0.8){
				int old_bucket_num=BucketsHashTable;
				BucketsHashTable=NextPrime(BucketsHashTable*2);
				struct Deduplicate_Hash_Array* new_Deduplicate_Hash_Array=Initialize_Hash_Array(BucketsHashTable);
				for(int j=0;j<old_bucket_num;j++){
					if(Deduplication_Array->array[j]==NULL) continue;
					struct Deduplicate_Node* start=Deduplication_Array->array[j];
					struct Deduplicate_Node* next_start=start->next;
					while(1){
						insert_hash_array(&new_Deduplicate_Hash_Array,BucketsHashTable,start->the_word);
						free(start->the_word);
						free(start);
						start=next_start;
						if(start==NULL) break;
						next_start=next_start->next;
					}
				}
				free(Deduplication_Array->array);
				free(Deduplication_Array);
				insert_hash_array(&new_Deduplicate_Hash_Array,BucketsHashTable,word);
				Deduplication_Array=new_Deduplicate_Hash_Array;
			}
			else insert_hash_array(&Deduplication_Array,BucketsHashTable,word);
			memset(word,0,MAX_WORD_LENGTH);
			continue;
		}
		word[len++]=query_str[i];
	}
	word_array=malloc(Deduplication_Array->entries_counter*sizeof(char*));
	int coun=0;
	for(int i=0;i<BucketsHashTable;i++){
		if(Deduplication_Array->array[i]==NULL)	continue;
		struct Deduplicate_Node* start=Deduplication_Array->array[i];
		while(1){
			word_array[coun]=malloc((strlen(start->the_word)+1)*sizeof(char));
			strcpy(word_array[coun],start->the_word);
			coun++;
			start=start->next;
			if(start==NULL) break;
		}
	}
	*size=coun;
	free_Deduplication_Hash_Array(Deduplication_Array,BucketsHashTable);
	return word_array;
} 



ErrorCode destroy_entry_index(Index* ix){
	if(ix==NULL) return EC_FAIL;
	destroy_index_nodes(ix->root);
	free(ix);
	ix=NULL;
	if(ix!=NULL) return EC_FAIL;	
	return EC_SUCCESS;
}

void destroy_index_nodes(struct EditNode* node){
	
}



void free_Deduplication_Hash_Array(struct Deduplicate_Hash_Array* hash,int BucketsHashTable){
	for(int i=0;i<BucketsHashTable;i++){
		if(hash->array[i]==NULL) continue;
		struct Deduplicate_Node* start=hash->array[i];
		struct Deduplicate_Node* next_start=start->next;
		while(1){
			free(start->the_word);
			free(start);
			start=next_start;
			if(start==NULL) break;
			next_start=next_start->next;
		}
	}
	free(hash->array);
	free(hash);
}

struct Deduplicate_Hash_Array* Initialize_Hash_Array(int BucketsHashTable){
	struct Deduplicate_Hash_Array* Deduplication_Array=malloc(sizeof(struct Deduplicate_Hash_Array));
	Deduplication_Array->entries_counter=0;
	Deduplication_Array->array=NULL;
	Deduplication_Array->array=malloc(BucketsHashTable*sizeof(struct Deduplicate_Node));
	for(int i=0;i<BucketsHashTable;i++)
		Deduplication_Array->array[i]=NULL;
	return Deduplication_Array;
}



void insert_hash_array(struct Deduplicate_Hash_Array** hash,int BucketsHashTable,char* word){
	int bucket_num=hashing(word)%BucketsHashTable;
	(*hash)->entries_counter++;
	struct Deduplicate_Node* start=(*hash)->array[bucket_num];
	struct Deduplicate_Node* new_node=malloc(sizeof(struct Deduplicate_Node));
	new_node->next=NULL;
	new_node->the_word=malloc((strlen(word)+1)*sizeof(char));
	strcpy(new_node->the_word,word);
	if(start==NULL) (*hash)->array[bucket_num]=new_node;
	else{
		struct Deduplicate_Node* node= (*hash)->array[bucket_num];
		while(1){
			if(node->next==NULL){
				node->next=new_node;
				break;
			}
			node=node->next;
		}
	}	
}

bool search_hash_array(struct Deduplicate_Hash_Array* hash,int BucketsHashTable,char* word){
	int bucket_num=hashing(word)%BucketsHashTable;
	int found=0;
	struct Deduplicate_Node* start=hash->array[bucket_num];
	if(start==NULL) return false;
	else{
		struct Deduplicate_Node* node=start;
		while(1){
			if(!strcmp(node->the_word,word)){
				found=1;
				break;
			}
			node=node->next;
			if(node==NULL) break;
		}
	}
	if(found==1) return true;
	else return false;
}


void Exact_Put(char** words,int num,QueryID query_id){
	for(int i=0;i<num;i++){
		int bucket_num=hashing(words[i])%bucket_sizeofHashTableExact;
		bool val1=check_if_word_exists(words[i],bucket_num,query_id);
		if(val1==true) continue;
		float load_factor=counting_load_factor(HashTableExact->entries_counter+1,bucket_sizeofHashTableExact);
		if(load_factor>=0.8){
			int old_size=bucket_sizeofHashTableExact;
			bucket_sizeofHashTableExact=NextPrime(bucket_sizeofHashTableExact*2);
			struct Exact_Root* NewHashTableExact=malloc(sizeof(struct Exact_Root));
			bucket_num=hashing(words[i])%bucket_sizeofHashTableExact;
			NewHashTableExact->entries_counter=0;
			bucket_num=hashing(words[i])%bucket_sizeofHashTableExact;
			NewHashTableExact->array=NULL;
			NewHashTableExact->array=malloc(bucket_sizeofHashTableExact*sizeof(struct Exact_Node*));
			for(int j=0;j<bucket_sizeofHashTableExact;j++)
				NewHashTableExact->array[j]=NULL;
			for(int j=0;j<old_size;j++){
				if(HashTableExact->array[j]==NULL) continue;
				struct Exact_Node* start=HashTableExact->array[j];
				struct Exact_Node* next_start=start->next;
				while(1){
					struct payload_node* ptr_beg=start->beg;
					insert_HashTableExact_V2(NewHashTableExact,start->wd,bucket_num,ptr_beg);
					free(start->wd);
					free(start);
					start=next_start;
					if(start==NULL) break;
					next_start=next_start->next;
				}
			}
			free(HashTableExact->array);
			free(HashTableExact);
			HashTableExact=NewHashTableExact;
		}
		else insert_HashTableExact(words[i],bucket_num,query_id);
	}
}




void insert_HashTableExact(const char* word,int bucket_num,QueryID query_id){
	struct Exact_Node* node=NULL;
	node=malloc(sizeof(struct Exact_Node));
	node->next=NULL;
	node->wd=NULL;
	node->prev=NULL;
	node->beg=NULL;
	node->wd=malloc((strlen(word)+1)*sizeof(char));
	strcpy(node->wd,word);
	struct payload_node* Pnode=NULL;
	Pnode=malloc(sizeof(struct payload_node));
	Pnode->next=NULL;
	Pnode->query_id=query_id;
	node->beg=Pnode;
	struct Exact_Node* start=HashTableExact->array[bucket_num];
	if(start==NULL) HashTableExact->array[bucket_num]=node;
	else{
		while(1){
			if(start->next==NULL){
				start->next=node;
				node->prev=start;
				break;
			}
			start=start->next;
		}
	}
	HashTableExact->entries_counter++;
}


void insert_HashTableExact_V2(struct Exact_Root* head,char* word,int bucket_num,struct payload_node* payload_ptr){
	struct Exact_Node* node=malloc(sizeof(struct Exact_Node));
	node->next=NULL;
	node->wd=NULL;
	node->prev=NULL;
	node->beg=NULL;
	node->beg=payload_ptr;
	node->wd=malloc((strlen(word)+1)*sizeof(char));
	strcpy(node->wd,word);
	struct Exact_Node* start=head->array[bucket_num];
	if(start==NULL) head->array[bucket_num]=node;
	else{
		while(1){
			if(start->next==NULL){
				start->next=node;
				node->prev=start;
				break;
			}
			start=start->next;
		}
	}
	head->entries_counter++;
}



bool check_if_word_exists(char* word,int bucket_num,QueryID query_id){
	struct Exact_Node* start=HashTableExact->array[bucket_num];
	bool found=false;
	if(start==NULL) return found;
	while(1){
		if(!strcmp(word,start->wd)){
			struct payload_node* Pnode=NULL;
			Pnode=malloc(sizeof(struct payload_node));
			Pnode->next=NULL;
			Pnode->query_id=query_id;
			struct payload_node* s1=start->beg;
			if(s1==NULL) start->beg=Pnode;
			else{
				while(1){
					if(s1->next==NULL){
						s1->next=Pnode;
						break;
					}
					s1=s1->next;
				}
			}
			found=true;
			break;
		}
		start=start->next;
		if(start==NULL)
			break;
	}
	return found;
}


void Check_Exact_Hash_Array(QueryID query_id){
	for(int i=0;i<bucket_sizeofHashTableExact;i++){
		struct Exact_Node* start=HashTableExact->array[i];
		if(start==NULL) continue;
		while(1){
			delete_specific_payload(start,query_id);
			bool val=empty_of_payload_nodes(start);
			if(val==true){
				if(start==HashTableExact->array[i]){
					HashTableExact->array[i]=start->next;
					start->next->prev=NULL;
					free(start);
				}
				else{
					start->prev->next=start->next;
					free(start);
				}
			}
			if(start==NULL)
				break;
			start=start->next;
		}
	}
}



bool empty_of_payload_nodes(struct Exact_Node* node){
	if(node->beg==NULL) return true;
	else return false;
}

void delete_specific_payload(struct Exact_Node* node,QueryID query_id){
	struct payload_node* s1=node->beg;
	struct payload_node* s1_next=s1->next;
	if(s1->query_id==query_id){
		node->beg=s1_next;
		free(s1);
	}
	else{
		if(s1_next==NULL)
			return ;
		while(1){
			if(s1_next->query_id==query_id){
				s1->next=s1_next->next;
				free(s1_next);
				break;
			}
			s1=s1_next;
			if(s1==NULL)
				break;
			s1_next=s1_next->next;
		}
	}
}






void Edit_Put(char** words_ofquery,int words_num,QueryID query_id,unsigned int match_dist){
	//printf("query_id=%d\n",query_id);
	for(int i=0;i<words_num;i++){
		//printf("word=%s\n",words_ofquery[i]);
		build_entry_index_Edit(words_ofquery[i],query_id,match_dist);
	}
}


ErrorCode build_entry_index_Edit(char* word,QueryID query_id,unsigned int match_dist){
	//printf("-----------------------------------------------------------\n");
	//printf("-----------------------------------------------------------\n");
	//printf("-----------------------------------------------------------\n");
	//printf("word=%s\n",word);
	//printf("query_id=%u\n",query_id);
	//printf("match_dist=%u\n",match_dist);
	//printf("-----------------------------------------------------------\n");
	//printf("-----------------------------------------------------------\n");
	//printf("-----------------------------------------------------------\n");
	//printf("-----------------------------------------------------------\n");
	//printf("-----------------------------------------------------------\n");
	if(BKTreeIndexEdit->root==NULL){
		struct EditNode* node=NULL;
		node=malloc(sizeof(struct EditNode));
		node->firstChild=NULL;
		node->next=NULL;
		node->distance=0;
		node->wd=malloc((strlen(word)+1)*sizeof(char));
		strcpy(node->wd,word);
		node->start_info=NULL;
		struct Info* info_node=malloc(sizeof(struct Info));
		info_node->next=NULL;
		info_node->query_id=query_id;
		info_node->match_dist=match_dist;
		node->start_info=info_node;
		BKTreeIndexEdit->root=node;
	}
	else{
		struct EditNode* curr_node=BKTreeIndexEdit->root;
		while(1){
			int distance=0;
			distance=EditDistance(curr_node->wd,strlen(curr_node->wd),word,strlen(word));
			if(distance==0){
				struct Info* info_node=malloc(sizeof(struct Info));
				info_node->next=NULL;
				info_node->query_id=query_id;
				info_node->match_dist=match_dist;
				struct Info* start_Infonode=curr_node->start_info;
				//printf("begin\n");
				//printf("curr->wd=%s\n",curr_node->wd);
				//rintf("word=%s\n",word);
				if(start_Infonode==NULL){
					if(!strcmp(word,"airlines")){
						//printf("1\n");
						//printf("----------------------------------------------\n");
						//printf("curr_query_id=%u\n",info_node->query_id);
						//printf("curr_match_dist=%u\n",info_node->match_dist);
						//printf("----------------------------------------------\n");
					}
					curr_node->start_info=info_node;
				}
				else{
					while(1){
						if(start_Infonode->next==NULL){
							if(!strcmp(word,"airlines")){
								//printf("2\n");
								//printf("----------------------------------------------\n");
								//printf("curr_query_id=%u\n",info_node->query_id);
								//printf("curr_match_dist=%u\n",info_node->match_dist);
								//printf("----------------------------------------------\n");
							}
							start_Infonode->next=info_node;
							break;
						}
						start_Infonode=start_Infonode->next;
					}			
				}
				break;
			}
			int found=0;
			struct EditNode* target=NULL;
			for(struct EditNode* c=curr_node->firstChild;c!=NULL;c=c->next){
				if(distance==c->distance){
					found=1;
					target=c;
					break;
				}
			}
			if(found==1){
				curr_node=target;
				continue;
			}
			struct EditNode* new_node=malloc(sizeof(struct EditNode));
			new_node->next=NULL;
			new_node->firstChild=NULL;
			new_node->distance=distance;
			new_node->wd=NULL;
			new_node->wd=malloc((strlen(word)+1)*sizeof(char));
			strcpy(new_node->wd,word);
			struct Info* info_node=malloc(sizeof(struct Info));
			info_node->next=NULL;
			info_node->query_id=query_id;
			info_node->match_dist=match_dist;
			new_node->start_info=info_node;
			struct EditNode* c=curr_node->firstChild;
			if(c==NULL)
				curr_node->firstChild=new_node;
			else{
				while(1){
					if(c->next==NULL){
						c->next=new_node;
						break;
					}
					c=c->next;
				}	
			}
			break;
		}

	}	
	return EC_SUCCESS;
}



void Hamming_Put(char** words_ofquery,int words_num,QueryID query_id,unsigned int match_dist){
	for(int i=0;i<words_num;i++)
		build_entry_index_Hamming(words_ofquery[i],query_id,match_dist);
}

ErrorCode build_entry_index_Hamming(char* word,QueryID query_id,unsigned int match_dist){
	int size_of_word=strlen(word);
	int position_of_word=size_of_word-4;
	//printf("--%d\n",size_of_word);
	struct word_RootPtr* word_ptr=&HammingDistanceStructNode->word_RootPtrArray[position_of_word];
	//printf("%d\n",word_ptr->word_length);
	if(word_ptr->HammingPtr==NULL){
		word_ptr->HammingPtr=malloc(sizeof(struct HammingIndex));
		struct HammingNode* Hnode=NULL;
		Hnode=malloc(sizeof(struct HammingNode));
		Hnode->next=NULL;
		Hnode->firstChild=NULL;
		Hnode->distance=0;
		Hnode->wd=malloc((strlen(word)+1)*sizeof(char));
		strcpy(Hnode->wd,word);
		struct Info* info_node=malloc(sizeof(struct Info));
		info_node->next=NULL;
		info_node->query_id=query_id;
		info_node->match_dist=match_dist;
		Hnode->start_info=info_node;
		word_ptr->HammingPtr->root=Hnode;
	}
	else{
		struct HammingNode* curr_node=word_ptr->HammingPtr->root;
		while(1){
			int distance=0;
			distance=HammingDistance(curr_node->wd,strlen(curr_node->wd),word,strlen(word));
			if(distance==0){
				struct Info* info_node=malloc(sizeof(struct Info));
				info_node->next=NULL;
				info_node->query_id=query_id;
				info_node->match_dist=match_dist;
				struct Info* start_Infonode=curr_node->start_info;
				if(start_Infonode==NULL)
					curr_node->start_info=info_node;
				else{
					while(1){
						if(start_Infonode->next==NULL){
							start_Infonode->next=info_node;
							break;
						}
						start_Infonode=start_Infonode->next;
					}			
				}
				break;
			}
			int found=0;
			struct HammingNode* target=NULL;
			for(struct HammingNode* c=curr_node->firstChild;c!=NULL;c=c->next){
				if(distance==c->distance){
					found=1;
					target=c;
					break;
				}
			}
			if(found==1){
				curr_node=target;
				continue;
			}
			struct HammingNode* new_node=malloc(sizeof(struct HammingNode));
			new_node->next=NULL;
			new_node->firstChild=NULL;
			new_node->distance=distance;
			new_node->wd=NULL;
			new_node->wd=malloc((strlen(word)+1)*sizeof(char));
			strcpy(new_node->wd,word);
			struct Info* info_node=malloc(sizeof(struct Info));
			info_node->next=NULL;
			info_node->query_id=query_id;
			info_node->match_dist=match_dist;
			new_node->start_info=info_node;
			struct HammingNode* c=curr_node->firstChild;
			if(c==NULL)
				curr_node->firstChild=new_node;
			else{
				while(1){
					if(c->next==NULL){
						c->next=new_node;
						break;
					}
					c=c->next;
				}	
			}
			break;
		}
	}
	return EC_SUCCESS;
}


void Check_Edit_BKTree(QueryID query_id){
	struct EditNode* beg_node=BKTreeIndexEdit->root;
	Delete_Query_from_Edit_Nodes(beg_node,query_id);		
}

void Delete_Query_from_Edit_Nodes(struct EditNode* node,QueryID query_id){
	for(struct EditNode* child=node->firstChild;child!=NULL;child=child->next)
		Delete_Query_from_Edit_Nodes(child,query_id);
	struct Info* info_node=node->start_info;
	if(info_node==NULL)
		return ;
	if(info_node->query_id==query_id){
		struct Info* delete_node=info_node;
		info_node=info_node->next;
		free(delete_node);
	}
	else{
		struct Info* info_node=node->start_info;
		struct Info* next_node=node->start_info->next;
		if(next_node==NULL)
			return ;
		while(1){
			if(next_node->query_id==query_id){
				info_node->next=next_node->next;
				free(next_node);
			}
			info_node=next_node;
			if(info_node==NULL)
				break;
			next_node=next_node->next;
		}
	}
}

void Check_Hamming_BKTrees(QueryID query_id){
	int HammingIndexSize=(MAX_WORD_LENGTH-MIN_WORD_LENGTH)+1;
	for(int i=0;i<HammingIndexSize;i++){
		Delete_Query_from_Hamming_Nodes(HammingDistanceStructNode->word_RootPtrArray[i].HammingPtr->root,query_id);
	}
}

void Delete_Query_from_Hamming_Nodes(struct HammingNode* node,QueryID query_id){
	for(struct HammingNode* child=node->firstChild;child!=NULL;child=child->next)
		Delete_Query_from_Hamming_Nodes(child,query_id);
	struct Info* info_node=node->start_info;
	if(info_node==NULL)
		return ;
	if(info_node->query_id==query_id){
		struct Info* delete_node=info_node;
		info_node=info_node->next;
		free(delete_node);
	}
	else{
		struct Info* info_node=node->start_info;
		struct Info* next_node=node->start_info->next;
		if(next_node==NULL)
			return ;
		while(1){
			if(next_node->query_id==query_id){
				info_node->next=next_node->next;
				free(next_node);
			}
			info_node=next_node;
			if(info_node==NULL)
				break;
			next_node=next_node->next;
		}
	}
}

char** words_ofquery(const char* query_str,int* num){
	char curr_words[5][MAX_WORD_LENGTH];
	int coun=0;
	char word[MAX_WORD_LENGTH];
	int len=0;
	for(int i=0;i<strlen(query_str);i++){
		if(query_str[i]==' '){
			word[len]='\0';
			len=0;
			strcpy(curr_words[coun],word);
			coun++;
			memset(word,0,MAX_WORD_LENGTH);
			continue;
		}
		if(i==strlen(query_str)-1){
			word[len++]=query_str[i];
			word[len]='\0';
			len=0;
			strcpy(curr_words[coun],word);
			coun++;
			memset(word,0,MAX_WORD_LENGTH);
			continue;
		}
		word[len++]=query_str[i];
	}
	char** returning_array=malloc(coun*sizeof(char*));
	for(int i=0;i<coun;i++)
		returning_array[i]=malloc(MAX_WORD_LENGTH*sizeof(char));
	for(int i=0;i<coun;i++)
		strcpy(returning_array[i],curr_words[i]);
	*num=coun;
	return returning_array;
}


ErrorCode create_entry_list(entry_list** el){
	if(*el!=NULL)
		return EC_FAIL;
	entry_list* node=malloc(sizeof(entry_list));
	node->first_node=NULL;
	node->current_node=NULL;
	node->counter=0;
	*el=node;
	return EC_SUCCESS;

}


Entry*  Exact_Result(char* word,int* num1){
	Entry* beg_ptr=NULL;
	int coun=0;
	for(int i=0;i<bucket_sizeofHashTableExact;i++){
		struct Exact_Node* start=HashTableExact->array[i];
		if(start==NULL)
			continue;
		while(1){
			if(!strcmp(start->wd,word)){
				coun++;
				Entry* en=Put_data(start);
				//printf("%s\n",en->my_word);
				if(beg_ptr==NULL)
					beg_ptr=en;
				else{
					Entry* s1=beg_ptr;
					while(1){
						if(s1->next==NULL){
							s1->next=en;
							break;
						}
						s1=s1->next;
					}
				}
			}
			start=start->next;
			if(start==NULL)
				break;
		}
	}
	*num1=coun;
	return beg_ptr;
}

Entry* Put_data(struct Exact_Node* node){
	Entry* en=malloc(sizeof(Entry));
	en->next=NULL;
	en->my_word=malloc((strlen(node->wd)+1)*sizeof(char));
	strcpy(en->my_word,node->wd);
	en->payload=NULL;
	payload_node* start1=node->beg;
	while(1){
		if(start1==NULL)
			break;
		payload_node* pnode=malloc(sizeof(payload_node));
		pnode->next=NULL;
		pnode->query_id=start1->query_id;
		if(en->payload==NULL)
			en->payload=pnode;
		else{
			payload_node* s2=en->payload;
			while(1){
				if(s2->next==NULL){
					s2->next=pnode;
					break;
				}
				s2=s2->next;
			}
		}
		start1=start1->next;
	}
	return en;
}

void push_stack_edit(struct Edit_Stack_Node** list, struct EditNode** n){
    struct Edit_Stack_Node* temp = malloc(sizeof(struct Edit_Stack_Node));
    temp->node = *n;
    temp->next = *list;
    *list = temp;
}
struct EditNode* pop_stack_edit(struct Edit_Stack_Node** list){
    struct Edit_Stack_Node* temp = *list;
    struct EditNode* ret = (*list)->node;
    *list = (*list)->next;
    free(temp);
    return ret;
}

void push_stack_hamming(struct Hamming_Stack_Node** list, struct HammingNode** n){
    struct Hamming_Stack_Node* temp = malloc(sizeof(struct Hamming_Stack_Node));
    temp->node = *n;
    temp->next = *list;
    *list = temp;
}
struct HammingNode* pop_stack_hamming(struct Hamming_Stack_Node** list){
    struct Hamming_Stack_Node* temp = *list;
    struct HammingNode* ret = (*list)->node;
    *list = (*list)->next;
    free(temp);
    return ret;
}

Entry* Edit_Result(char* word,int* num){
	if(BKTreeIndexEdit == NULL){
		return NULL;
	}
	if(BKTreeIndexEdit->root == NULL){
		return NULL;
	}
	int d, bot, ceil;
	Entry* result_list = NULL;
	Entry* new_entry = result_list;
	struct EditNode* curr;
	struct Edit_Stack_Node* candidate_list = NULL;
	push_stack_edit(&candidate_list, &(BKTreeIndexEdit->root));
	struct EditNode* children = NULL;
	struct Info* info;
	struct payload_node* pl;
	while(candidate_list != NULL){
		curr = pop_stack_edit(&candidate_list);
		info = curr->start_info;
		d = EditDistance(word, strlen(word), curr->wd, strlen(curr->wd));
		while(info != NULL){
			if(d <= info->match_dist){
				//printf("d=%u\n",d);
				//printf("word=%s\n",word);
				//printf("curr->wd=%s\n",curr->wd);
				//printf("info->query_id=%d\n",info->query_id);
				//printf("info->match_dis=%d\n",info->match_dist);
				if(new_entry == NULL){
					//printf("enterrring--------------------------------------------------\n");
					(*num)++;
					new_entry = malloc(sizeof(Entry));
<<<<<<< HEAD
					new_entry->next = NULL;
=======
					new_entry->next=NULL;
>>>>>>> a6d49bdff594de5f4e7df5e004042c9c4477e249
					new_entry->my_word = malloc(sizeof(char)*(strlen(word)+1));
					strcpy(new_entry->my_word, word);
					new_entry->payload = malloc(sizeof(payload_node));
					new_entry->payload->query_id = info->query_id;
					new_entry->payload->next = NULL;
					pl = new_entry->payload->next;
				}
				else{
					pl = malloc(sizeof(payload_node));
					pl->query_id = info->query_id;
					pl->next = NULL;
					pl = pl->next;
				}
			}
			info = info->next;
		}
		if(new_entry != NULL){
			new_entry = new_entry->next;
		}
		if(d <= MAX_MATCH_DIST){
			bot = MAX_MATCH_DIST - d;
		}else{
			bot = d - MAX_MATCH_DIST;
		}
		ceil = d + MAX_MATCH_DIST;
		children = curr->firstChild;
		while(children != NULL){
			if(children->distance >= bot && children->distance <= ceil){
				push_stack_edit(&candidate_list, &children);
			}
			children = children->next;
		}
	}
	if(new_entry!=NULL){
		printf("new entry not null\n");
	}
	return result_list;
}

Entry* Hamming_Result(char* word,int* num){
	int position = strlen(word) - 4;
	if(HammingDistanceStructNode->word_RootPtrArray[position].HammingPtr == NULL){
		return NULL;
	}
	struct HammingNode* tree = 	HammingDistanceStructNode->word_RootPtrArray[position].HammingPtr->root;
	if(tree == NULL){
		return NULL;
	}
	int d, bot, ceil;
	Entry* result_list = NULL;
	Entry* new_entry = result_list;
	struct HammingNode* curr;
	struct Hamming_Stack_Node* candidate_list = NULL;
	push_stack_hamming(&candidate_list, &tree);
	struct HammingNode* children = NULL;
	struct Info* info;
	struct payload_node* pl;
	while(candidate_list != NULL){
		curr = pop_stack_hamming(&candidate_list);
		info = curr->start_info;
		d = HammingDistance(word, strlen(word), curr->wd, strlen(curr->wd));
		while(info != NULL){
			if(d <= info->match_dist){
				//printf("HAMING \n");
				//printf("d=%u\n",d);
				//printf("word=%s\n",word);
				//printf("curr->wd=%s\n",curr->wd);
				//printf("info->query_id=%d\n",info->query_id);
				//printf("info->match_dis=%d\n",info->match_dist);
				if(new_entry == NULL){
					//printf("enter\n");
					(*num)++;
					new_entry = malloc(sizeof(Entry));
<<<<<<< HEAD
					new_entry->next = NULL;
=======
					new_entry->next=NULL;
>>>>>>> a6d49bdff594de5f4e7df5e004042c9c4477e249
					new_entry->my_word = malloc(sizeof(char)*(strlen(word)+1));
					strcpy(new_entry->my_word, word);
					new_entry->payload = malloc(sizeof(payload_node));
					new_entry->payload->query_id = info->query_id;
					new_entry->payload->next = NULL;
					pl = new_entry->payload->next;
				}
				else{
<<<<<<< HEAD
					pl = malloc(sizeof(payload_node));
					pl->query_id = info->query_id;
					pl->next = NULL;
					pl = pl->next;
=======
					//printf("Nikos oikonomopoulos\n");
					new_entry->payload->next = malloc(sizeof(payload_node));
					new_entry->payload->query_id = info->query_id;
>>>>>>> a6d49bdff594de5f4e7df5e004042c9c4477e249
				}
			}
			info = info->next;
		}
		if(new_entry != NULL){
			new_entry = new_entry->next;
		}
		if(d <= MAX_MATCH_DIST){
			bot = MAX_MATCH_DIST - d;
		}else{
			bot = d - MAX_MATCH_DIST;
		}
		ceil = d + MAX_MATCH_DIST;
		children = curr->firstChild;
		while(children != NULL){
			if(children->distance >= bot && children->distance <= ceil){
				push_stack_hamming(&candidate_list, &children);
			}
			children = children->next;
		}
	}
	if(new_entry==NULL){
		//printf("word=%s\n",word);
		printf("empty new\n");
	}
	if(new_entry!=NULL){
		printf("new entry not null\n");
	}
	return result_list;
}



void Delete_Query_from_Active_Queries(QueryID query_id){
	struct Query_Info* start=ActiveQueries;
	if(start->query_id==query_id){
		struct Query_Info* query_node=start->next;
		free(start);
		start=query_node;
		return;
	}
	struct Query_Info* next_start=ActiveQueries->next;
	while(1){
		if(next_start->query_id==query_id){
			start->next=next_start->next;
			free(next_start);
		}
		start=next_start;
		if(start==NULL)
			break;
		next_start=next_start->next;
	}
}

void Put_query_on_Active_Queries(QueryID query_id,int words_num){
	struct Query_Info* start=ActiveQueries;
	struct Query_Info* node=malloc(sizeof(struct Query_Info));
	node->next=NULL;
	node->query_id=query_id;
	node->counter_of_distinct_words=words_num;
	if(start==NULL){
		start=node;
		return ;
	}
	while(1){
		if(start->next==NULL){
			start->next=node;
			break;
		}
		start=start->next;
	}
}



unsigned int hash_interger(unsigned int x){
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}



void Delete_Result_List(Entry* en){
	Entry* start1=en;
	if(en==NULL)
		return ;
	Entry* start1_next=en->next;
	while(1){
		payload_node* k1=start1->payload;
		payload_node* k2=k1->next;
		while(1){
			free(k1);
			k1=k2;
			if(k1==NULL)
				break;
			k2=k2->next;
		}
		free(start1->my_word);
		free(start1);
		start1=start1_next;
		if(start1==NULL)
			break;
		start1_next=start1_next->next;
	}
}




QueryID* Put_On_Result_Hash_Array(Entry* en1,Entry* en2,Entry* en3,int num1,int num2,int num3,int* result_counter){
	int sum=num1+num2+num3;
	float curr_size=sum/0.8;
	int size=(int)curr_size;
	size=NextPrime(size);
	struct Result_Hash_Node** hash_array=malloc(size*sizeof(struct Result_Hash_Node*));
	for(int i=0;i<size;i++)
		hash_array[i]=NULL;
	Entry* s1=en1;
	while(1){
		if(s1==NULL)
			break;
		char* the_word=s1->my_word;
		payload_node* p1=s1->payload;
		while(1){
			if(p1==NULL)
				break;
			QueryID q=p1->query_id;
			int bucket_size=hash_interger(q)%size;
			struct Result_Hash_Node* r1=hash_array[bucket_size];
			Hash_Put_Result(q,the_word,&r1);
			p1=p1->next;
		}
		s1=s1->next;
	}
	s1=en2;
	while(1){
		if(s1==NULL)
			break;
		char* the_word=s1->my_word;
		payload_node* p1=s1->payload;
		while(1){
			if(p1==NULL)
				break;
			QueryID q=p1->query_id;
			int bucket_size=hash_interger(q)%size;
			struct Result_Hash_Node* r1=hash_array[bucket_size];
			Hash_Put_Result(q,the_word,&r1);
			p1=p1->next;
		}
		s1=s1->next;
	}
	s1=en3;
	while(1){
		if(s1==NULL)
			break;
		char* the_word=s1->my_word;
		payload_node* p1=s1->payload;
		while(1){
			if(p1==NULL)
				break;
			QueryID q=p1->query_id;
			int bucket_size=hash_interger(q)%size;
			struct Result_Hash_Node* r1=hash_array[bucket_size];
			Hash_Put_Result(q,the_word,&r1);
			p1=p1->next;
		}
		s1=s1->next;
	}
	struct Info* result_list=NULL;
	int length_final_array=0;
	struct Query_Info* qnode=ActiveQueries;
	while(1){
		if(qnode==NULL) 
			break;
		int correct_distinct_words=qnode->counter_of_distinct_words;
		QueryID q=qnode->query_id;
		int bucket_num=hash_interger(q)%size;
		struct Result_Hash_Node* rhn1=hash_array[bucket_num];
		while(1){
			if(rhn1==NULL)
				break;
			if(rhn1->query_id==q){
				int coun1=rhn1->distinct_words;
				if(coun1==correct_distinct_words){
					struct Info* info_node=malloc(sizeof(struct Info));
					info_node->query_id=q;
					info_node->next=NULL;
					length_final_array++;
					if(result_list==NULL)
						result_list=info_node;
					else{
						struct Info* start_result=result_list;
						while(1){
							if(start_result->next==NULL){
								start_result->next=info_node;
								break;
							}
							start_result=start_result->next;
						}
					}
				}
			}
			rhn1=rhn1->next;
		}
		qnode=qnode->next;
	}
	// edw free to pinka katakermatismou
	QueryID* final=malloc(length_final_array*sizeof(QueryID));
	struct Info* start_result=result_list;
	int i=0;
	while(1){
		if(start_result==NULL)
			break;
		final[i++]=start_result->query_id;
		start_result=start_result->next;
	}
	free(hash_array);
	*result_counter=length_final_array;
	return final;
}








void Hash_Put_Result(QueryID q,char* word,struct Result_Hash_Node** rr1){
	struct Result_Hash_Node* ptr1=*rr1;
	if(ptr1==NULL){
		struct Result_Hash_Node* new_node=malloc(sizeof(struct Result_Hash_Node));
		new_node->next=NULL;
		new_node->query_id=q;
		new_node->distinct_words=1;
		struct word_node* wn=malloc(sizeof(struct word_node));
		wn->next=NULL;
		wn->word=malloc((strlen(word)+1)+sizeof(char));
		strcpy(wn->word,word);
		new_node->word_start=wn;
		*rr1=new_node;
		return ;
	}
	struct Result_Hash_Node* ptr2=*rr1;
	int found=0;
	while(1){
		if(ptr2->query_id==q){
			found=1;
			break;
		}
		ptr2=ptr2->next;
		if(ptr2==NULL)
			break;
	}
	if(found==1){
		int found2=0;
		struct word_node* s1=ptr2->word_start;
		while(1){
			if(!strcmp(s1->word,word)){
				found2=1;
				break;
			}
			s1=s1->next;
		}
		if(found2==0){
			struct word_node* wn=malloc(sizeof(struct word_node));
			wn->next=NULL;
			wn->word=malloc((strlen(word)+1)+sizeof(char));
			strcpy(wn->word,word);
			ptr2->distinct_words++;
			struct word_node* s1=ptr2->word_start;
			while(1){
				if(s1->next==NULL){
					s1->next=wn;
					break;
				}
				s1=s1->next;
			}
		}
	}
	else{
		struct Result_Hash_Node* new_node=malloc(sizeof(struct Result_Hash_Node));
		new_node->next=NULL;
		new_node->query_id=q;
		new_node->distinct_words=1;
		struct word_node* wn=malloc(sizeof(struct word_node));
		wn->next=NULL;
		wn->word=malloc((strlen(word)+1)+sizeof(char));
		strcpy(wn->word,word);
		new_node->word_start=wn;
		struct Result_Hash_Node* ptr2=*rr1;
		while(1){
			if(ptr2->next==NULL){
				ptr2->next=new_node;
				break;
			}
			ptr2=ptr2->next;
		}
	}
}














void Put_On_Stack_Result(DocID docID,int size,QueryID* query_array){
	struct result* node=malloc(sizeof(struct result));
	node->doc_id=docID;
	node->result_counter=size;
	node->query_id=malloc(node->result_counter*sizeof(QueryID));
	node->next=NULL;
	for(int i=0;i<size;i++)
		node->query_id[i]=query_array[i];
	node->next=StackArray->top;
	StackArray->top=node;
	if(StackArray->first==NULL)
		StackArray->first=node;
}


void Delete_From_Stack(){
	struct result* node=StackArray->top;
	StackArray->top=StackArray->top->next;
	if(StackArray->top==NULL)
		StackArray->first=NULL;
	free(node->query_id);
	free(node);
}