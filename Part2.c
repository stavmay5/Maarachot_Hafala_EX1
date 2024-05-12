/*
        Stav Macri 324084722 - סתיו מכרי
        Nadav Swartz 208296400 - נדב שוורץ
*/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define COMP_PROG_PATH "./comp.out"

char* readLine(int*);
char* strCat(char*, char*);
int strCmp(char*, char*);
size_t strLen(const char* str);
int compileProgram(char*, char*);
int runAndWriteOutputFile(char*, char*, char*);
int checkOutputAndGrade(char*, char*);

int main(int argc, char* argv[])
{
    char* configPath;
    char* sourceFile;
    char* outputFile;
    char* programOutputFile;

    int grades_desc = open("results.csv", O_CREAT | O_RDWR, 0644);

    char* gradesContent;

    if (grades_desc == -1) {
        printf("Couldn't create grades file\n");
        exit(1);
    }

    if (argc != 2) {
        printf("Expected just a config file as an argument\n");
        exit(1);
    }

    configPath = argv[1];
    int fDescriptor = open(configPath, O_RDONLY);

    if (fDescriptor == -1) {
        printf("Couldn't open the config file\n");
        exit(1);
    }

    char* studentPath = readLine(&fDescriptor);
    char* keyboardInputFile = readLine(&fDescriptor);
    char* expectedOutputFile = readLine(&fDescriptor);

    close(fDescriptor);

    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(studentPath)) == NULL) {
        printf("Couldn't open students directory\n");
        exit(1);
    }

    // for each student in students directory
    while ((ent = readdir(dir)) != NULL) {
        sourceFile = strCat(strCat(strCat(strCat(strCat(studentPath, "/"), ent->d_name), "/"), ent->d_name), ".c");
        outputFile = strCat(strCat(strCat(strCat(strCat(studentPath, "/"), ent->d_name), "/"), ent->d_name), ".out");
        programOutputFile = strCat(strCat(strCat(strCat(strCat(studentPath, "/"), ent->d_name), "/"), ent->d_name), ".txt");

        if (strCmp(ent->d_name, ".") || strCmp(ent->d_name, ".."))
            continue;

        if (compileProgram(sourceFile, outputFile) == FALSE) {
            gradesContent = strCat(ent->d_name, ",0");
            if (write(grades_desc, gradesContent, strLen(gradesContent)) == -1) {
                printf("Failed to write grade of student %s\n", ent->d_name);
            }
        }
        else {
            if (runAndWriteOutputFile(outputFile, programOutputFile, keyboardInputFile) == TRUE) {
                char grade[5] = "";
                sprintf(grade, "%d", checkOutputAndGrade(programOutputFile, expectedOutputFile));
                gradesContent = strCat(strCat(ent->d_name, ","), grade);
                if (write(grades_desc, gradesContent, strLen(gradesContent)) == -1) {
                    printf("Failed to write grade of student %s\n", ent->d_name);
                }
            }
            else {
                gradesContent = strCat(ent->d_name, ",0");
                if (write(grades_desc, gradesContent, strLen(gradesContent)) == -1) {
                    printf("Failed to write grade of student %s\n", ent->d_name);
                }
            }
        }
        write(grades_desc, "\n", 1);

        free(sourceFile);
        free(outputFile);
        free(programOutputFile);
    }

    closedir(dir);
    close(grades_desc);

    free(gradesContent);

    free(studentPath);
    free(keyboardInputFile);
    free(expectedOutputFile);

    exit(0);
}

int runAndWriteOutputFile(char* programPath, char* outputFilePath, char* keyboardInputFile)
{
    int status;
    int inputFileDesc = open(keyboardInputFile, O_RDONLY);
    int output_desc = open(outputFilePath, O_RDWR | O_CREAT, 0644);

    if (inputFileDesc == -1) {
        printf("Failed to open input file\n");
        return FALSE;
    }

    if (output_desc == -1) {
        printf("Failed to open file\n");
        return FALSE;
    }

    pid_t pid = fork();

    if (pid == -1) {
        printf("Failed to fork process\n");
        return FALSE;
    }
    else if (pid == 0) { // child process
        if (dup2(inputFileDesc, STDIN_FILENO) == -1) {
            printf("Failed to redirect input\n");
            exit(1);
        }

        if (dup2(output_desc, STDOUT_FILENO) == -1) {
            printf("Failed to redirect output\n");
            exit(1);
        }

        if (execl(programPath, programPath, (char*)NULL) == -1)
            exit(1);
        else
            exit(0);

    }
    else { // parent process
        wait(&status);
        close(output_desc);
        close(inputFileDesc);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            return TRUE;
        return FALSE;

    }
}

int checkOutputAndGrade(char* progOut, char* expectedOut)
{
    pid_t pid = fork();
    int stat;
    int success;

    if (pid == -1) {
        printf("Failed to fork\n");
        return 0;
    }
    else if (pid == 0) { // child process
        if (execl(COMP_PROG_PATH, "comp.out", progOut, expectedOut, NULL) == -1)
            exit(1);
    }
    else { // parent process
        wait(&stat);
        success = WEXITSTATUS(stat);
        return success == 2 ? 100 : 0;
    }
}

int compileProgram(char* sourceFile, char* outputFile)
{
    int stat;

    pid_t pid = fork();

    if (pid == -1) {
        printf("Failed to fork process\n");
        return FALSE;
    }
    else if (pid == 0) { // child process
        if (execlp("gcc", "gcc", "-o", outputFile, sourceFile, NULL) == -1)
            exit(1);
        exit(0);
    }
    else { // parent process
        wait(&stat);
        if (WEXITSTATUS(stat) == 1)
            return FALSE;
        return TRUE;
    }
}

char* readLine(int* fd)
{
    ssize_t bytes_read;
    char* line = (char*)malloc(sizeof(char));
    size_t idx = 0;
    char c;
    while ((bytes_read = read(*fd, &c, sizeof(char))) > 0 && c != '\n') {
        line = (char*)realloc(line, (idx + 1) * sizeof(char) + sizeof(char));
        line[idx++] = c;
    }

    line[idx] = '\0';
    return line;
}

char* strCat(char* str1, char* str2)
{
    size_t str1Len = strLen(str1);
    size_t str2Len = strLen(str2);

    char* result = (char*)malloc((str1Len + str2Len + 1) * sizeof(char));

    strcpy(result, str1);
    strcpy(result + str1Len, str2);

    return result;
}

int strCmp(char* str1, char* str2)
{
    return strcmp(str1, str2) == 0;
}
size_t strLen(const char* str) {
    size_t length = 0;

    // Iterate over the characters until we find the null terminator
    while (str[length] != '\0') {
        ++length;
    }

    return length;
}