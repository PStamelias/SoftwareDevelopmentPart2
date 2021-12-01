#include "../include/core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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
				if(val==1){
					distance=HammingDistance(start->wd,strlen(start->wd),curr_word,strlen(curr_word));
				}
				if(val==2){
					distance=EditDistance(start->wd,strlen(start->wd),curr_word,strlen(curr_word));
				}
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