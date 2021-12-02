#include "../include/core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define PRIME 401
int EditDistance(char* a, int na, char* b, int nb)
{
	return 0;
}

unsigned int HammingDistance(char* a, int na, char* b, int nb)
{
	return 0;
}

struct HammingDistanceStruct* HammingDistanceStructNode;
Index*  BKTreeIndexEdit;
Entry** HashTableExact;
int bucket_sizeofHashTableExact;


ErrorCode InitializeIndex(){
	BKTreeIndexEdit=NULL;
	HashTableExact=NULL;
	bucket_sizeofHashTableExact=10;
	HashTableExact=malloc(bucket_sizeofHashTableExact*sizeof(Entry*));
	for(int i=0;i<bucket_sizeofHashTableExact;i++)
		HashTableExact[i]=NULL;
	int HammingIndexSize=(MAX_WORD_LENGTH-MIN_WORD_LENGTH)+1;
	HammingDistanceStructNode=NULL;
	HammingDistanceStructNode=malloc(sizeof(struct HammingDistanceStruct));
	HammingDistanceStructNode->word_RootPtrArray=malloc(HammingIndexSize*sizeof(struct word_RootPtr));
	for(int i=0;i<HammingIndexSize;i++){
		HammingDistanceStructNode->word_RootPtrArray[i].IndexPtr=NULL;
		HammingDistanceStructNode->word_RootPtrArray[i].word_length=4+i;
	}
	return EC_SUCCESS;
}

ErrorCode DestroyIndex(){
	return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	printf("-----------------------------\n");
	int words_num=0;
	char** words_ofquery=Deduplicate_Method(query_str,&words_num);
	printf("%d\n",words_num);
	for(int i=0;i<words_num;i++)
		printf("%s\n",words_ofquery[i]);
	if(match_type==0){
		printf("EXACT MATCH\n");
	}
	else if(match_type==1){
		printf("HAMMING DISTANCE\n");
	}	
	else if(match_type==2){
		printf("EDIT DISTANCE\n");
	}
	printf("id=%d\n",query_id);
	printf("query_str=%s\n",query_str);
	printf("match_type=%d\n",match_type);
	printf("match_dist=%u\n",match_dist);
	for(int i=0;i<words_num;i++)
		free(words_ofquery[i]);
	free(words_ofquery);
	return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID query_id)
{
	return EC_SUCCESS;
}

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	return EC_SUCCESS;
}


ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{
	return EC_SUCCESS;
}


ErrorCode build_entry_index(const entry_list* el,MatchType type,Index** ix){
	int val=0;
	if(type==1)//MT_HAMMING_DIST 
		val=1;
	if(type==2)//MT_EDIT_DIST
		val=2;
	if(el==NULL)
		return EC_FAIL;
	if(*ix!=NULL)
		return EC_FAIL;
	*ix=malloc(sizeof(Index));
	(*ix)->root=NULL;
	if(val==1)
		(*ix)->type=0;
	if(val==2)
		(*ix)->type=1;
	for(struct entry* node=el->first_node;node!=NULL;node=node->next){
		word* curr_word=node->my_word;
		if((*ix)->root==NULL){
			struct NodeIndex* e=malloc(sizeof(struct NodeIndex));
			e->next=NULL;
			e->firstChild=NULL;
			e->distance=0;
			e->wd=NULL;
			e->wd=malloc((strlen(curr_word)+1)*sizeof(char));
			strcpy(e->wd,curr_word);
			(*ix)->root=e;
		}
		else{
			struct NodeIndex* start=(*ix)->root;
			while(1){
				int distance=0;
				if(val==1)
					distance=HammingDistance(start->wd,strlen(start->wd),curr_word,strlen(curr_word));
				if(val==2)
					distance=EditDistance(start->wd,strlen(start->wd),curr_word,strlen(curr_word));
				int found=0;
				struct NodeIndex* target=NULL;
				for(struct NodeIndex* c=start->firstChild;c!=NULL;c=c->next){
					if(distance==c->distance){
						found=1;
						target=c;
						break;
					}
				}
				if(found==1){
					start=target;
					continue;
				}
				struct NodeIndex* new_some=malloc(sizeof(struct NodeIndex));
				new_some->next=NULL;
				new_some->firstChild=NULL;
				new_some->distance=distance;
				new_some->wd=NULL;
				new_some->wd=malloc((strlen(curr_word)+1)*sizeof(char));
				strcpy(new_some->wd,curr_word);
				struct NodeIndex* c=start->firstChild;
				if(c==NULL){
					start->firstChild=new_some;
				}
				else{
					while(1){
						if(c->next==NULL){
							c->next=new_some;
							break;
						}
						c=c->next;
					}	
				}
				break;
			}
		}
	}
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

int hash_number_char(char* symbol,int buckets){
	char current;
	int h=0;
	int a=10;
	int i;
	int result;
	int size=strlen(symbol)+1;
	for(i=0;i<size;i++){
		current=symbol[i];
		h+=(h*a+current)%PRIME;
	}
	return h%buckets;
}
char** Deduplicate_Method(const char* query_str,int* size){
	int counter=0;
	int BucketsHashTable=0;
	char** word_array=NULL;
	struct Deduplicate_Node** Deduplication_Array=NULL;
	char word[MAX_WORD_LENGTH];
	int len=0;
	int len1=0;
	/*check how many words query contains*/
	for(int i=0;i<strlen(query_str);i++){
		if(query_str[i]==' ')
			counter+=1;
	}
	/*********************************/
	counter+=1;
	word_array=malloc(counter*sizeof(char* ));
	/*set bucket size*/
	BucketsHashTable=1.3*counter;
	BucketsHashTable=NextPrime(BucketsHashTable);
	/******************************/
	/*Hash TBle Initialization*/
	Deduplication_Array=malloc(BucketsHashTable*sizeof(struct Deduplicate_Node*));
	for(int i=0;i<BucketsHashTable;i++)
		Deduplication_Array[i]=NULL;
	/**************************/
	/*Put the unique words of query to hash table*/
	for(int i=0;i<strlen(query_str);i++){
		if(i==strlen(query_str)-1){
			word[len++]=query_str[i];
			word[len]='\0';
			len=0;
			int bucket=hash_number_char(word,BucketsHashTable);
			struct Deduplicate_Node* start=Deduplication_Array[bucket];
			if(start==NULL){
				struct Deduplicate_Node* new_node=malloc(sizeof(struct Deduplicate_Node));
				new_node->the_word=malloc((strlen(word)+1)*sizeof(char));
				strcpy(new_node->the_word,word);
				new_node->next=NULL;
				Deduplication_Array[bucket]=new_node;
			}
			else{
				while(1){
					if(!strcmp(start->the_word,word))
						break;
					if(start->next==NULL){
						struct Deduplicate_Node* new_node=malloc(sizeof(struct Deduplicate_Node));
						new_node->the_word=malloc((strlen(word)+1)*sizeof(char));
						strcpy(new_node->the_word,word);
						new_node->next=NULL;
						start->next=new_node;
						break;
					}
					start=start->next;
				}
			}
			memset(word,0,MAX_WORD_LENGTH);
			continue;
		}
		if(query_str[i]==' '){
			word[len]='\0';
			len=0;
			int bucket=hash_number_char(word,BucketsHashTable);
			struct Deduplicate_Node* start=Deduplication_Array[bucket];
			if(start==NULL){
				struct Deduplicate_Node* new_node=malloc(sizeof(struct Deduplicate_Node));
				new_node->the_word=malloc((strlen(word)+1)*sizeof(char));
				strcpy(new_node->the_word,word);
				new_node->next=NULL;
				Deduplication_Array[bucket]=new_node;
			}
			else{
				while(1){
					if(!strcmp(start->the_word,word))
						break;
					if(start->next==NULL){
						struct Deduplicate_Node* new_node=malloc(sizeof(struct Deduplicate_Node));
						new_node->the_word=malloc((strlen(word)+1)*sizeof(char));
						strcpy(new_node->the_word,word);
						new_node->next=NULL;
						start->next=new_node;
						break;
					}
					start=start->next;
				}
			}
			memset(word,0,MAX_WORD_LENGTH);
			continue;
		}
		word[len++]=query_str[i];
	}
	/*********************************************/
	/*Put the word of Hash Table to char array*/
	for(int i=0;i<BucketsHashTable;i++){
		if(Deduplication_Array[i]==NULL) continue;
		struct Deduplicate_Node* start=Deduplication_Array[i];
		while(1){
			if(start==NULL) break;
			word_array[len1]=malloc((strlen(start->the_word)+1)*sizeof(char));
			strcpy(word_array[len1++],start->the_word);
			start=start->next;
		}
	}
	/************************************************/
	/**Free memomy of Hash Table**/
	for(int i=0;i<BucketsHashTable;i++){
		if(Deduplication_Array[i]==NULL) continue;
		struct Deduplicate_Node* start=Deduplication_Array[i];
		struct Deduplicate_Node* start_next=start->next;
		while(1){
			free(start->the_word);
			free(start);
			start=start_next;
			if(start==NULL)
				break;
			start_next=start_next->next;
		}
	}
	free(Deduplication_Array);
	/***************************/
	*size=counter;
	return word_array;
} 