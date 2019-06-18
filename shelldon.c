/*
 * shelldon interface program

KUSIS ID: 53694  PARTNER NAME: Buğra Sipahioğlu
KUSIS ID: 54020  PARTNER NAME: Orçun Özdemir

 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <dirent.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_HISTORY_COMMANDS 100
#define DIRENT_DIRECTORY_TYPE 4 /*the regular directory type enum is not standart in dirent library, thus defined manually*/

struct codeSearchHistogram
{ /*Holds all the necessary information about codesearching */

  char *matchedFilePaths[100];
  char *matchedLines[100];
  int matchedLineNumbers[100];
  int totalNumberOfLines;
} codeSearchHistogram;

int parseCommand(char inputBuffer[], char *args[], int *background, int *isRedirectionExists, char *redirectionArray[50], char[]);
void printCommandHistory(int, char *[]); /*Prints last 10 commands of user to screen*/
char *removeDoubleQuotationsFromStr(char *);
int isSourceCodeFile(const char *fileName);                  /*Returns 1 if file is ending with ".c"; 0 otherwise.*/
int isTargetWordFoundInLine(char line[], char targetWord[]); /*Returns 1 if target word is in the specific string, line in this case*/
void searchInFilesRecursively(char *startPoint, char *targetString, int recursionHelper, struct codeSearchHistogram *codeSearchHist);
void printCodeSearchHistogram(struct codeSearchHistogram *codeSearchHist);
void searchInTargetedFile(char *targetString, char *targetFilePath, struct codeSearchHistogram *codeSearchHist);
void searchInCurrentFile(char *targetString, struct codeSearchHistogram *codeSearchHist);
void bashTheCrontab(void);
void shapeCrontab(char *text, char *ret[]);
void scheduleMusicViaCrontab(char t[], char hour[], char min[]);

int main(void)
{
  char inputBuffer[MAX_LINE];   /* buffer to hold the command entered */
  int background;               /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE / 2 + 1]; /* command line (of 80) has max of 40 arguments */
  pid_t child;                  /* process id of the child process */
  int status;                   /* result from execv system call*/
  int shouldrun = 1;
  int i, upper;

  /*Part 2: IO Redirection */
  int isRedirectionExists;      /* if there is a '>' in input buffer, this integer will be set to 1 */
  char *redirectionArray[50];   /* stores redirection chars and the file name from user*/
  int redirectedFileDescriptor; /*the redirected file*/

  /* Part 2: History */
  char *commandHistoryArray[MAX_HISTORY_COMMANDS]; /* holds latest 10 commands*/
  int currentHistoryPosition = 0;                  /* holds how many commands entered */
  char wholeUserCommand[MAX_LINE];                 /* contains all the characters in the input buffer (filled in parseCommand)*/
  char *argsArrayForOldCommand[50];                /* since oldCommand is needed to be executed in some cases, args is stored */

  while (shouldrun)
  { /* Program terminates normally inside setup */
    background = 0;
    isRedirectionExists = 0;
    shouldrun = parseCommand(inputBuffer, args, &background, &isRedirectionExists, redirectionArray, wholeUserCommand); /* get next command */

    if (strncmp(inputBuffer, "exit", 4) == 0)
    {
      shouldrun = 0; /* Exiting from shelldon*/
    }

    if (shouldrun)
    {
      /* add the command to histories array*/
      if(args[0][0] != '!') {
        commandHistoryArray[currentHistoryPosition++] = strdup(wholeUserCommand);
      }

      child = fork(); /* create a new child process*/
      if (child == 0)
      { /* child process*/

        if (isRedirectionExists)
        {
          if (strcmp(redirectionArray[0], ">") == 0)
          {
            redirectedFileDescriptor = open(redirectionArray[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH);
          }
          else
          {
            redirectedFileDescriptor = open(redirectionArray[1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | S_IWOTH);
          }

          if (redirectedFileDescriptor < 1)
          {
            perror("File for redirection cannot be created");
            exit(0);
          }

          dup2(redirectedFileDescriptor, 1);
          execvp(args[0], args);
          close(redirectedFileDescriptor);
        }

        else if (strcmp(args[0], "history") == 0)
        {
          printCommandHistory(currentHistoryPosition, commandHistoryArray);
          break;
        }

        else if (strcmp(args[0], "!!") == 0)
        {
          printf("here!");
          //  char oldCommand[strlen(commandHistoryArray[currentHistoryPosition - 2])];
          char oldCommand[50];
          strcpy(oldCommand, commandHistoryArray[currentHistoryPosition - 2]);

          int init_size = strlen(oldCommand);
          char delim[] = " ";
          char *ptr = strtok(oldCommand, delim);
          int argct = 0;
          while (ptr != NULL)
          {
            argsArrayForOldCommand[argct++] = ptr;
            ptr = strtok(NULL, delim);
          }
          argsArrayForOldCommand[argct] = NULL;
          execvp(argsArrayForOldCommand[0], argsArrayForOldCommand);
        }
        else if(args[0][0] == '!' && isdigit(args[0][1])) {

          if (isdigit(args[0][2]))
              { /* Case 2.a: history !N1N2 */
                char numberOfCommandAsString[] = {args[0][1], args[0][2]};
                int numberOfCommand = atoi(numberOfCommandAsString);
                char oldCommand[50];

                strcpy(oldCommand, commandHistoryArray[numberOfCommand]);

                int init_size = strlen(oldCommand);
                char delim[] = " ";
                char *ptr = strtok(oldCommand, delim);
                int argct = 0;
                while (ptr != NULL)
                {
                  argsArrayForOldCommand[argct++] = ptr;
                  ptr = strtok(NULL, delim);
                }
                argsArrayForOldCommand[argct] = NULL;
                execvp(argsArrayForOldCommand[0], argsArrayForOldCommand);
              }
              else
              {
                int firstDigit = args[0][1] - '0';
                //  char oldCommand[strlen(commandHistoryArray[firstDigit])];
                char oldCommand[50];

                strcpy(oldCommand, commandHistoryArray[firstDigit]);
                printf("\n Old command for history %d is %s ", firstDigit, oldCommand);
                int init_size = strlen(oldCommand);
                char delim[] = " ";
                char *ptr = strtok(oldCommand, delim);
                int argct = 0;
                while (ptr != NULL)
                {
                  argsArrayForOldCommand[argct++] = ptr;
                  ptr = strtok(NULL, delim);
                }
                argsArrayForOldCommand[argct] = NULL;
                execvp(argsArrayForOldCommand[0], argsArrayForOldCommand);
              }
          break;
        }







        // else if (args[0][0] == '!')
        //   {
        //     if (args[1][1] == '!') /* Case 1: history !! */
        //     {
        //       //  char oldCommand[strlen(commandHistoryArray[currentHistoryPosition - 2])];
        //       char oldCommand[50];
        //       strcpy(oldCommand, commandHistoryArray[currentHistoryPosition - 2]);
        //       int init_size = strlen(oldCommand);
        //       char delim[] = " ";
        //       char *ptr = strtok(oldCommand, delim);
        //       int argct = 0;
        //       while (ptr != NULL)
        //       {
        //         argsArrayForOldCommand[argct++] = ptr;
        //         ptr = strtok(NULL, delim);
        //       }
        //       argsArrayForOldCommand[argct] = NULL;
        //       execvp(argsArrayForOldCommand[0], argsArrayForOldCommand);
        //     }
        //     else if (isdigit(args[1][1]))
        //     { /* Case 2: history !Nth */

        //       if (isdigit(args[1][2]))
        //       { /* Case 2.a: history !N1N2 */
        //         char numberOfCommandAsString[] = {args[1][1], args[1][2]};
        //         int numberOfCommand = atoi(numberOfCommandAsString);
        //         char oldCommand[50];

        //         strcpy(oldCommand, commandHistoryArray[numberOfCommand]);

        //         int init_size = strlen(oldCommand);
        //         char delim[] = " ";
        //         char *ptr = strtok(oldCommand, delim);
        //         int argct = 0;
        //         while (ptr != NULL)
        //         {
        //           argsArrayForOldCommand[argct++] = ptr;
        //           ptr = strtok(NULL, delim);
        //         }
        //         argsArrayForOldCommand[argct] = NULL;
        //         execvp(argsArrayForOldCommand[0], argsArrayForOldCommand);
        //       }
        //       else
        //       {
        //         int firstDigit = args[1][1] - '0';
        //         //  char oldCommand[strlen(commandHistoryArray[firstDigit])];
        //         char oldCommand[50];

        //         strcpy(oldCommand, commandHistoryArray[firstDigit]);
        //         printf("\n Old command for history %d is %s ", firstDigit, oldCommand);
        //         int init_size = strlen(oldCommand);
        //         char delim[] = " ";
        //         char *ptr = strtok(oldCommand, delim);
        //         int argct = 0;
        //         while (ptr != NULL)
        //         {
        //           argsArrayForOldCommand[argct++] = ptr;
        //           ptr = strtok(NULL, delim);
        //         }
        //         argsArrayForOldCommand[argct] = NULL;
        //         execvp(argsArrayForOldCommand[0], argsArrayForOldCommand);
        //       }
        //     }

            
        //   }


        else if (strncmp(inputBuffer, "codesearch", 10) == 0)
        {
          if (args[1][0] == '"' && args[2] == NULL) /*  Regular Code Search: 'codesearch " foo "' */
          {
            char targetString[50];
            struct codeSearchHistogram codeSearchHist;
            codeSearchHist.totalNumberOfLines = 0;

            /*Remove the double quotation mark from the user input*/
            strcpy(targetString, args[1]);
            strcpy(targetString, removeDoubleQuotationsFromStr(targetString));

            searchInCurrentFile(targetString, &codeSearchHist); /*Store all the search result information in the codeSearchHist struct*/
            printCodeSearchHistogram(&codeSearchHist);
            break;
          }

          if (args[2][0] == '-' && args[2][1] == 'f')
          { /*targeted search: codeseach "foo" -f ./include/util.h */
            char targetString[50], targetFilePath[100];
            struct codeSearchHistogram codeSearchHist;
            codeSearchHist.totalNumberOfLines = 0;

            strcpy(targetFilePath, args[3]);
            strcpy(targetString, args[1]);
            strcpy(targetString, removeDoubleQuotationsFromStr(targetString));
            searchInTargetedFile(targetString, targetFilePath, &codeSearchHist);
            printCodeSearchHistogram(&codeSearchHist);
            break;
          }

          if (args[1][0] == '-' && args[1][1] == 'r')
          { /* shelldon> codesearch -r "foo" //recursive usage */
            char targetString[50];
            struct codeSearchHistogram codeSearchHist;
            codeSearchHist.totalNumberOfLines = 0;
            strcpy(targetString, args[2]);
            strcpy(targetString, removeDoubleQuotationsFromStr(targetString));
            searchInFilesRecursively(".", targetString, 0, &codeSearchHist);
            printCodeSearchHistogram(&codeSearchHist);
          }

          break;
        }

        else if (strcmp(args[0], "birdakika") == 0)
        {

          char songDurationPart[5];
          strcpy(songDurationPart, args[1]);

          char min2[] = {songDurationPart[3], songDurationPart[4], '\0'};
          char hour2[] = {songDurationPart[0], songDurationPart[1], '\0'};

          char *min = "12";
          char *hour = "12";

          int numberInt = atoi(min2);

          char modifiedMin[20];

          int numberIntModifed = numberInt + 1;

          sprintf(modifiedMin, "%d", numberIntModifed);
          scheduleMusicViaCrontab(args[2], hour2, min2);
        }

         else if(strcmp(args[0], "exam") == 0){

           char datePart[20];
           strcpy(datePart, args[3]);

           char timePart[20];
           strcpy(timePart, args[4]);

           char adjustedDay[] = {datePart[0], datePart[1], '\0'};
           char adjustedMonth[] = {datePart[3], datePart[4], '\0'};
           //char adjustedYear[] = {datePart[6],datePart[7],datePart[8],datePart[9]};

           char adjustedMin[] = {timePart[3], timePart[4], '\0'};
           char adjustedHour[] = {timePart[0], timePart[1], '\0'};

           char courseName[20];
           strcpy(courseName, args[2]);

           if (strcmp(args[1], "-add") == 0)
           {

             examWarningAdder(courseName, adjustedDay, adjustedMonth, adjustedHour, adjustedMin);
             bashTheCrontabForExam();

             // examListTracker(courseName,adjustedDay,adjustedMonth,adjustedHour);
           }
           //examListTracker(courseName,adjustedDay,adjustedMonth,adjustedHour);

           break;
         }

         else
         { /* regular UNIX expressions*/
           int userCommandPathLength;
           FILE *fileStream;
           int fd;
           char whichCommand[50], userCommand[50], redirectionPart[50], den[50], userCommandPathAsString[50], userCommandPathBuffer[50];
           /*a command is created for redirecting the absolute path of a command to a file 'userCommandPath'*/
           //char *redirectionArgs[5] = {"which ", args[0], "> ", "userCommanPath", (char *)NULL };
           strcpy(whichCommand, "which ");
           strcpy(userCommand, args[0]);
           strcpy(redirectionPart, " >userCommandPath");
           strcat(whichCommand, userCommand);
           strcat(whichCommand, redirectionPart);
           system(whichCommand); /* execute the command that outputs the absolute path of the command*/
           fileStream = fopen("userCommandPath", "r");
           if (fileStream == NULL)
             printf("NULL ERROR");
           fgets(userCommandPathAsString, 50, fileStream);
           fclose(fileStream);
           printf("userCommandPath: --> %s", userCommandPathAsString);
           system("rm userCommandPath"); /*delete the file since path is already extracted*/
           userCommandPathLength = strlen(userCommandPathAsString);
           strncpy(userCommandPathBuffer, userCommandPathAsString, userCommandPathLength - 1);
           if (execv(userCommandPathBuffer, args) < 0)
           {
             perror("Execv cannot be executed");
           }
        }

        exit(0); /*child terminates itself after finishing its jobs*/
      }

      if (child > 0)
      { /* parent process */
        if (background != 1)
        {
          wait(NULL);
        }
      }
      else
      {
        perror("There was a problem with the creation of child process.");
      }
    }
  }
  return 0;
}

/**
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings.
 */

int parseCommand(char inputBuffer[], char *args[], int *background, int *isRedirectionExists, char *redirectionArray[50], char wholeUserCommand[])
{
  int length,         /* # of characters in the command line */
      i,              /* loop index for accessing inputBuffer array */
      start,          /* index where beginning of next command parameter is */
      ct = 0,         /* index of where to place the next parameter into args[] */
      command_number, /* index of requested command number */
      counterForRedirection = 0,
      redirectionStatus = 0;

  /* read what the user enters on the command line */
  do
  {
    printf("shelldon>");
    fflush(stdout);
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
  } while (inputBuffer[0] == '\n'); /* swallow newline characters */

  /**
     *  0 is the system predefined file descriptor for stdin (standard input),
     *  which is the user's screen in this case. inputBuffer by itself is the
     *  same as &inputBuffer[0], i.e. the starting address of where to store
     *  the command that is read, and length holds the number of characters
     *  read in. inputBuffer is not a null terminated C-string.
     */
  start = -1;
  if (length == 0)
    exit(0); /* ^d was entered, end of user command stream */

  /**
     * the <control><d> signal interrupted the read system call
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check this  value
     * and disregard the -1 value
     */

  if ((length < 0) && (errno != EINTR))
  {
    perror("error reading the command");
    exit(-1); /* terminate with error code of -1 */
  }

  /**
     * Parse the contents of inputBuffer
     */

  for (i = 0; i < length; i++)
  {
    wholeUserCommand[i] = inputBuffer[i];
    /* examine every character in the inputBuffer */

    switch (inputBuffer[i])
    {
    case ' ':
    case '\t': /* argument separators */
      if (start != -1)
      {
        if (redirectionStatus == 1)
        {
          redirectionArray[counterForRedirection++] = &inputBuffer[start];
        }
        else
        {
          args[ct] = &inputBuffer[start]; /* set up pointer */
          ct++;
        }
      }

      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;

    case '\n': /* should be the final char examined */
      if (start != -1)
      {

        if (redirectionStatus == 1)
        {
          redirectionArray[counterForRedirection++] = &inputBuffer[start];
        }
        else
        {
          args[ct] = &inputBuffer[start];
          ct++;
        }
      }

      wholeUserCommand[i] = '\0';
      inputBuffer[i] = '\0';
      args[ct] = NULL; /* no more arguments to this command */
      break;

    default: /* some other character */
      if (start == -1)
        start = i;
      if (inputBuffer[i] == '&')
      {
        *background = 1;
        inputBuffer[i - 1] = '\0';
      }

      if (inputBuffer[i] == '>')
      {
        redirectionStatus = 1;
        *isRedirectionExists = 1;
      }

    } /* end of switch */
  }   /* end of for */

  /**
     * If we get &, don't enter it in the args array
     */

  if (*background)
    args[--ct] = NULL;

  args[ct] = NULL; /* just in case the input line was > 80 */

  return 1;

} /* end of parseCommand routine */

void printCommandHistory(int currentHistoryPosition, char *hist[])
{
  int counter = 10;
  for (int i = currentHistoryPosition - 1; i > 0; i--)
  {
    if (counter == 0)
      break;
    printf("\n%d %s", i, hist[i]);
    counter--;
  }
  printf("\n");
}

char *removeDoubleQuotationsFromStr(char *targetString)
{
  int j;
  for (int i = 0; i < strlen(targetString); i++)
  {

    if (targetString[i] == '"')
    {
      for (j = i; j < strlen(targetString); j++)
      {
        targetString[j] = targetString[j + 1];
      }
    }
  }
  return targetString;
}

int isSourceCodeFile(const char *fileName)
{
  char *fileExtention;
  fileExtention = strrchr(fileName, '.');

  if (fileExtention)
  {
    return strcmp(fileExtention, ".c") == 0;
  }
  return 0;
}

int isTargetWordFoundInLine(char line[], char target[])
{
  int i, j, k, lineLength = strlen(line), targetWordLength = strlen(target), position = -1;

  if (targetWordLength > lineLength)
  {
    return -1;
  }

  for (i = 0; i <= lineLength - targetWordLength; i++)
  {
    position = k;
    k = i;

    for (j = 0; j < targetWordLength; j++)
    {
      if (target[j] == line[k])
      {
        k++;
      }
      else
      {
        break;
      }
    }
    if (j == targetWordLength)
    {
      return position;
    }
  }

  return -1;
}

void printCodeSearchHistogram(struct codeSearchHistogram *codeSearchHist)
{
  int numberOfLines = codeSearchHist->totalNumberOfLines;
  int i;
  if (numberOfLines == 0)
  {
    printf("\ncodesearch couldn't find a match\n");
  }
  else
  {
    for (i = 0; i < numberOfLines; i++) /* 45: ./foo.c -> void foo(int a, int b); */
    {
      printf("%d: %s -> %s", codeSearchHist->matchedLineNumbers[i] + 1, codeSearchHist->matchedFilePaths[i], codeSearchHist->matchedLines[i]);
    }
    printf("\n");
  }
}

void searchInFilesRecursively(char *startPoint, char *targetString, int recursionHelper, struct codeSearchHistogram *codeSearchHist)
{
  DIR *directory;
  struct dirent *dirEntity;
  int dirct, lineNumberInFile, lineIndexForLinesArray;
  FILE *currentFilePointer;
  char *line = NULL;
  size_t len;
  ssize_t nread;
  char workingDirectory[50];
  char parentDirectory[50];
  char childDirectory[50];

  if (!(directory = opendir(startPoint)))
  {
    return;
  }

  while ((dirEntity = readdir(directory)) != NULL)
  {
    if (dirEntity->d_type == DIRENT_DIRECTORY_TYPE)
    {
      char path[500];
      if (strcmp(dirEntity->d_name, ".") == 0 || strcmp(dirEntity->d_name, "..") == 0)
      {
        continue;
      }
      snprintf(path, sizeof(path), "%s/%s", startPoint, dirEntity->d_name);
      searchInFilesRecursively(path, targetString, recursionHelper + 2, codeSearchHist);
    }
    else
    {
      /* right now; path is ./parentDirectory */
      char childDirectory[50];
      strcpy(childDirectory, startPoint);
      strcat(childDirectory, "/");
      strcat(childDirectory, dirEntity->d_name); /* ./parentDirectory/childDirectory*/

      currentFilePointer = fopen(childDirectory, "r"); /*Open the file*/
      if (currentFilePointer != NULL)
      {
        if (isSourceCodeFile(dirEntity->d_name))
        {
          lineNumberInFile = 0;
          while ((nread = getline(&line, &len, currentFilePointer)) != -1)
          {
            if (isTargetWordFoundInLine(line, targetString) != -1)
            {
              codeSearchHist->matchedLines[codeSearchHist->totalNumberOfLines] = strdup(line);
              codeSearchHist->matchedFilePaths[codeSearchHist->totalNumberOfLines] = strdup(childDirectory);
              codeSearchHist->matchedLineNumbers[codeSearchHist->totalNumberOfLines] = lineNumberInFile;
              codeSearchHist->totalNumberOfLines++;
            }
            lineNumberInFile++;
          }
          fclose(currentFilePointer);
          free(line);
        }
      }
    }
  }
  closedir(directory);
}

void searchInTargetedFile(char *targetString, char *targetFilePath, struct codeSearchHistogram *codeSearchHist)
{
  FILE *currentFilePointer;
  char *line = NULL;
  size_t len;
  ssize_t nread;
  int lineNumberInFile;
  line = NULL;
  len = 0;
  currentFilePointer = fopen(targetFilePath, "r");
  if (currentFilePointer != NULL)
  {
    lineNumberInFile = 0;
    while ((nread = getline(&line, &len, currentFilePointer)) != -1)
    {
      if (isTargetWordFoundInLine(line, targetString) != -1)
      {
        codeSearchHist->matchedLines[codeSearchHist->totalNumberOfLines] = strdup(line);
        codeSearchHist->matchedFilePaths[codeSearchHist->totalNumberOfLines] = strdup(targetFilePath);
        codeSearchHist->matchedLineNumbers[codeSearchHist->totalNumberOfLines] = lineNumberInFile;
        codeSearchHist->totalNumberOfLines++;
      }
      lineNumberInFile++;
    }
    fclose(currentFilePointer);
    free(line);
  }
}

void searchInCurrentFile(char *targetString, struct codeSearchHistogram *codeSearchHist)
{
  DIR *d;
  struct dirent *dir;
  char workingDirectory[50];
  FILE *currentFilePointer;
  char *line = NULL;
  size_t len;
  ssize_t nread;
  int lineNumberInFile;
  line = NULL;
  len = 0;
  int dirct;
  char tempdirName[50];
  d = opendir("."); /* Open the current working directory */
  if (d)
  {
    dirct = 0;
    while ((dir = readdir(d)) != NULL)
    {

      strcpy(tempdirName, dir->d_name);
      currentFilePointer = fopen(tempdirName, "r");
      if (currentFilePointer != NULL)
      {

        if (isSourceCodeFile(dir->d_name))
        {
          strcpy(workingDirectory, "./");
          strcat(workingDirectory, tempdirName);
          lineNumberInFile = 0;

          while ((nread = getline(&line, &len, currentFilePointer)) != -1)
          {

            if (isTargetWordFoundInLine(line, targetString) != -1)
            {
              codeSearchHist->matchedLines[codeSearchHist->totalNumberOfLines] = strdup(line);
              codeSearchHist->matchedFilePaths[codeSearchHist->totalNumberOfLines] = strdup(workingDirectory);
              codeSearchHist->matchedLineNumbers[codeSearchHist->totalNumberOfLines] = lineNumberInFile;
              codeSearchHist->totalNumberOfLines++;
            }

            lineNumberInFile++;
          }
          fclose(currentFilePointer);
          free(line);
        }
      }
    }
    closedir(d);
  }
}
int crontabCreator(char *text, char *hour, char *min)
{

  int numberInt = atoi(min);
  char modifiedMin[20];
  int numberIntModifed = numberInt + 1;
  sprintf(modifiedMin, "%d", numberIntModifed);
  printf("modifedMin: %s", modifiedMin);

  char *line1 = "#!/bin/bash";

  char *line2 = "crontab -l >> cronFile";
  char *con1 = "echo \"";

  char con2[500];
  strcpy(con2, "  * * * mpg321 ");
  char cwd[500];
  getcwd(cwd, sizeof(cwd));
  strcat(con2, cwd);
  strcat(con2, "/");

  char *conline1 = "\" >> cronFile";
  char *line4 = "crontab cronFile";
  char *line5 = "rm -f cronTabFile.txt";
  char *line6 = "rm -f cronFile";
  char *line3 = malloc(strlen(conline1) + strlen(con1) + strlen(hour) + strlen(min) + strlen(con2) + strlen(text) + 1);
  char *line7 = malloc(strlen(conline1) + strlen(con1) + strlen(hour) + strlen(min) + strlen(con2) + 1);

  strcpy(line7, con1);
  strcat(line7, modifiedMin);
  strcat(line7, " ");
  strcat(line7, hour);
  strcat(line7, " * * * pkill mpg321 ");
  strcat(line7, conline1);

  strcpy(line3, con1);
  strcat(line3, min);
  strcat(line3, " ");
  strcat(line3, hour);
  strcat(line3, con2);
  strcat(line3, text);
  strcat(line3, conline1);

  FILE *file = fopen("cronTabFile.txt", "ab+");
  if (file == NULL)
  {
    perror("File could not be created\n");
    return 0;
  }

  fprintf(file, "%s \n", line1);
  fprintf(file, "%s \n", line2);
  fprintf(file, "%s \n", line3);
  fprintf(file, "%s \n", line7);
  fprintf(file, "%s \n", line4);
  fprintf(file, "%s \n", line6);
  fprintf(file, "%s \n", line5);

  fclose(file);
  return 1;
}

void scheduleMusicViaCrontab(char t[], char hour[], char min[])
{
  crontabCreator(t, hour, min);
  bashTheCrontab();
}

void shapeCrontab(char *text, char *ret[])
{
  char arr[100];
  strcpy(arr, text);
  char *token;
  int index = 0;
  token = strtok(arr, " ");
  while (token != NULL)
  {

    ret[index] = malloc(sizeof(char) * 100);
    strcpy(ret[index++], token);
    token = strtok(NULL, " ");
  }
}

void bashTheCrontab()
{

  int i;
  char *temp[100];
  char *text = "bash cronTabFile.txt";
  shapeCrontab(text, temp);
  i = execvp(temp[0], temp);
}
int examWarningAdder(char *course, char *day, char *month, char *hour, char *min){
    
 int numberInt = atoi(day);
 char modifiedDay[100];
 int numberIntModifed = numberInt - 1;
 sprintf(modifiedDay, "%d", numberIntModifed);
 printf("modifedDay: %s\n", modifiedDay);

 char line1Array[100] = "#!/bin/bash";

 char line2Array[100] = "crontab -l >> cronFileExam";
 char con1Array[100] = "echo \"";

 char con2Array[500];
 strcpy(con2Array, " * XDG_RUNTIME_DIR=/run/user/$(id -u) notify-send 'Exam Reminder' ");
//  char cwd[500];
//  getcwd(cwd, sizeof(cwd));
//  strcat(con2, cwd);
//  strcat(con2, "/");

 char conline1Array[100] = "\" >> cronFileExam";
 char line4Array[100] = "crontab cronFileExam";
 char line5Array[100] = "rm -f ExamFile.txt";
 char line6Array[100] = "rm -f cronFileExam";

 char line3Array[500];

 strcpy(line3Array,con1Array);
 strcat(line3Array,min);
 strcat(line3Array," ");
 strcat(line3Array, hour);
 strcat(line3Array," ");
 strcat(line3Array,modifiedDay);
 strcat(line3Array," ");
 strcat(line3Array,month);
 strcat(line3Array, con2Array);
 strcat(line3Array," '");
 strcat(line3Array,course);
 strcat(line3Array,"' ");
 strcat(line3Array,conline1Array);

 printf("LINE 3:  %s\n", line3Array);

 FILE *file = fopen("ExamFile.txt", "ab+");

 fprintf(file, "%s \n", line1Array);
 fprintf(file, "%s \n", line2Array);
 fprintf(file, "%s \n", line3Array);
 fprintf(file, "%s \n", line4Array);
 fprintf(file, "%s \n", line6Array);
 fprintf(file, "%s \n", line5Array);

 fclose(file);


 return 1;

}

void bashTheCrontabForExam()
{

 int i;
 char *temp[100];
 char *text = "bash ExamFile.txt";
 shapeCrontab(text, temp);
 i = execvp(temp[0], temp);
}