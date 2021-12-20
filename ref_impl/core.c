#include "../include/core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

/*Global Variables*/
struct HammingDistanceStruct* HammingDistanceStructNode;
Index*  BKTreeIndexEdit;
struct Query_Info* ActiveQueries;
struct Exact_Root* HashTableExact;
int bucket_sizeofHashTableExact;
unsigned int active_queries;
struct Stack_result* StackArray;
/*******************/


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


ErrorCode InitializeIndex(){
	active_queries=0;
	/*StackArray contains for each id the queries that matching*/
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
	if(StackArray==NULL) return EC_FAIL;
	if(BKTreeIndexEdit==NULL) return EC_FAIL;
	if(HashTableExact==NULL) return EC_FAIL;
	if(HammingDistanceStructNode==NULL) return EC_FAIL;
	return EC_SUCCESS;
}

ErrorCode DestroyIndex(){
	int HammingIndexSize=(MAX_WORD_LENGTH-MIN_WORD_LENGTH)+1;
	/*Free the Exact_Hash_Array Nodes*/
	for(int i=0; i<bucket_sizeofHashTableExact; i++){
		if(HashTableExact->array[i] == NULL) continue;
		struct Exact_Node* start = HashTableExact->array[i];
		struct Exact_Node* start_next = start->next;
		while(1){
			struct payload_node* p_start=start->beg;
			struct payload_node* p_start_next=p_start->next;
			while(1){
				free(p_start);
				p_start=p_start_next;
				if(p_start==NULL)
					break;
				p_start_next=p_start_next->next;
			}
			free(start->wd);
			free(start);
			start = start_next;
			if(start == NULL) break;
			start_next = start_next->next;
		}
	}
	Free_Active_Queries();
	free(StackArray);
	free(HashTableExact->array);
	free(HashTableExact);
	destroy_Edit_index(BKTreeIndexEdit);
	for(int i=0;i<HammingIndexSize;i++)
		destroy_hamming_entry_index(HammingDistanceStructNode->word_RootPtrArray[i].HammingPtr);
	free(HammingDistanceStructNode->word_RootPtrArray);
	free(HammingDistanceStructNode);	
	return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	active_queries++;
	int words_num=0;
	char** query_words=words_ofquery(query_str,&words_num);
	/*Return for each query the words*/
	Put_query_on_Active_Queries(query_id,words_num);
	/*Put specific query to active queries*/
	/*Add words to specific matching node*/
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
	//bool found = false;/////////////
	active_queries--;
	Delete_Query_from_Active_Queries(query_id);
	/*check if query exists on ExactHashTable*/
	//found = 
	Check_Exact_Hash_Array(query_id);
	//if(!found){/////
	/*check if query exists on EditBKTree*/
	Check_Edit_BKTree(query_id);
	/*check if query exists on HammingBKTrees*/
	Check_Hamming_BKTrees(query_id);
	///}
	return EC_SUCCESS;
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	struct Match_Type_List* Final_List=malloc(sizeof(struct Match_Type_List));
	Final_List->start=NULL;
	Final_List->cur=NULL;
	Final_List->counter=0;
	int words_num=0;
	char** words_oftext=Deduplicate_Method(doc_str,&words_num);
	int num_result=0;
	/*Appending the results for each word to Final_List*/
	for(int i=0;i<words_num;i++){
		/*Check exact node*/
		struct Match_Type_List* Exact_Node=Exact_Result(words_oftext[i]);
		if(Final_List->start==NULL){
			Final_List->start=Exact_Node->start;
			Final_List->cur=Exact_Node->cur;
		}
		else{
			if(Exact_Node->start!=NULL){
				Final_List->cur->next=Exact_Node->start;
				Final_List->cur=Exact_Node->cur;
			}
		}
		/*Check edit index*/
		Final_List->counter+=Exact_Node->counter;
		struct Match_Type_List* Edit_Node=Edit_Result(words_oftext[i]);
		if(Final_List->start==NULL){
			Final_List->start=Edit_Node->start;
			Final_List->cur=Edit_Node->cur;
		}
		else{
			if(Edit_Node->start!=NULL){
				Final_List->cur->next=Edit_Node->start;
				Final_List->cur=Edit_Node->cur;
			}
		}
		/*Check hamming indexes*/
		Final_List->counter+=Edit_Node->counter;
		struct Match_Type_List* Hamming_Node=Hamming_Result(words_oftext[i]);
		if(Final_List->start==NULL){
			Final_List->start=Hamming_Node->start;
			Final_List->cur=Hamming_Node->cur;
		}
		else{
			if(Hamming_Node->start!=NULL){
				Final_List->cur->next=Hamming_Node->start;
				Final_List->cur=Hamming_Node->cur;
			}
		}
		Final_List->counter+=Hamming_Node->counter;
	}
	QueryID* query_id_result=Put_On_Result_Hash_Array(Final_List,&num_result);
	/*Put the queries(array) that matching to Stack of Results*/
	Put_On_Stack_Result(doc_id,num_result,query_id_result);
	Delete_Result_List(Final_List);
	for(int i=0;i<words_num;i++)
		free(words_oftext[i]);
	free(words_oftext);
	free(Final_List);
	return EC_SUCCESS;
}


ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{	
	/*Get From top node its info and put it on input variables*/
	DocID doc=StackArray->top->doc_id;
	*p_doc_id=doc;
	unsigned int counter=StackArray->top->result_counter;
	*p_num_res=counter;
	QueryID* curr=malloc(StackArray->top->result_counter*sizeof(QueryID));
	for(int i=0;i<StackArray->top->result_counter;i++)
		curr[i]=StackArray->top->query_id[i];
	*p_query_ids=curr;
	/*Delete the top node from stack */
	Delete_From_Stack();
	return EC_SUCCESS;
}



/*Function returns Next prime of given input number */
int NextPrime(int N)
{
    bool found = false;
    while(!found){
        found = isPrime(N);
        N++;
    }
    return (N - 1);
}

/*Function check if specific number is prime or not */
bool isPrime(int N){
    
    for(int i = 2; i <= (N/2); i++){
        if(N % i == 0) return false;
    }
    return true;
}


/*Hash Function for Strings*/
unsigned int hashing(const char *word) {
    unsigned int hash = 0, c;

    for (size_t i = 0; word[i] != '\0'; i++) {
        c = (unsigned char)word[i];
        hash = (hash << 3) + (hash >> (sizeof(hash) * CHAR_BIT - 3)) + c;
    }
    return hash;
}

/*Compute load factor */
float counting_load_factor(int n, int k){
	float re;
	re=(float)n/(float)k;
	return re;

}

char** Deduplicate_Method(const char* query_str,int* size){
	int BucketsHashTable=10;
	/*start size of hash array*/
	char** word_array=NULL;
	struct Deduplicate_Hash_Array* Deduplication_Array=Initialize_Hash_Array(BucketsHashTable);
	char word[MAX_WORD_LENGTH];
	int len=0;
	for(int i=0;i<strlen(query_str);i++){
		if(query_str[i]==' '){
			word[len]='\0';
			len=0;
			/*Search the hash_array if word exists or not*/
			if(search_hash_array(Deduplication_Array,BucketsHashTable,word)==true){
				memset(word,0,MAX_WORD_LENGTH);
				continue;
			}
			float load_factor=counting_load_factor(Deduplication_Array->entries_counter+1,BucketsHashTable);
			/* if load_factor is 0.8 or upper it means that hash_array must be rebuild with new bucket_num size*/
			/* else just put the word to Hash_Array*/
			if(load_factor>=0.8){
				int old_bucket_num=BucketsHashTable;
				BucketsHashTable=NextPrime(BucketsHashTable*2);
				struct Deduplicate_Hash_Array* new_Deduplicate_Hash_Array=Initialize_Hash_Array(BucketsHashTable);
				/*Put all current records to new  Hash_Array and also delete old Hash_Array */
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
			/* if load_factor is 0.8 or upper it means that hash_array must be rebuild with new bucket_num size*/
			/* else just put the word to Hash_Array*/
			if(load_factor>=0.8){
				int old_bucket_num=BucketsHashTable;
				BucketsHashTable=NextPrime(BucketsHashTable*2);
				struct Deduplicate_Hash_Array* new_Deduplicate_Hash_Array=Initialize_Hash_Array(BucketsHashTable);
				/*Put all current records to new  Hash_Array and also delete old Hash_Array */
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
	/*Put on word_array the distinct words of Query and return it*/
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


/*Function that destroys all Nodes of Edit Index*/
ErrorCode destroy_Edit_index(Index* ix){
	if(ix==NULL) return EC_SUCCESS;
	destroy_Edit_nodes(ix->root);
	free(ix);
	ix=NULL;
	if(ix!=NULL) return EC_FAIL;	
	return EC_SUCCESS;
}

/*Function that destroys all Nodes of Hamming Indexes*/
ErrorCode destroy_hamming_entry_index(struct HammingIndex* ix){
	if(ix==NULL) return EC_SUCCESS;
	destroy_hamming_nodes(ix->root);
	free(ix);
	ix=NULL;
	if(ix!=NULL) return EC_FAIL;	
	return EC_SUCCESS;
}

/*Destroy hamming node and its children*/
void destroy_hamming_nodes(struct HammingNode* node){
	for(struct HammingNode* first=node->firstChild;first!=NULL;first=first->next)
		destroy_hamming_nodes(first);
	struct Info* start=node->start_info;
	if(start==NULL){
		free(node->wd);
		free(node);
		return ;
	}
	struct Info* start_next=start->next;
	while(1){
		if(start==NULL)
			break;
		free(start);
		start=start_next;
		if(start==NULL)
			break;
		start_next=start_next->next;
	}
	free(node->wd);
	free(node);
}


/*Destroy edit node and its children*/
void destroy_Edit_nodes(struct EditNode* node){
	for(struct EditNode* first=node->firstChild;first!=NULL;first=first->next)
		destroy_Edit_nodes(first);
	struct Info* start=node->start_info;
	if(start==NULL){
		free(node->wd);
		free(node);
		return ;
	}
	struct Info* start_next=start->next;
	while(1){
		if(start==NULL)
			break;
		free(start);
		start=start_next;
		if(start==NULL)
			break;
		start_next=start_next->next;
	}
	free(node->wd);
	free(node);
}


/*Free Deduplication Array*/
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

/*Initialize Deduplication Array*/
struct Deduplicate_Hash_Array* Initialize_Hash_Array(int BucketsHashTable){
	struct Deduplicate_Hash_Array* Deduplication_Array=malloc(sizeof(struct Deduplicate_Hash_Array));
	Deduplication_Array->entries_counter=0;
	Deduplication_Array->array=NULL;
	Deduplication_Array->array=malloc(BucketsHashTable*sizeof(struct Deduplicate_Node));
	for(int i=0;i<BucketsHashTable;i++)
		Deduplication_Array->array[i]=NULL;
	return Deduplication_Array;
}


/*Insert on Deduplication Array the specific word*/
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

/*check if specific word exists on Hash_Array*/
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


/*Put each word of query to Exact Hash Struct*/
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
			insert_HashTableExact(words[i],bucket_num,query_id);
		}
		else insert_HashTableExact(words[i],bucket_num,query_id);
	}
}



/*Function put to Global Hash Array  a record*/
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

/*Function put to head Hash Array a record*/
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


void Check_Exact_Hash_Array(QueryID query_id){/////void
	//bool ret = false;//////
	for(int i=0;i<bucket_sizeofHashTableExact;i++){
		struct Exact_Node* start=HashTableExact->array[i];
		if(start==NULL) continue;
		while(1){
			//if(delete_specific_payload(&start,query_id)){
			delete_specific_payload(&start,query_id);
			//	ret = true;//////
			//}///
			bool val=empty_of_payload_nodes(start);
			if(val==true){
				if(start==HashTableExact->array[i]){
					if(start->next!=NULL){
						HashTableExact->array[i]->prev=NULL;
						HashTableExact->array[i]=start->next;
					}
					else{
						HashTableExact->array[i]=NULL;
					}
					free(start->wd);
					free(start);
				}
				else{
					struct Exact_Node* delete_node=start;
					if(start->next!=NULL){
						start->next->prev=start->prev;
						start->next->prev->next=start->next;
						free(delete_node->wd);
						free(delete_node);
					}
					else{
						free(delete_node->wd);
						free(delete_node);
						start->prev->next=NULL;
					}
				}
			}
			start=start->next;
			if(start==NULL)
				break;
		}
	}
	//return ret;////
}


/*check if specific node is empty or not*/
bool empty_of_payload_nodes(struct Exact_Node* node){
	if(node->beg==NULL) return true;
	else return false;
}


void delete_specific_payload(struct Exact_Node** node,QueryID query_id){/////void
	struct payload_node* s1=(*node)->beg;
	if((*node)->beg==NULL)
		return ;//false;/////return
	struct payload_node* s1_next=s1->next;
	if(s1->query_id==query_id){
		(*node)->beg=s1_next;
		free(s1);
		//return true;/////
	}
	else{
		if(s1_next==NULL)
			return ;//false;////return
		while(1){
			if(s1_next==NULL)
				break;
			if(s1_next->query_id==query_id){
				s1->next=s1_next->next;
				free(s1_next);
				break;//return true;///////break
			}
			s1=s1_next;
			if(s1==NULL)
				break;
			s1_next=s1_next->next;
		}
	}
	//return false;//////
	
}


/*Put each word of query to Edit Index*/
void Edit_Put(char** words_ofquery,int words_num,QueryID query_id,unsigned int match_dist){
	for(int i=0;i<words_num;i++){
		build_entry_index_Edit(words_ofquery[i],query_id,match_dist);
	}
}


ErrorCode build_entry_index_Edit(char* word,QueryID query_id,unsigned int match_dist){
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
			/*check distance and if is zero word have been already put*/
			if(distance==0){
				/*Just add the query_id*/
				struct Info* info_node=malloc(sizeof(struct Info));
				info_node->next=NULL;
				info_node->query_id=query_id;
				info_node->match_dist=match_dist;
				struct Info* start_Infonode=curr_node->start_info;
				if(start_Infonode==NULL){
					curr_node->start_info=info_node;
				}
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
			/*check if exist a child with specific distance*/
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
			/*Create the node with specific word and specific query_id*/
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



/*Put each word of query to Hamming Node*/
void Hamming_Put(char** words_ofquery,int words_num,QueryID query_id,unsigned int match_dist){
	for(int i=0;i<words_num;i++)
		build_entry_index_Hamming(words_ofquery[i],query_id,match_dist);
}

ErrorCode build_entry_index_Hamming(char* word,QueryID query_id,unsigned int match_dist){
	int size_of_word=strlen(word);
	int position_of_word=size_of_word-4;
	struct word_RootPtr* word_ptr=&HammingDistanceStructNode->word_RootPtrArray[position_of_word];
	/*Navigate to right Index*/
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
			/*check distance and if is zero word have been already put*/
			distance=HammingDistance(curr_node->wd,strlen(curr_node->wd),word,strlen(word));
			if(distance==0){
				/*Just add the query_id*/
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
			/*check if exist a child with specific distance*/
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
			/*Create the node with specific word and specific query_id*/
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


/*Delete from Edit Index the specific query_id*/
void Check_Edit_BKTree(QueryID query_id){
	struct EditNode* beg_node=BKTreeIndexEdit->root;
	Delete_Query_from_Edit_Nodes(beg_node,query_id);		
}


/*Delete for each edit node and its children the specific query_id*/
void Delete_Query_from_Edit_Nodes(struct EditNode* node,QueryID query_id){
	for(struct EditNode* child=node->firstChild;child!=NULL;child=child->next)
		Delete_Query_from_Edit_Nodes(child,query_id);
	struct Info* info_node=node->start_info;
	if(info_node==NULL)
		return ;
	/*Base Case*/
	if(info_node->query_id==query_id){
		struct Info* delete_node=info_node;
		node->start_info=node->start_info->next;
		free(delete_node);
	}
	else{
		struct Info* info_node=node->start_info;
		struct Info* next_node=node->start_info->next;
		if(next_node==NULL)
			return ;
		while(1){
			if(next_node==NULL)
				break;
			if(next_node->query_id==query_id){
				info_node->next=next_node->next;
				free(next_node);
				break;
			}
			info_node=next_node;
			next_node=next_node->next;
		}
	}
}

/*Delete from  all roots of Hamming Node of Indexes the specific query_id*/
void Check_Hamming_BKTrees(QueryID query_id){
	int HammingIndexSize=(MAX_WORD_LENGTH-MIN_WORD_LENGTH)+1;
	for(int i=0;i<HammingIndexSize;i++){
		if(HammingDistanceStructNode->word_RootPtrArray[i].HammingPtr==NULL)
			continue;
		Delete_Query_from_Hamming_Nodes(HammingDistanceStructNode->word_RootPtrArray[i].HammingPtr->root,query_id);
	}
}



/*Delete for each hamming node and its children the specific query_id*/
void Delete_Query_from_Hamming_Nodes(struct HammingNode* node,QueryID query_id){
	for(struct HammingNode* child=node->firstChild;child!=NULL;child=child->next)
		Delete_Query_from_Hamming_Nodes(child,query_id);
	/*Base Case*/
	struct Info* info_node=node->start_info;
	if(info_node==NULL)
		return ;
	if(info_node->query_id==query_id){
		struct Info* delete_node=info_node;
		node->start_info=node->start_info->next;
		free(delete_node);
	}
	else{
		struct Info* info_node=node->start_info;
		struct Info* next_node=node->start_info->next;
		if(next_node==NULL)
			return ;
		while(1){
			if(next_node==NULL)
				break;
			if(next_node->query_id==query_id){
				info_node->next=next_node->next;
				free(next_node);
				break;
			}
			info_node=next_node;
			next_node=next_node->next;
		}
	}
}

/*For each query return the words of it to an array*/
char** words_ofquery(const char* query_str,int* num){
	char curr_words[MAX_QUERY_WORDS][MAX_WORD_LENGTH];
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



/*Checking on global Hash_Array for Exact result*/
struct Match_Type_List*   Exact_Result(char* word){
	struct Match_Type_List* beg_ptr=malloc(sizeof(struct Match_Type_List));
	beg_ptr->start=NULL;
	beg_ptr->cur=NULL;
	beg_ptr->counter=0;
	int coun=0;
	for(int i=0;i<bucket_sizeofHashTableExact;i++){
		struct Exact_Node* start=HashTableExact->array[i];
		if(start==NULL)
			continue;
		while(1){
			if(!strcmp(start->wd,word)){
				coun++;
				Entry* en=Put_data(start);
				if(beg_ptr->start==NULL){
					beg_ptr->start=en;
					beg_ptr->cur=en;
					beg_ptr->counter++;
				}
				else{
					beg_ptr->cur->next=en;
					beg_ptr->cur=en;
					beg_ptr->counter++;
				}
			}
			start=start->next;
			if(start==NULL)
				break;
		}
	}
	return beg_ptr;
}


/*From node create entry_node*/
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


/*Function returns a list with mathing entries of word for hamming*/
struct Match_Type_List* Edit_Result(char* word){
	struct Match_Type_List* Match_Node=malloc(sizeof(struct Match_Type_List));
	Match_Node->start=NULL;
	Match_Node->cur=NULL;
	Match_Node->counter=0;
	if(BKTreeIndexEdit == NULL){
		return Match_Node;
	}
	if(BKTreeIndexEdit->root == NULL){
		return Match_Node;
	}
	int d, bot, ceil;
	struct EditNode* curr;
	struct Edit_Stack_Node* candidate_list = NULL;
	/*push on stack the word of root*/
	push_stack_edit(&candidate_list, &(BKTreeIndexEdit->root));
	struct EditNode* children = NULL;
	struct Info* info;
	while(candidate_list != NULL){
		curr = pop_stack_edit(&candidate_list);
		info = curr->start_info;
		d = EditDistance(word, strlen(word), curr->wd, strlen(curr->wd));
		bool enter=false;
		Entry* s=NULL;
		while(info != NULL){
			/*if enter on while loop word is appearing on queryid else not and juct ignore it*/
			if(d <= info->match_dist){
				if(enter==false){
					/*In that case word is mathing for first time */
					Entry* new_node=malloc(sizeof(Entry));
					new_node->next=NULL;
					new_node->my_word=malloc((strlen(curr->wd)+1)*sizeof(char));
					strcpy(new_node->my_word,curr->wd);
					payload_node* p_node=malloc(sizeof(payload_node));
					p_node->query_id=info->query_id;
					p_node->next=NULL;
					enter=true;
					s=new_node;
					new_node->payload=p_node;
				}
				else{
					/*In that case we juct add the payload*/
					payload_node* p_node=malloc(sizeof(payload_node));
					p_node->query_id=info->query_id;
					p_node->next=NULL;
					payload_node* beginning1=s->payload;
					while(1){
						if(beginning1->next==NULL){
							beginning1->next=p_node;
							break;
						}
						beginning1=beginning1->next;
					}
				}
			}
			info = info->next;
		}
		if(enter==true){
			if(Match_Node->start==NULL){
				Match_Node->start=s;
				Match_Node->cur=s;
				Match_Node->counter=1;
			}
			else{
				Match_Node->cur->next=s;
				Match_Node->cur=s;
				Match_Node->counter++;
			}
		}
		if(d-MAX_MATCH_DIST<0)
			bot = 0;
		else
			bot=d-MAX_MATCH_DIST;
		ceil = d + MAX_MATCH_DIST;
		children = curr->firstChild;
		/*push on stack the childs of Node*/
		while(children != NULL){
			if(children->distance >= bot && children->distance <= ceil){
				push_stack_edit(&candidate_list, &children);
			}
			children = children->next;
		}
	}
	return Match_Node;
}



/*Function returns a list with mathing entries of word for hamming*/
struct Match_Type_List* Hamming_Result(char* word){
	struct Match_Type_List* Match_Node=malloc(sizeof(struct Match_Type_List));
	Match_Node->start=NULL;
	Match_Node->cur=NULL;
	Match_Node->counter=0;
	int position = strlen(word) - 4;
	/*Navigate to right Index*/
	if(HammingDistanceStructNode->word_RootPtrArray[position].HammingPtr == NULL){
		return Match_Node;
	}
	struct HammingNode* tree = 	HammingDistanceStructNode->word_RootPtrArray[position].HammingPtr->root;
	if(tree == NULL){
		return Match_Node;
	}
	int d, bot, ceil;
	struct HammingNode* curr;
	struct Hamming_Stack_Node* candidate_list = NULL;
	/*push on stack the word of root*/
	push_stack_hamming(&candidate_list, &tree);
	struct HammingNode* children = NULL;
	struct Info* info;
	while(candidate_list != NULL){
		curr = pop_stack_hamming(&candidate_list);
		info = curr->start_info;
		bool enter=false;
		Entry* s=NULL;
		d = HammingDistance(word, strlen(word), curr->wd, strlen(curr->wd));
		while(info != NULL){
			/*if enter on while loop word is appearing on queryid else not and juct ignore it*/
			if(d <= info->match_dist){
				if(enter==false){
					/*In that case word is mathing for first time */
					Entry* new_node=malloc(sizeof(Entry));
					new_node->next=NULL;
					new_node->my_word=malloc((strlen(curr->wd)+1)*sizeof(char));
					strcpy(new_node->my_word,curr->wd);
					payload_node* p_node=malloc(sizeof(payload_node));
					p_node->query_id=info->query_id;
					p_node->next=NULL;
					enter=true;
					s=new_node;
					new_node->payload=p_node;
				}
				else{
					/*In that case we juct add the payload*/
					payload_node* p_node=malloc(sizeof(payload_node));
					p_node->query_id=info->query_id;
					p_node->next=NULL;
					payload_node* beginning1=s->payload;
					while(1){
						if(beginning1->next==NULL){
							beginning1->next=p_node;
							break;
						}
						beginning1=beginning1->next;
					}
				}			
			}
			info = info->next;
		}
		if(enter==true){
			if(Match_Node->start==NULL){
				Match_Node->start=s;
				Match_Node->cur=s;
				Match_Node->counter=1;
			}
			else{
				Match_Node->cur->next=s;
				Match_Node->cur=s;
				Match_Node->counter++;
			}
		}
		if(d-MAX_MATCH_DIST<0)
			bot = 0;
		else
			bot=d-MAX_MATCH_DIST;
		ceil = d + MAX_MATCH_DIST;
		children = curr->firstChild;
		/*push on stack the childs of Node*/
		while(children != NULL){
			if(children->distance >= bot && children->distance <= ceil){
				push_stack_hamming(&candidate_list, &children);
			}
			children = children->next;
		}
	}
	return Match_Node;
}



void Delete_Query_from_Active_Queries(QueryID query_id){
	struct Query_Info* start=ActiveQueries;
	if(start->query_id==query_id){
		struct Query_Info* query_node=start->next;
		free(start);
		ActiveQueries=query_node;
		return;
	}
	struct Query_Info* next_start=ActiveQueries->next;
	while(1){
		if(next_start->query_id==query_id){
			start->next=next_start->next;
			free(next_start);
			break;
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
		ActiveQueries=node;
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


/*Delete each Node of struct Match_Type_List*/
void Delete_Result_List(struct Match_Type_List* en){
	Entry* start1=en->start;
	if(start1==NULL)
		return ;
	Entry* start1_next=start1->next;
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




QueryID* Put_On_Result_Hash_Array(struct Match_Type_List* en,int* result_counter){
	int sum=en->counter;
	float curr_size=sum/0.8;
	if(sum==0){
		*result_counter=0;
		return NULL;
	}
	int size=(int)curr_size;
	size=NextPrime(size);
	/*Define the number of buckets of Hash_Array*/
	struct Result_Hash_Node** hash_array=malloc(size*sizeof(struct Result_Hash_Node*));
	for(int i=0;i<size;i++)
		hash_array[i]=NULL;
	Entry* start1=en->start;
	/*Add the words that have matched*/
	while(1){
		if(start1==NULL)
			break;
		char* the_word=start1->my_word;
		payload_node* p1=start1->payload;
		while(1){
			if(p1==NULL)
				break;
			QueryID q=p1->query_id;
			int bucket_size=hash_interger(q)%size;
			Hash_Put_Result(q,the_word,&hash_array[bucket_size]);
			p1=p1->next;
		}
		start1=start1->next;
	}
	struct Info* result_list=NULL;
	int length_final_array=0;
	struct Query_Info* qnode=ActiveQueries;
	/*Check for each query if all words of it were matched*/
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
	/*Copy the mathing queries to return val of function*/
	QueryID* final=malloc(length_final_array*sizeof(QueryID));
	struct Info* start_result=result_list;
	int i=0;
	while(1){
		if(start_result==NULL)
			break;
		final[i++]=start_result->query_id;
		start_result=start_result->next;
	}
	/*Free Hash Array Node*/
	for(int i=0;i<size;i++){
		struct Result_Hash_Node* hash_node=hash_array[i];
		if(hash_node==NULL) continue;
		struct Result_Hash_Node* hash_node_next=hash_node->next;
		while(1){
			struct word_node* beg=hash_node->word_start;
			struct word_node* beg_next=beg->next;
			while(1){
				free(beg->word);
				free(beg);
				beg=beg_next;
				if(beg==NULL)
					break;
				beg_next=beg_next->next;
			}
			free(hash_node);
			hash_node=hash_node_next;
			if(hash_node==NULL)
				break;
			hash_node_next=hash_node_next->next;
		}
	}
	free(hash_array);
	*result_counter=length_final_array;
	return final;
}







/*Put on HashArray that contains the queries aand their words that have been matched new Node*/
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
	/*check if specific queryid found*/
	while(1){
		if(ptr2->query_id==q){
			found=1;
			break;
		}
		ptr2=ptr2->next;
		if(ptr2==NULL)
			break;
	}
	/*if found just check if word exists or not*/
	if(found==1){
		int found2=0;
		struct word_node* s1=ptr2->word_start;
		while(1){
			if(s1==NULL)
				break;
			if(!strcmp(s1->word,word)){
				found2=1;
				break;
			}
			s1=s1->next;
		}
		/*if found specific word have matched*/
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
	/* if not just create new_node with specific query_id*/
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













/*Put the result node  that contains a docid and an array of QueryID*/
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
	StackArray->counter++;
	if(StackArray->first==NULL)
		StackArray->first=node;
}


/*Delete the topp node of StackArray*/
void Delete_From_Stack(){
	struct result* node=StackArray->top;
	StackArray->top=StackArray->top->next;
	if(StackArray->top==NULL)
		StackArray->first=NULL;
	StackArray->counter--;
	free(node->query_id);
	free(node);
}


/*free all nodes of active queries struct*/
void Free_Active_Queries(){
	if(ActiveQueries == NULL){
		return;
	}
	struct Query_Info* start=ActiveQueries;
	struct Query_Info* next_start=start->next;
	while(1){
		free(start);
		start=next_start;
		if(start==NULL)
			break;
		next_start=next_start->next;
	}
	ActiveQueries=NULL;
}