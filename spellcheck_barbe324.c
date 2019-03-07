#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DICT_HASHTABLE_SIZE	100000
#define DOC_HASHTABLE_SIZE 	 10000
#define MAX_WORD_SIZE		   100
#define DEBUG_INPUT_IO		   0x1
#define DEBUG_DICTIONARY_IO	   0x2
#define arraySize(x)  (sizeof(x) / sizeof((x)[0]))

int debug;
const char *usage = "Usage: spellcheck [-d debug_level] dictionary input\n";
//linked List struct definition
typedef struct llNode
{
	char* word;
	struct llNode *nextPtr;
}llNode;
//global hashTable variable declares
llNode** dict_hashtable;
llNode ** doc_hashtable;
/* Is this a character that could be part of a word? */
int is_word_char(int c) {
    return isalpha(c) || c == '\'' || c == '@';
}

//https://stackoverflow.com/questions/7666509/hash-function-for-string
int hashCode (unsigned char *str, int size){
	//random large prime
	unsigned int hash = 1997;
	unsigned int c;
	while (c = *str++){
		hash = ((hash << 5) + hash) + c;
	}
	hash = hash % size;
	return (hash);
}
//attach node takes 2 established llNode pointers move through list and attach at first open position)
void attachNode(struct llNode *head, struct llNode *n){
	while (head && head->nextPtr){
		head = (head->nextPtr);	
	}
	head->nextPtr = n;
}
//print the contents of a linkedList takes llNode pointer
//NON INSTRUMENTAL DEBUGGING FUNCTION
void printLL (struct llNode * head){
	printf("llNode: ");
	while (head){
	printf("%s=>",head->word);
	head = head->nextPtr;	
	};
	printf("\n");
}

//print the contents of an array of linked lists orderly, ( TAKES ARRAY OF POINTERS TO llNode)
//utilizes above printLL function
//prints in range 1,000 because you probably never need to look at a full 10000+ word dictionary as it is
//limitations: thorough testing must done in earlier range of given order(ie alphabet, doc word count), ultimately minor
//NON INSTRUMENTAL DEBUGGING FUNCTION
void printHashTable(struct llNode** ht){
	printf("\n==========================\nPrinting hashTable\n");
	for (int i = 0; i < 10000; i++){
		if(ht[i]){
			printf("[%d] ", i);
			printLL(ht[i]);
		};
	};

}

//return true if you find inputWord [AS IT COMES] in the dictionary
//use hashCode func and navigate linked list checking DICT hashtable at that hashCode position.
//return 1 if found, 0 if not
//O(C) 
//	C = {mean collisions}
int word_in_dict (char* inputWord){
	int wordHashCode = hashCode(inputWord, DICT_HASHTABLE_SIZE);
	llNode* ll = dict_hashtable[wordHashCode];
	int found = 0;
	while(ll && !found){
		if (strcmp(inputWord, ll->word) == 0) found = 1;
		ll = ll->nextPtr;
	};
	return found;
}
//mutating substring function, takes originalString, ideally empty subString, a start position p, and a len of subString l
void subString(char originalString[], char subString[], int p, int l) {
   int c = 0;
 
   while (c < l && subString[c] != "\0") {
      subString[c] = originalString[p+c];
      c++;
   }
   subString[c] = '\0';
} 
//return "???" in the case that string has already been checked (i.e. IS IN doc_hashtable )
//return "?" in the case went through all checks, in doc ht or in dict ht, word not found
//note: reasoning behind this is that you will only want to suggest a word if you have not already seen the given word in the document [not ???]
// and not in the dictionary itself [not the word found]

//return back inputWord if wordFound
//used by maybe_process_word and suggest_word


//first see if word exists in misspelled words ht, if it has assign returnWord to ??? our already seen' flag
//add to doc ht
//if (word does not already exists) {add it to misspelled word ht};
//second check if word in dictionary, if so return lowerCase or ulaltered version (for printing suggestions)
//if unfound in doc and dict hashtables then return default value "?"
//when '?' value returned means to suggest a word (within maybe_process_word)
//when not a '?..' value returned means values must be a word, and it is return value (within suggest_word function)

//the flag values are named in a way to be able to make a match to the first element '?' for logical grouping implications in certain checks (ex. suggest_word & maybe_process_word)
char* check_word(char* inputWord){
	llNode* llHead;
	llNode* baseNode;
	char * inputWordLower = malloc (100*sizeof(char));
	strcpy(inputWordLower, inputWord);
	for(int i = 0; inputWordLower[i]; i++){
		if (inputWordLower[i] != '\'') inputWordLower[i] = tolower(inputWordLower[i]);
	};
	
	int word_hash_code = hashCode(inputWord, DOC_HASHTABLE_SIZE);
	baseNode = doc_hashtable[word_hash_code];
	llHead = doc_hashtable[word_hash_code];
	char* returnWord = malloc (100*sizeof(char));
	strcpy(returnWord, "?");
	while(baseNode){
		if 	(strcmp(inputWord,baseNode->word) == 0){
			//word is found to have already been processed
			//printf("WORD ALREADY BEEN CHECKED SOMEHOW STOP CHECKING AGAIN\n");
			return "???";
		};
		baseNode = baseNode->nextPtr;
	};
	llNode * newNodeP = malloc (sizeof(llNode));	
	newNodeP->word = strdup(inputWord);
	newNodeP->nextPtr = NULL;
	if (llHead){
		attachNode(llHead, newNodeP);
	} else {
		doc_hashtable[word_hash_code] = newNodeP;
	};
	//we didnt find it so now we have to check if its in the dictionary
	//check if in dict
	//first check for unaltered theWord if contained in dict
	//second check for lowercase version
	char * wordCopy = strdup(inputWord);
	char * wordCopyLower = strdup(inputWord);
	for(int i = 0; wordCopyLower[i]; i++){
		if (wordCopyLower[i] != '\'') wordCopyLower[i] = tolower(wordCopyLower[i]);
	};
	
	int unalteredInDict = word_in_dict(wordCopy);
	int lowerInDict = word_in_dict(wordCopyLower);
	if (unalteredInDict || lowerInDict ){
		//word in dictionary either as is or lowerCase
		if (unalteredInDict > lowerInDict){
			
			returnWord = strdup(wordCopy);
		}else{
			returnWord = strdup(wordCopyLower);
		};
	} 	
	return returnWord;
};
//cycle through permutations of the string given by adjusting to each 'one off char' string
//check for one diff character permutations, first because we assume user sent correct number of chars
//check for one fewer character permutations
//check for one greater character permutations
// O(53(N)) -> O(N)
//	 N = inputWord length
// cycle through word permutations is constant, due to constant suggest_char_array size
//the suggest_char_array is 53 elements long.
char* suggest_char_array[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "'"};
char * suggest_word(char * inputWord){
	int found = 0;
	int pos = 1;
	int leng = strlen(inputWord);
	char* subStr = malloc (100*sizeof(char)); 
	char* check_word_subStr;
	strcpy(subStr, "?");
	//CHECK FOR ONE DIFFERENT CHAR WORD
	//i represents pos of the char replaced here
	for(int i = 0; i < leng && inputWord[i] != "\0" && !found; i++){
		char* start = malloc (100*sizeof(char));
		char* end = malloc (100*sizeof(char));
		
		for (int j = 0; j < arraySize(suggest_char_array) && !found; j++){
			subString(inputWord, start, 0, i);
			subString(inputWord, end, i+1, leng-i);
			char* replacementLetter = suggest_char_array[j];
			subStr = strcat(start, replacementLetter);
			subStr = strcat(subStr, end);
				check_word_subStr = check_word(subStr);
				if (strcmp(check_word_subStr, "?") != 0 && strcmp(check_word_subStr, "???") != 0 ) found = 1;
		};
		
	};
	//CHECK FOR ONE FEWER CHAR WORD
	//i represents the pos of char taken out here
	for(int i = 0; i < leng && inputWord[i] != "\0" && !found; i++){
		char* start = malloc (100*sizeof(char));
		char* end = malloc (100*sizeof(char));
		subString(inputWord, start, 0, i);
		subString(inputWord, end, i+1, leng-i-1);
		strcat(start,end);
		check_word_subStr = check_word(start);
		if (strcmp(check_word_subStr, "?") != 0 && strcmp(check_word_subStr, "???") != 0 ) found = 1;
	};
	//CHECK FOR ONE GREATER CHAR WORD
	//i represents pos of char added
	for(int i = 0; i < leng && inputWord[i] != "\0" && !found; i++){
		char* start = malloc (100*sizeof(char));
		char* end = malloc (100*sizeof(char));
		for (int j = 0; j < arraySize(suggest_char_array) && !found; j++){
			subString(inputWord, start, 0, i);
			subString(inputWord, end, i, leng-i);
			char* insertLetter = suggest_char_array[j];
			subStr = strcat(start, insertLetter);
			subStr = strcat(subStr, end);
			
			//printf("%s\n", subStr);
				check_word_subStr = check_word(subStr);
				if (strcmp(check_word_subStr, "?") != 0 && strcmp(check_word_subStr, "???") != 0 ){
				found = 1;
			};
			
		};
		
	};
	if(!found){check_word_subStr = "?";};
	return check_word_subStr;
}
/* IO: Read in dictionary by word */
void read_dictionary(char *filename)
{
	struct llNode* baseNode;
    FILE *fh;
    int i = 0;
    int line_num = 1;
    int c;
    char word[MAX_WORD_SIZE];
	int hCode;
    fh = fopen(filename, "r");
    if (!fh) {
		fprintf(stderr, "Error: could not open wordlist %s: %s\n",
			filename, strerror(errno));
		exit(1);
    }
    while ((c = fgetc(fh)) != EOF) {
	if (c == '\n') {
	    line_num++;
	    if (i == 0)
		continue;
	    word[i] = '\0';
	    i = 0;
	    if (debug & DEBUG_DICTIONARY_IO)
		printf("%s\n", word);
	    char * newWord = strdup(word);
	    llNode *newNodeP = malloc ( sizeof(llNode) );
		newNodeP->word = newWord;
		newNodeP->nextPtr = NULL;
		hCode = hashCode(word, DICT_HASHTABLE_SIZE);
		//reuse baseNode for temporarily holding the contents of the hCode psotion in the hashTable
		baseNode = dict_hashtable[hCode];
		if (baseNode){
			attachNode(baseNode, newNodeP);
			
		}else{
			dict_hashtable[hCode] = newNodeP;
		};
	} else if (i == MAX_WORD_SIZE - 1) {
	    word[i] = 0;
	    fprintf(stderr, "Word too long at wordlist line %d: %s...\n",
		    line_num, word);
	    exit(2);
	} else {
	    if (!is_word_char(c)) {
			fprintf(stderr, "Bad character '%c' in word at line %d\n",
				c, line_num);
			exit(2);
			}
			word[i] = c;
			i++;
		}
    }
    fclose(fh);
    //printdHashTable(dict_hashtable);
}
/* "word" should be a character buffer, where *i_ref is the number of
   valid characters in the buffer (or equivalently, the index where
   the next character would be added. If the buffer is not empty, then
   terminate the buffer, pass it to check_word, and then empty the
   buffer. */
//A take a word and its pointer, see if pointer is valid
//B if A then check if word valid in dictionary
//C if NOT B then suggest a word
void maybe_process_word(char *word, int *i_ref) {
    if (*i_ref != 0) {
		word[*i_ref] = '\0';
		if (debug & DEBUG_INPUT_IO)
	    printf("DBG: i=%-2d word=\"%s\"\n", *i_ref, word);
		/* TODO: process word here, e.g. check_word(word) */
		char* suggestion;
		//if word NOT in dictionary ht then suggest a word
		//if word NOT found in dictionary or document words then suggest a word
		char* check_word_Word = check_word(word);
		
		
		if(strcmp(check_word_Word, "?") == 0){
				//WHERE LINES PRINT
				suggestion = suggest_word(word);
				printf("%s : %s\n\n",word, suggestion);
		};
		*i_ref = 0;
    }
}
/* IO: Read in input by word */
void process_document(char *filename)
{
    FILE *fh;
    int i = 0;
    int c;
    int line_num = 1;
    int check_hyphen_mode = 0;
    int seen_hyphen_newline = 0;
    char word[MAX_WORD_SIZE];
    fh = fopen(filename, "r");
    if (!fh) {
		fprintf(stderr, "Error: could not open input %s: %s\n", filename,
			strerror(errno));
		exit(1);
    }
    printf("Printing missppelled word suggestions\n=====================================\n");
    while ((c = fgetc(fh)) != EOF) {
		if (c == '\n')
			line_num++;
		if (check_hyphen_mode) {
			if (isspace(c)) {
			if (c == '\n')
				seen_hyphen_newline = 1;
			continue;
			} else {
			check_hyphen_mode = 0;
			if (!seen_hyphen_newline) {
				/* If no newline, treat as two words */
				//printf("%s", word);
				maybe_process_word(word, &i);   
			} else {
				/* If there was a newline, ignore all the
				   whitespace */
			}
			seen_hyphen_newline = 0;
			}
		}
		if (c == '-') {
			/* Can't decide yet whether this is the end of a word, it
			   depends what comes next. */
			check_hyphen_mode = 1;
		} else if (!is_word_char(c)) {
			maybe_process_word(word, &i);
			//printf("%s\n",word);  
		} else if (i == MAX_WORD_SIZE - 1) {
			word[i] = 0;
			fprintf(stderr, "Word too long in document at line %d: %s...\n",
				line_num, word);
			exit(2);
		} else {
			assert(is_word_char(c));
			//change @ to a at file input
			if (c == '@'){
				word[i] = 'a';
			}else{
				word[i] = c;
			}
			i++;
		}
    }
    maybe_process_word(word, &i);
	//printf("%s\n",word);
    fclose(fh);
}
int main(int argc, char **argv){
    char *dict_fname;
    char *doc_fname;
    if (argc == 3) {
	/* No options */
	dict_fname = argv[1];
	doc_fname = argv[2];
    } else if (argc == 5) {
	/* Maybe one option and its argument, which must come first. */
	if (argv[1][0] == '-') {
	    if (argv[1][1] == 'd' && argv[1][2] == '\0') {
		char *endptr;
		debug = strtol(argv[2], &endptr, 0);
		if (debug < 0 || endptr == argv[2] || *endptr != '\0') {
		    fprintf(stderr, "%s", usage);
		    fprintf(stderr, "Argument to -d should be a "
			    "non-negative integer\n");
		    return 1;
		}
		dict_fname = argv[3];
		doc_fname = argv[4];
	    } else {
		fprintf(stderr, "%s", usage);
		fprintf(stderr, "Unrecognized option '%s'\n", argv[1]);
		return 1;
	    }
	} else {
	    fprintf(stderr, "%s", usage);
	    fprintf(stderr, "Too many non-option arguments\n");
	    return 1;
	}
    } else {
		fprintf(stderr, "%s", usage);
		fprintf(stderr, "Wrong number of arguments\n");
		return 1;
    }
	dict_hashtable = malloc (DICT_HASHTABLE_SIZE * sizeof(llNode));
	doc_hashtable = malloc (DOC_HASHTABLE_SIZE*(sizeof(llNode)));
    read_dictionary(dict_fname);
    process_document(doc_fname);
	free(dict_hashtable);
	free(doc_hashtable);

}
