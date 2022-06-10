#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

typedef struct Word
{
    char *Word;
    long Length;
    long StringNumber;
    long PositionInString;
    bool IsInMatch;
} Word;

typedef struct Text
{
    Word *Words;
    long Length;
} Text;

typedef struct PartInfo
{
    long StartIndex;
    long EndIndex;
    int ThreadNumber;
} PartInfo;

typedef struct MatchingPartInfo
{
    int ThreadNumber;
    
    long SplittedTextStartStringNumber;
    long SplittedTextPositionInStartString;
    long SplittedTextEndStringNumber;
    long SplittedTextPositionInEndString;

    long FullTextStartStringNumber;
    long FullTextPositionInStartString;
    long FullTextEndStringNumber;
    long FullTextPositionInEndString;
    
    char *Content;

    struct MatchingPartInfo *Next;
} MatchingPartInfo;

#define AlphabetLength 52

const char Alphabet[] = { 'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F',
    'f', 'G', 'g', 'H', 'h', 'I', 'i', 'J', 'j', 'K', 'k', 'L', 'l', 'M', 'm',
    'N', 'n', 'O', 'o', 'P', 'p', 'Q', 'q', 'R', 'r', 'S', 's', 'T', 't', 'U',
    'u', 'V', 'v', 'W', 'w', 'X', 'x', 'Y', 'y', 'Z', 'z' };

Text SplittedText, FullText;

bool TryParseLong(const char *str, long *result)
{
    errno = 0;
    char *error;
    long res = strtol(str, &error, 0);
    if (error == str || *error != '\0' || ((res == LONG_MIN || res == LONG_MAX) && errno == ERANGE))
    {
        fprintf(stderr, "Could not convert '%s' to long or value is out of range\n", str);
        return false;
    }
    *result = res;
    return true;
}

bool TryOpenAndReadFile(const char *filename, char **res, long *size)
{
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) 
    {
        perror("Cannot open in-file\n");
        return false;
    }
	
	if (fseek(file, 0, SEEK_END) == -1)
	{
		perror("Cannot reset in-file");
		return false;
	}
	
	long length = ftell(file);
	if (length == -1)
	{
		perror("Cannot get in-file length");
		return false;
	}
	
	rewind(file);
	
	char *result = (char *)malloc((length) * sizeof(char));
	if (result == NULL)
	{
		perror("Cannot allocate memory for buffer");
		return false;
	}
	
    char ch;
	for (long i = 0; i < length; ++i)
    {
        ch = getc(file);
        if (ch == EOF)
        {
            free(result);
            perror("Cannot get char from in-file");
		    return false; 
        }
        result[i] = ch;
    }
		
	if (fclose(file) != 0)
	{
		free(result);
        perror("Cannot close in-file");
		return false;
	}
	
    *res = result;
    *size = length;
	return true;
}

bool IsInAlphabet(char symb)
{
    for (int i = 0; i < AlphabetLength; ++i)
    {
        if (Alphabet[i] == symb)
        {
            return true;
        }
    }
    return false;
}

long GetWordsCount(char *content, long length)
{
    long wordsCount = 0;
    long i = 0;
    do 
    {
        while (i < length && !IsInAlphabet(content[i]))
        {
            ++i;
        }
        if (i < length)
        {
            ++wordsCount;
            while (i < length && IsInAlphabet(content[i]))
            {
                ++i;
            }
        }
    }
    while (i < length);
        
    return wordsCount;
}

Word *GetWords(char *content, long contentLength, long wordsCount)
{
    if (wordsCount == 0)
    {
        return NULL;
    }

    Word *result = malloc(sizeof(Word) * wordsCount);
    long wordNumber = 0;
    long i = 0;
    long stringNumber = 1, positionInString = 1;

    do 
    {
        while (i < contentLength && !IsInAlphabet(content[i]))
        {
            if (content[i] != '\n')
            {
                ++positionInString;
            }
            else
            {
                ++stringNumber;
                positionInString = 1;
            }
            ++i;
        }
        if (i < contentLength)
        {
            long wordStartPos = i;
            while (i < contentLength && IsInAlphabet(content[i]))
            {
                ++i;
            }
            
            long length = i - wordStartPos;
            char *word = malloc(length * sizeof(char));
            strncpy(word, content + wordStartPos, length);
            
            result[wordNumber].Length = length;
            result[wordNumber].Word = word;
            result[wordNumber].StringNumber = stringNumber;
            result[wordNumber].PositionInString = positionInString;
            result[wordNumber].IsInMatch = false;
            ++wordNumber;
        }
    }
    while (i < contentLength && wordNumber < wordsCount);

    return result;
}

Text ParseWords(char *content, long length)
{
    Text text;

    text.Length = GetWordsCount(content, length);
    text.Words = GetWords(content, length, text.Length);
    
    free(content);
    return text;
}

bool IsWordsEqual(Word *first, Word *second)
{
    if (first->Length != second->Length)
    {
        return false;
    }

    for (int i = 0; i < first->Length; ++i)
    {
        char firstChar = toupper(first->Word[i]), secondChar = toupper(second->Word[i]);
        if (firstChar != secondChar)
        {
            return false;
        }
    }

    return true;
}

void ResetText(Text *text)
{
    for (int i = 0; i < text->Length; ++i)
    {
        text->Words[i].IsInMatch = false;
    }
}

MatchingPartInfo *CreateMatchingPartInfo()
{
    MatchingPartInfo *result = malloc(sizeof(MatchingPartInfo));
    result->Content = '\0';
    result->Next = NULL;
    return result;
}

void *ComparePartWithFullText(void *partInfo)
{
	ResetText(&SplittedText);
    ResetText(&FullText);
    
    PartInfo *pi = (PartInfo *)partInfo;
    MatchingPartInfo *result = CreateMatchingPartInfo();
    MatchingPartInfo *currentMatch = result;

    for (int i = 0; i < FullText.Length; ++i)
    {
        long k = pi->StartIndex;
        while (k <= pi->EndIndex)
        {
            while (k <= pi->EndIndex && (!IsWordsEqual(&SplittedText.Words[k], &FullText.Words[i]) || SplittedText.Words[k].IsInMatch))
            {
                ++k;
            }

            if (k <= pi->EndIndex)
            {
                long startWordInSplittedText = k;
                long currentWordInFullText = i;
                while (k <= pi->EndIndex && currentWordInFullText < FullText.Length \
                    && !SplittedText.Words[k].IsInMatch && IsWordsEqual(&SplittedText.Words[k], &FullText.Words[currentWordInFullText]))
                {
                    SplittedText.Words[k].IsInMatch = true;
                    ++k;
                    ++currentWordInFullText;
                } 
                --k;
                --currentWordInFullText;
                
                currentMatch->Next = CreateMatchingPartInfo();
                currentMatch = currentMatch->Next;

                currentMatch->ThreadNumber = pi->ThreadNumber;

                currentMatch->SplittedTextStartStringNumber = SplittedText.Words[startWordInSplittedText].StringNumber;
                currentMatch->SplittedTextPositionInStartString = SplittedText.Words[startWordInSplittedText].PositionInString;
                currentMatch->SplittedTextEndStringNumber = SplittedText.Words[k].StringNumber;
                currentMatch->SplittedTextPositionInEndString = SplittedText.Words[k].PositionInString;

                currentMatch->FullTextStartStringNumber = FullText.Words[i].StringNumber;
                currentMatch->FullTextPositionInStartString = FullText.Words[i].PositionInString;
                currentMatch->FullTextEndStringNumber = FullText.Words[currentWordInFullText].StringNumber;
                currentMatch->FullTextPositionInEndString = FullText.Words[currentWordInFullText].PositionInString;

                long contentLength = 0;
                for (int i = startWordInSplittedText; i <= k; ++i)
                {
                    contentLength += SplittedText.Words[i].Length + 1; 
                }

                long posInContent = 0;
                currentMatch->Content = malloc(sizeof(char) * contentLength);
                for (int i = startWordInSplittedText; i <= k; ++i)
                {
                    strcpy(currentMatch->Content + posInContent, SplittedText.Words[i].Word);
                    posInContent += SplittedText.Words[i].Length;
                    currentMatch->Content[posInContent] = ' ';
                    ++posInContent;        
                }
                currentMatch->Content[posInContent - 1] = '\0';

                ++k;
            }
        }
    }

    return (void *)result;
}

void ReleaseMatchResources(MatchingPartInfo *matches[], long length)
{
    for (int i = 0; i < length; ++i)
    {
        MatchingPartInfo *currentMatch = matches[i], *prevMatch;
        while (currentMatch != NULL)
        {
            free(currentMatch->Content);
            prevMatch = currentMatch;
            currentMatch = currentMatch->Next;
            free(prevMatch);
        }
    }
}

void ReleaseTextResources(Text *text)
{
    for (int i = 0; i < text->Length; ++i)
    {
        free(text->Words[i].Word);
    }
    free(text->Words);
}

void ReleasePartInfoResources(PartInfo *partInfos[], long lastIndex)
{
    for (int i = 0; i <= lastIndex; ++i)
    {
        free(partInfos[i]);
    }
}

void PrintMatches(MatchingPartInfo *matches[], long length, char *outFileName, char *splittedFileName, char *fullFileName)
{
    FILE *file;
    if ((file = fopen(outFileName, "w")) == NULL) 
    {
        perror("Cannot open out-file\n");
        return;
    }

    for (int i = 0; i < length; ++i)
    {
        MatchingPartInfo *currentMatch = matches[i]->Next;
        while (currentMatch != NULL)
        {
            if (fprintf(file, "\nThread number: %d\nFirst file name: %s\n\tStart ln [%ld] start ln pos [%ld] end ln [%ld] end ln pos [%ld]\nSecond file name: %s\n\tStart ln [%ld] start ln pos [%ld] end ln [%ld] end ln pos [%ld]\nContent: %s\n\n",  
                currentMatch->ThreadNumber, splittedFileName, currentMatch->SplittedTextStartStringNumber, currentMatch->SplittedTextPositionInStartString,  
                currentMatch->SplittedTextEndStringNumber, currentMatch->SplittedTextPositionInEndString, fullFileName,  
                currentMatch->FullTextStartStringNumber, currentMatch->FullTextPositionInStartString, currentMatch->FullTextEndStringNumber,  
                currentMatch->FullTextPositionInEndString, currentMatch->Content) < 0)
            {
                perror("Cannot write to out-file");
                return;
            }   

            printf("\nThread number: %d\nFirst file name: %s\n\tStart ln [%ld] start ln pos [%ld] end ln [%ld] end ln pos [%ld]\nSecond file name: %s\n\tStart ln [%ld] start ln pos [%ld] end ln [%ld] end ln pos [%ld]\nContent: %s\n\n",  
                currentMatch->ThreadNumber, splittedFileName, currentMatch->SplittedTextStartStringNumber, currentMatch->SplittedTextPositionInStartString,  
                currentMatch->SplittedTextEndStringNumber, currentMatch->SplittedTextPositionInEndString, fullFileName,  
                currentMatch->FullTextStartStringNumber, currentMatch->FullTextPositionInStartString, currentMatch->FullTextEndStringNumber,  
                currentMatch->FullTextPositionInEndString, currentMatch->Content);
            

            currentMatch = currentMatch->Next;
        }
    }

    if (fclose(file) != 0)
	{
		perror("Cannot close out-file");
	}
}

int main(int argc, char *argv[])
{
	long firstFileLength = 0;
    long secondFileLength = 0;
    char *firstFileContent = NULL, *secondFileContent = NULL, *splittedFileName = NULL, *fullFileName = NULL;
    
    // argc = 5;
    // char *newArgs[] = {"Che", "first", "second", "3", "out"};
    // argv = newArgs;

    if (argc != 5)
	{
		fprintf(stderr, "Error! Invalid operands count!\n\t1 - first file name\n\t2 - second file name\n\t3 - threads count\n\t4 - output file name");
		return -1;
	}
	
	long threadCount;
    if (!TryParseLong(argv[3], &threadCount))
    {
        return -1;
    }
	
    if (!TryOpenAndReadFile(argv[1], &firstFileContent, &firstFileLength))
    {
        return -1;
    }

    if (!TryOpenAndReadFile(argv[2], &secondFileContent, &secondFileLength))
    {
        free(firstFileContent);
        return -1;
    }

    SplittedText = ParseWords(firstFileContent, firstFileLength);
    FullText = ParseWords(secondFileContent, secondFileLength);
    long blockSize = SplittedText.Length / threadCount;
    splittedFileName = argv[1];
    fullFileName = argv[2];

    if (blockSize == 0)
    {
        if (SplittedText.Length > FullText.Length)
        {
            threadCount = SplittedText.Length;
            blockSize = 1;
            printf("Thread count reduced to %ld", threadCount);
        }
        else
        {
            splittedFileName = argv[2];
            fullFileName = argv[1];
            
            Text tmp = SplittedText;
            SplittedText = FullText;
            FullText = tmp;

            blockSize = SplittedText.Length / threadCount;
            if (blockSize == 0)
            {
                threadCount = SplittedText.Length;
                blockSize = 1;
                printf("Thread count reduced to %ld", threadCount);    
            }
        }
    }

    pthread_t threads[threadCount];
    PartInfo *partInfos[threadCount];
    for (int i = 0; i < threadCount; ++i)
    {
        PartInfo *partInfo = malloc(sizeof(PartInfo));
        partInfos[i] = partInfo;
        partInfo->ThreadNumber = i;
        partInfo->StartIndex = i * blockSize;
        if (i < threadCount - 1)
        {
            partInfo->EndIndex = partInfo->StartIndex + blockSize - 1;
        }
        else
        {
            partInfo->EndIndex = SplittedText.Length - 1;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, ComparePartWithFullText, partInfo) != 0)
        {
            ReleaseTextResources(&SplittedText);
            ReleaseTextResources(&FullText);
            ReleasePartInfoResources(partInfos, i);
            fprintf(stderr, "Cannot create %dth thread", i);
            perror(" ");
            return -1;
        }
        printf("%dth thread created (looking from %ld to %ld in first file)\n", i, partInfo->StartIndex, partInfo->EndIndex);
        threads[i] = thread;
    }

    void *returnValue = NULL;
    MatchingPartInfo *matches[threadCount];
    for (int i = 0; i < threadCount; ++i)
    {
        int joinResult = pthread_join(threads[i], &returnValue);
        if (joinResult != 0 || returnValue == NULL)
        {
            fprintf(stderr, "Cannot join %dth thread", i);
			perror(" ");
        }
        else
        {
            matches[i] = (MatchingPartInfo *)returnValue;
        }
    }

    PrintMatches(matches, threadCount, argv[4], splittedFileName, fullFileName);
    ReleasePartInfoResources(partInfos, threadCount - 1);
    ReleaseMatchResources(matches, threadCount);
    ReleaseTextResources(&SplittedText);
    ReleaseTextResources(&FullText);

    return 0;
}