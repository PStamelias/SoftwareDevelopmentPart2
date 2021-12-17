/*
 * core.h version 1.0
 * Copyright (c) 2013 KAUST - InfoCloud Group (All Rights Reserved)
 * Authors: Amin Allam, Fuad Jamour
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
 *
 * Current version: 1.0 (initial release -- Feb 1, 2013)
 */

#ifndef __SIGMOD_CORE_H_
#define __SIGMOD_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
///////////////////////////////////////////////////////////////////////////////////////////////
//*********************************************************************************************

/// Maximum document length in characters.
#define MAX_DOC_LENGTH (1<<22)

/// Maximum word length in characters.
#define MAX_WORD_LENGTH 31

/// Minimum word length in characters.
#define MIN_WORD_LENGTH 4

/// Maximum number of words in a query.
#define MAX_QUERY_WORDS 5

/// Maximum query length in characters.
#define MAX_QUERY_LENGTH ((MAX_WORD_LENGTH+1)*MAX_QUERY_WORDS)

///Maximum Matching Distance
#define MAX_MATCH_DIST 3


/// Query ID type.
typedef unsigned int QueryID;

/// Document ID type.
typedef unsigned int DocID;


/// Matching types:
enum MatchType {
    /**
    * Two words match if they are exactly the same.
    */
    MT_EXACT_MATCH,
    /**
    * Two words match if they have the same number of characters, and the
    * number of mismatching characters in the same position is not more than
    * a specific threshold.
    */
    MT_HAMMING_DIST,
    /**
    * Two words match if one of them can can be transformed into the other word
    * by inserting, deleting, and/or replacing a number of characters. The number
    * of such operations must not exceed a specific threshold.
    */
    MT_EDIT_DIST
};

typedef char word;
typedef struct payload_node{
    QueryID query_id;
    struct payload_node* next;
}payload_node;
typedef struct Index{
  struct EditNode* root;
}Index;
struct HammingIndex{
    struct HammingNode* root;
};
struct Name_Info{
  struct Name_Info* next;
  struct Name* ptr;
};
typedef struct entry {
  word* my_word;
  payload_node* payload;
  struct entry* next;
}Entry;
typedef struct entry_list{
  Entry* first_node;
  Entry* current_node;
  unsigned int counter;
}entry_list;
struct NodeIndex{
  word* wd;
  int distance;
  payload_node* query_list;
  struct NodeIndex* next;
  struct NodeIndex* firstChild;
};
struct Match_Type_List{
    Entry* start;
    Entry* cur;
    unsigned int counter;
};
struct result{
    DocID doc_id;
    unsigned int result_counter;
    QueryID* query_id;
    struct result* next;
};
struct word_node{
    char* word;
    struct word_node* next;
};
struct Result_Hash_Node{
    QueryID query_id;
    unsigned int distinct_words;
    struct word_node* word_start;
    struct Result_Hash_Node* next;
};
struct Stack_result{
    struct result* first;
    struct result* top; 
    int counter;
};
struct Deduplicate_Node{
    word* the_word;
    struct Deduplicate_Node* next;
};
struct Deduplicate_Hash_Array{
    struct Deduplicate_Node** array;
    unsigned int entries_counter;
};
struct word_RootPtr{
    int word_length;
    struct HammingIndex* HammingPtr;   
};
struct HammingDistanceStruct{
    struct word_RootPtr* word_RootPtrArray;
};
struct Exact_Node{
    word* wd;
    payload_node* beg;
    struct Exact_Node* next;
    struct Exact_Node* prev;
};
struct Info{
    QueryID query_id;
    unsigned int match_dist;
    struct Info* next;
};
struct Query_Info{
    QueryID query_id;
    unsigned int counter_of_distinct_words;
    struct Query_Info* next;
};
struct EditNode{
  word* wd;
  int distance;
  struct Info* start_info;
  struct EditNode* next;
  struct EditNode* firstChild;
};
struct HammingNode{
  word* wd;
  int distance;
  struct Info* start_info;
  struct HammingNode* next;
  struct HammingNode* firstChild;
};
struct Exact_Root{
    struct Exact_Node** array;
    unsigned int entries_counter;
};

struct Edit_Stack_Node{
    struct EditNode* node;
    struct Edit_Stack_Node* next;
};

struct Hamming_Stack_Node{
    struct HammingNode* node;
    struct Hamming_Stack_Node* next;
};

extern struct HammingDistanceStruct* HammingDistanceStructNode;
extern Index*  BKTreeIndexEdit;
extern struct Exact_Root* HashTableExact;
extern int bucket_sizeofHashTableExact;

/// Error codes:			
enum ErrorCode {
    /**
    * Must be returned by each core function unless specified otherwise.
    */
    EC_SUCCESS,
    /**
    * Must be returned only if there is no available result to be returned
    * by GetNextAvailRes(). That is, all results have already been returned
    * via previous calls to GetNextAvailRes().
    */
    EC_NO_AVAIL_RES,
    /**
    * Used only for debugging purposes, and must not be returned in the
    * final submission.
    */
    EC_FAIL
};

typedef enum MatchType MatchType;
typedef enum ErrorCode ErrorCode;
///////////////////////////////////////////////////////////////////////////////////////////////
//*********************************************************************************************

/**
* Called only once at the beginning of the whole test.
* Performs any required initializations. 
*/
ErrorCode InitializeIndex();

/**
* Called only once at the end of the whole test.
* Can be used for releasing all memory used to index active queries.
* The time spent in this function will not affect the score of the
* submission.
*/
ErrorCode DestroyIndex();

/**
* Add a query (associated with matching type) to the active query set.
* 
* @param[in] query_id
*   The integral ID of the query. This function will not be called twice
*   with the same query ID.
*
* @param[in] query_str
*   A null-terminated string representing the query. It consists of
*   a space separated set of words. The length of any word will be at
*	least MIN_WORD_LENGTH characters and at most MAX_WORD_LENGTH
*	characters. The number of words in a query will not exceed
*	MAX_QUERY_WORDS words.
*   "query_str" contains at least one non-space character.
*   "query_str" contains only lower case letters from 'a' to 'z'
*   and space characters.
* 
* @param[in] match_type
*   The type of mechanism used to consider a query as a
*   match to any document, as specified in MatchType enumeration.
*
* @param[in] match_dist
*   The hamming or edit distance (according to "match_type")
*   threshold used as explained in MatchType enumeration.
*   This parameter must be equal 0 for exact matching. The possible
*   values of this parameter are 0,1,2,3.
*   A query matches a document if and only if: for each word in
*	the query, there exist a word in the document that matches it
*	under the "match_type" and "match_dist" constraints. Note that
*	the "match_dist" constraint is applied independently for each
*	word in the query.
*
* @return ErrorCode
*   - \ref EC_SUCCESS
*          if the query was registered successfully
*/
ErrorCode StartQuery(QueryID        query_id,   
                     const char*    query_str,
                     MatchType      match_type,
                     unsigned int   match_dist);

/**
* Remove a query from the active query set.
*
* @param[in] query_id
*   The integral ID of the query. This function will not be called twice
*   with the same query ID.
*
* @return ErrorCode
*   - \ref EC_SUCCESS
*         if the query was unregistered successfully
*/
ErrorCode EndQuery(QueryID query_id);

/**
* Push a document to the server.
*
* @param[in] doc_id
*   The integral ID of the document. This function will not be called twice
*   with the same document ID.
*
* @param[in] doc_str
*   A null-terminated string representing the document. It consists of
*   a space separated set of words. The length of any word
*   will be at least MIN_WORD_LENGTH characters and at most
*	MAX_WORD_LENGTH characters. The length of any document will not
*   exceed MAX_DOC_LENGTH characters.
*   "doc_str" contains at least one non-space character.
*   "doc_str" contains only lower case letters from 'a' to 'z'
*   and space characters.
*
*   @return ErrorCode
*   - \ref EC_SUCCESS
*          if the document was added successfully
*/
ErrorCode MatchDocument(DocID         doc_id,
                        const char*   doc_str);

/**
* Return the next available active queries subset that matches any previously
* submitted document, sorted by query IDs. The returned result must depend
* on the state of the active queries at the time of calling MatchDocument().
*
* @param[out] *p_doc_id
*   A document ID that has not been returned before. You can choose to return
*   the results of any document that has not been returned before.
*
* @param[out] *p_num_res
*   The number of active queries that matched the document *p_doc_id.
*
* @param[out] *p_query_ids
*   An array of the IDs of the *p_num_res matching queries,
*   ordered by the ID values. This array must be allocated by this
*   core library using malloc(). This array must not be freed by the
*   core library, since it will be freed by the testing benchmark.
*   If *p_num_res=0, this array must not be allocated, as it will not
*   be freed by the testing benchmark in that case.
*   Allocating this array using "new" is not acceptable.
*   In case of *p_num_res is not zero, The size of this array must be 
*   equal to "(*p_num_res)*sizeof(QueryID)" bytes.
*
* @return ErrorCode
*   - \ref EC_NO_AVAIL_RES
*          if all documents have already been returned by previous calls to this function
*   - \ref EC_SUCCESS
*          results returned successfully
*/
ErrorCode GetNextAvailRes(DocID*         p_doc_id,
                          unsigned int*  p_num_res,
                          QueryID**      p_query_ids);

///////////////////////////////////////////////////////////////////////////////////////////////
//*********************************************************************************************

int min(int, int, int);
void push_stack_edit(struct Edit_Stack_Node**, struct EditNode**);
struct EditNode* pop_stack_edit(struct Edit_Stack_Node**);
void push_stack_hamming(struct Hamming_Stack_Node**, struct HammingNode**);
struct HammingNode* pop_stack_hamming(struct Hamming_Stack_Node**);
Entry* Put_data(struct Exact_Node* node);
ErrorCode build_entry_index_Edit(char* word,QueryID query_id,unsigned int match_dist);
unsigned int hash_interger(unsigned int x);
struct Match_Type_List*  Exact_Result(char* word);
struct Match_Type_List* Edit_Result(char* word);
ErrorCode create_entry_list(entry_list** el);
struct Match_Type_List* Hamming_Result(char* word);
void Delete_Query_from_Active_Queries(QueryID query_id);
int NextPrime(int N);
bool isPrime(int N);
int hash_number_char(char* symbol,int buckets);
char** Deduplicate_Method(const char* query_str,int* size);
ErrorCode destroy_entry_index(Index* ix);
void destroy_index_nodes(struct EditNode* node);
struct Deduplicate_Hash_Array* Initialize_Hash_Array(int BucketsHashTable);
void free_Deduplication_Hash_Array(struct Deduplicate_Hash_Array* hash,int BucketsHashTable);
void insert_hash_array(struct Deduplicate_Hash_Array** hash,int BucketsHashTable,char* word);
bool search_hash_array(struct Deduplicate_Hash_Array* hash,int BucketsHashTable,char* word);
void Exact_Put(char** words,int num,QueryID query_id);
bool check_if_word_exists(char* word,int bucket_num,QueryID query_id);
void insert_HashTableExact(const char* word,int bucket_num,QueryID query_id);
void insert_HashTableExact_V2(struct Exact_Root* head,char* word,int bucket_num,struct payload_node* payload_ptr);
bool empty_of_payload_nodes(struct Exact_Node* node);
void Check_Exact_Hash_Array(QueryID query_id);
ErrorCode build_entry_index_Hamming(char* word,QueryID query_id,unsigned int match_dist);
void delete_specific_payload(struct Exact_Node* node,QueryID query_id);
void Hamming_Put(char** words_ofquery,int words_num,QueryID query_id,unsigned int match_dist);
void Edit_Put(char** words_ofquery,int words_num,QueryID query_id,unsigned int match_dist);
void Delete_Query_from_Edit_Nodes(struct EditNode* node,QueryID query_id);
void Check_Edit_BKTree(QueryID query_id);
void Check_Hamming_BKTrees(QueryID query_id);
void Delete_Query_from_Hamming_Nodes(struct HammingNode* node,QueryID query_id);
char** words_ofquery(const char* query_str,int* num);
QueryID* Put_On_Result_Hash_Array(struct Match_Type_List* en1,int* result_counter);
void Put_query_on_Active_Queries(QueryID query_id,int words_num);
Entry* Merge_List(Entry* a,Entry* b,Entry* c);
void Delete_Result_List(struct Match_Type_List* en);
void Put_On_Stack_Result(DocID docID,int size,QueryID* query_array);
void Hash_Put_Result(QueryID q,char* word,struct Result_Hash_Node** rr1);
void Delete_From_Stack();

#ifdef __cplusplus
}
#endif

#endif // __SIGMOD_CORE_H_
