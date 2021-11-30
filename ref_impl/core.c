/*
 * core.cpp version 1.0
 * Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
 * Author: Amin Allam
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "../include/core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// Keeps all information related to an active query

Index* EditIndex;

///////////////////////////////////////////////////////////////////////////////////////////////

// Computes edit distance between a null-terminated string "a" with length "na"
//  and a null-terminated string "b" with length "nb" 
int EditDistance(char* a, int na, char* b, int nb)
{
	int oo=0x7FFFFFFF;

	static int T[2][MAX_WORD_LENGTH+1];

	int ia, ib;

	int cur=0;
	ia=0;

	for(ib=0;ib<=nb;ib++)
		T[cur][ib]=ib;

	cur=1-cur;

	for(ia=1;ia<=na;ia++)
	{
		for(ib=0;ib<=nb;ib++)
			T[cur][ib]=oo;

		int ib_st=0;
		int ib_en=nb;

		if(ib_st==0)
		{
			ib=0;
			T[cur][ib]=ia;
			ib_st++;
		}

		for(ib=ib_st;ib<=ib_en;ib++)
		{
			int ret=oo;

			int d1=T[1-cur][ib]+1;
			int d2=T[cur][ib-1]+1;
			int d3=T[1-cur][ib-1]; if(a[ia-1]!=b[ib-1]) d3++;

			if(d1<ret) ret=d1;
			if(d2<ret) ret=d2;
			if(d3<ret) ret=d3;

			T[cur][ib]=ret;
		}

		cur=1-cur;
	}

	int ret=T[1-cur][nb];

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// Computes Hamming distance between a null-terminated string "a" with length "na"
//  and a null-terminated string "b" with length "nb" 
unsigned int HammingDistance(char* a, int na, char* b, int nb)
{
	int j, oo=0x7FFFFFFF;
	if(na!=nb) return oo;
	
	unsigned int num_mismatches=0;
	for(j=0;j<na;j++) if(a[j]!=b[j]) num_mismatches++;
	
	return num_mismatches;
}

///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode InitializeIndex(){
	EditIndex=NULL;
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode DestroyIndex(){
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode StartQuery(QueryID query_id, const char* query_str, MatchType match_type, unsigned int match_dist)
{
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode EndQuery(QueryID query_id)
{
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode MatchDocument(DocID doc_id, const char* doc_str)
{
	
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode GetNextAvailRes(DocID* p_doc_id, unsigned int* p_num_res, QueryID** p_query_ids)
{
	return EC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////
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
					//distance=HammingDistance(start->wd,curr_word);
				}
				if(val==2){
					//distance=EditDistance(start->wd,curr_word,0);
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
ErrorCode destroy_entry_index(Index* ix){
	if(ix==NULL)
		return EC_FAIL;
	destroy_index_nodes(ix->root);
	free(ix);
	ix=NULL;
	if(ix!=NULL)
		return EC_FAIL;	
	return EC_SUCCESS;
}
void destroy_index_nodes(struct NodeIndex* node){
	for(struct NodeIndex* s=node->firstChild;s!=NULL;s=s->next){
		destroy_index_nodes(s);
	}
	free(node->wd);
	free(node);
}