/* ********************************
 * "Rhyming" dictionary with interface for user to search the dictionary
 * for a specific suffix, add a word, delete a word, or display
 * n number of words containing the given suffix, with each successive
 * use of the display function only printing out those words that have not
 * been previously listed.
 ***************************************
 */

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cctype>

using namespace std;
#define MAX_WORD 40

bool endOfWords = false;
bool deleteCheck = false;

//node struct for trie
struct node {
	char letter;
	bool validStart;        //valid beginning of a word in the trie
	node *leftMostChild;    //list of left children
	node *rightSibling;     //list of right children
	node *parent;           //parent of leftMostChild and rightSibling
};


// Reverses the string Note: changes the original string
char * strReverse(char *str)
{
	char *p1, *p2;

	if (! str || ! *str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}


// Convert word to lower case
void convertToLowerCase( char theWord[], const int size)
{
	// convert dictionary word to lower case
	for (int i=0; i<size; i++) {
		theWord[i] = tolower(theWord[i]);
	}
}


//print a concise list of commands.
void printMenuBrief(){
	cout << "Options:" << endl;
	cout << "f str   Find str" << endl;
	cout << "p n     Print next n" << endl;
	cout << "a str   Add str" << endl;
	cout << "d str   Delete str" << endl;
	cout << "? Help" << endl;
	cout << "x Exit" << endl;

	return;
}

//print out the menu with full descriptions on input '?'.
void printMenuFull(){
	cout << "Menu Options: " << endl;
	cout << "<f> <suffix> : Find the entered suffix within the dictionary" << endl;
	cout << "<p> <n> : Print the next n words in the dictionary that contain the suffix entered with the <f> command" << endl;
	cout << "<a> <word> : Add the given word to the dictionary" << endl;
	cout << "<d> <word> : Delete the given word from the dictionary" << endl;
	cout << "<?> : Print this menu" << endl;
	cout << "<x> : Exit the program" << endl;

	return;
}


//deallocate all nodes in trie
void freeMem(node *&nRoot){
	if (nRoot == NULL)
		return;

	freeMem(nRoot->leftMostChild);
	freeMem(nRoot->rightSibling);
	if (nRoot != NULL)
		delete nRoot;
	return;
}


//store a word into a trie using struct node. nRoot is the beginning of the trie; parentNode is
//the parent of a given level's left child and right siblings; char word[] is the word to be stored;
//length is the word length; counter is to check whether entire word has been stored (sent as hardcoded '0')
void enterIntoTrie(node *&nRoot, node *parentNode, char word[MAX_WORD], int length, int counter){
	//end of word reached
	if (counter == length)
		return;

	//position for new, non-repeated (on a given level) letter found; create new node
	if (nRoot == NULL ){
		nRoot = new node;
		nRoot->leftMostChild = NULL;
		nRoot->rightSibling = NULL;
		nRoot->parent = parentNode;

		//char word[] is 0 based, length of word is 1 based, offset to find actual node
		//where end of array is to mark a valid word
		if (counter == length-1)
			nRoot->validStart = true;
		else
			nRoot->validStart = false;

		nRoot->letter = word[counter];
		enterIntoTrie(nRoot->leftMostChild, nRoot, word, length, ++counter);    //send down a level since previous letter stored at this level
	}

	//search current level for letter; if not found, run to end of list, make new node
	else if (nRoot->letter != word[counter])
		enterIntoTrie(nRoot->rightSibling, parentNode, word, length, counter);

	//if letter is already in node, increment counter and send down to next level with new letter
	else{
		if (counter == length-1)
			nRoot->validStart = true;
		enterIntoTrie(nRoot->leftMostChild, nRoot, word, length, ++counter);
	}
	return;
}


/*read in words from dictionary, reverse them, and call enterIntoTrie to insert them into trie. nRoot is start of trie and parentNode is
parent of left child and right siblings of lower level. Returns void
 */
void dictionaryRead(node *&nRoot, node *parentNode){

	ifstream inStream;
	char fileName[] = "dictionary.txt";
	inStream.open( fileName);
	assert(!inStream.fail());

	bool alpha = true;
	char word[MAX_WORD];
	//loop through dictionary file
	while(inStream >> word){
		//check if word is alphabetic
		int i = 0;
		while(word[i]){
			if (!isalpha(word[i]))
				alpha = false;
			i++;
		}
		//if word is alphabetic, make it lower case, reverse it, send it to recursive function
		//to insert into trie
		if (alpha){
			convertToLowerCase(word, strlen(word));
			strReverse(word);
			i = 0;
			enterIntoTrie(nRoot, parentNode, word, strlen(word), 0);
		}
		alpha = true;   //reset alpha for next loop
	}
	return;
}


/*print the words as valid word nodes are found using parent pointers of valid node; currentNode is node found
that has the beginning of a valid word. Returns void*/
void printWord(node *currentNode){
	node *pTemp = currentNode;

	while (pTemp->parent != NULL){
		cout << pTemp->letter;
		pTemp = pTemp->parent;
	}
	cout << endl;
	return;
}


/*Find and print n words as chosen by user. Receives nRoot (root of trie),
wordCounter (number of words chosen) and numCounter (sum of number of words chosen
for this suffix so far) */
void findWords(node *nRoot, int *wordCounter, int *numCounter){
	if (*wordCounter-1 < 0)
		return;
	if (nRoot == NULL)
		return;

	//at correct level of trie to find matching words; recursively DFS from here;
	//if a valid beginning of a word is found, call printWord
	findWords(nRoot->leftMostChild, wordCounter, numCounter);

	if (nRoot->validStart == true && *wordCounter > 0){
		endOfWords = true;
		//don't display words already displayed
		if (*numCounter <= 0){
			*wordCounter -= 1;
			printWord(nRoot);
			endOfWords = false;
		}
		*numCounter -= 1;
	}
	findWords(nRoot->rightSibling, wordCounter, numCounter);
}


/*delete the chosen word by deleting nodes associated with the word. */
void deleteNode(node *&nRoot, node *&prevRightSib,  node *&prevLeftChild, char word[MAX_WORD], int wordCounter, int length){
	if (wordCounter == length - 1){
		if (nRoot->leftMostChild != NULL){
			nRoot->validStart = false;
			deleteCheck = true;
		}
	}
	if (wordCounter == length)
		return;

	if (nRoot == NULL)
		return;
	//get to appropriate level of tree by moving down # of times = suffix's length
	if (nRoot->letter != word[wordCounter])
		deleteNode(nRoot->rightSibling, nRoot, nRoot, word, wordCounter,length);
	else
		deleteNode(nRoot->leftMostChild, nRoot->leftMostChild, nRoot, word, ++wordCounter,length);

	if (prevLeftChild->leftMostChild == nRoot && nRoot->leftMostChild == NULL){
		prevLeftChild->leftMostChild = nRoot->rightSibling;
		delete(nRoot);
		deleteCheck = true;
	}
	else if (nRoot->leftMostChild == NULL){
		prevRightSib->rightSibling = nRoot->rightSibling;
		delete(nRoot);
		deleteCheck = true;
	}
}


/*Finds the suffix chosen by user in the tree, and updates the position cursor. Receives nRoot (root
of trie), cursor (position tracker), suffix (char array of given suffix), suffixCounter (counter
to check for end of suffix), and length (length of suffix). Returns the suffixCounter. */
int findSuffix(node *nRoot, node *&cursor,  char suffix[MAX_WORD], int suffixCounter, int length){
	if (suffixCounter == length)
		return suffixCounter;
	if (nRoot == NULL)
		return suffixCounter;
	//get to appropriate level of tree by moving down # of times = suffix's length
	cursor = nRoot;
	if (nRoot->letter == suffix[suffixCounter])
		return findSuffix(nRoot->leftMostChild, cursor, suffix, ++suffixCounter,length);
	else
		return findSuffix(nRoot->rightSibling, cursor, suffix, suffixCounter,length);
}


/* Check for valid input and reverse the string given by user. Receives char array
str (suffix or word entered by user) and the sLength (length of string entered by user). */
void checkAndReverseInput(char str[MAX_WORD], int sLength){

	for (int i = 0; i < sLength; i++){
		if (!isalpha(str[i])){
			cout << "Your input is not alphabetic" << endl << "Exiting";
			exit(0);
		}
	}
	strReverse(str);
	return;
}


/*compare suffix and word chosen to delete to differentiate for how to call
delete node. Receives suffix char array, entered word array, and length of word. */
bool compareWords(char suffix[MAX_WORD], char word[MAX_WORD], int length){
	bool sameWord = false;
	for (int i = 0; i < length; i++){
		if (suffix[i] == word[i])
			sameWord = true;
		else
			sameWord = false;
	}
		return sameWord;

}


/* Controlling function. Determines which operation user wants to execute and calls
appropriate functions. Prints the menu, loops until user wishes to quit. */
void menuControl(node *nRoot){
	int sLength = 0, numWords = 0, prevNumWords = 0, numWordsTemp = 0, prevNumTemp = 0;
	int counter = 0;
	char menuChoice, suffix[MAX_WORD], word[MAX_WORD];
	node *cursor = new node;
	bool suffixFound = false, fUsed = false, suffixPrint = false;

	//loop until 'x' selected by user
	while(1){
		printMenuBrief();
		cout << "Please enter command: " << endl;
		cin >> menuChoice;

		switch (menuChoice)
		{
		case 'f':
		    cin >> suffix;
			cursor = nRoot;
			fUsed = true;           //set up length, boolean and cursor for later use
			endOfWords = false;
			suffixPrint = true;
			sLength = strlen(suffix);

			checkAndReverseInput(suffix, sLength);
			counter = findSuffix(nRoot->leftMostChild, cursor, suffix, 0, sLength);
			//suffix not in dictionary
			if (counter != sLength){
				cout << "String not found" << endl;
				cursor = NULL;
				suffixFound = false;
			}
			//suffix is in dictionary
			else{
				cout << "Suffix was found. Congratulations." << endl;
				suffixFound = true;
			}
			break;

		case 'p':           //print n words with given suffix
			//suffix was given and found
			cin >> numWords;
			if (suffixFound){
				numWordsTemp = numWords;
				if (fUsed)
					prevNumWords = 0;
				prevNumTemp = prevNumWords;
				if (suffixPrint && cursor->validStart){
					suffixPrint = false;
					printWord(cursor);
                    //only one word with given suffix
					if (cursor->leftMostChild == NULL)
						endOfWords = true;
					numWordsTemp -= 1;
				}
				if (cursor->leftMostChild != NULL)
					findWords(cursor->leftMostChild, &numWordsTemp, &prevNumTemp);
				if (endOfWords)
					cout << "All of the words have been printed. Please choose another suffix";
                //set up for next round of printing, if any
				prevNumWords += numWords;
				fUsed = false;
				cout << endl;
			}
			//suffix either not found or not given
			else
				cout << "You must choose a valid suffix first. Please try again" << endl;

			break;

		case 'a':           //add a word to trie
			cin >> word;
			sLength = strlen(word);
			checkAndReverseInput(word, sLength);
			enterIntoTrie(nRoot->leftMostChild, nRoot, word, sLength, 0);
			cout << "Word added to dictionary." << endl;
			break;

		case 'd':           //delete a word from trie
			cin >> word;
			//suffix given and located
			if (suffixFound){
				sLength = strlen(word);
				checkAndReverseInput(word, sLength);
				if (compareWords(suffix, word, sLength))
					deleteNode(cursor, cursor, cursor, word, (strlen(suffix))-1, sLength);
				else
					deleteNode(cursor->leftMostChild, cursor->leftMostChild, cursor->leftMostChild, word, strlen(suffix), sLength);
				if (deleteCheck)
					cout << "Word deleted." << endl;
				else
					cout << "Word not found." << endl;
				deleteCheck = false;
				cursor = NULL;
				suffixFound = false;
			}
			//suffix not given or not located
			else
				cout << "You must choose a valid suffix first. Please try again" << endl;
			break;

		case '?':           //print options menu
			printMenuFull();
			break;

		case 'x':           //quit
			return;

		default:            //user input not valid menu choice
			cout << "Invalid menu selection, please try again." << endl;
			cin >> suffix;
			break;
		}
	}
	return;
}


/* initialize root node of trie, read in dictionary from file, prompt user, free allocated memory */
int main(){
	//initialize root of trie; given a character just in case I needed it
	node *nRoot = new node;
	nRoot->letter = '+';
	nRoot->validStart = false;
	nRoot->leftMostChild = NULL;
	nRoot->rightSibling = NULL;
	nRoot->parent = NULL;

	cout << "   Using a trie to find, print, add and delete rhyming words via user input" << endl;
	cout << "         *********************************************************" << endl;
	cout << endl;

	dictionaryRead(nRoot->leftMostChild, nRoot);
	menuControl(nRoot);
	freeMem(nRoot);
	cout << "Exiting program...." << endl;
	return 0;
}
