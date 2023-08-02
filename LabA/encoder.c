#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void encode(FILE *infile, FILE *outfile, int encodingMode, char *encodingStr);
int encryptAscii(int lower, int higher, int encodingMode, int inputChar, char *encodingStr, int loop);

int main(int argc, char **argv)
{
    FILE *infile = stdin;
    FILE *outfile = stdout;
    char *encodingSign;
    int deBugMode = 0;
    int encodingMode = 0;
    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "+D", 2) == 0)
        {
            deBugMode = 1;
        }
        if (deBugMode)
        {
            fprintf(stderr, "%s\n", argv[i]);
        }
        if (strncmp(argv[i], "-D", 2) == 0)
        {
            deBugMode = 0;
        }
        if (strncmp(argv[i], "+e", 2) == 0)
        {
            encodingMode = 1;
            encodingSign = argv[i] + 2;
        }
        if (strncmp(argv[i], "-e", 2) == 0)
        {
            encodingMode = -1;
            encodingSign = argv[i] + 2;
        }
        if (strncmp(argv[i], "-o", 2) == 0)
        {
            outfile = fopen(argv[i] + 2, "w");
        }
        if (strncmp(argv[i], "-i", 2) == 0)
        {
            infile = fopen(argv[i] + 2, "r");
            if (!infile)
            {
                fprintf(stderr, "an error has occured while trying to open a file\n");
                return 1; // exit(1);
            }
        }
    }
    if (encodingMode != 0)
        encode(infile, outfile, encodingMode, encodingSign);
    fclose(infile);
    fclose(outfile);
}

void encode(FILE *input, FILE *output, int encodingMode, char *encodingStr)
{
    int outputChar;
    int inputChar;
    int loop = 0;
    // inputChar = fgetc(input);
    while (inputChar != EOF)
    {
        inputChar = fgetc(input);
        if (encodingStr[loop] == '\0')
        {
            loop = 0;
        }
        if (inputChar != EOF)
            outputChar = inputChar;
        if (inputChar >= 'a' && inputChar <= 'z')
        {
            outputChar = encryptAscii('a', 'z', encodingMode, inputChar, encodingStr, loop);
        }
        else if (inputChar >= 'A' && inputChar <= 'Z')
        {
            outputChar = encryptAscii('A', 'Z', encodingMode, inputChar, encodingStr, loop);
        }
        else if (inputChar >= '0' && inputChar <= '9')
        {
            outputChar = inputChar + encodingMode * (encodingStr[loop] - '0');
        }
        loop = loop + 1;

        fputc(outputChar, output);
    }
}

int encryptAscii(int lower, int higher, int encodingMode, int inputChar, char *encodingStr, int loop)
{
    int outputChar;
    outputChar = inputChar + encodingMode * (encodingStr[loop] - '0');
    if (outputChar > higher) // case > z/Z
        outputChar = outputChar - higher + lower - 1;
    if (outputChar < lower) // case < a/A
        outputChar = higher - (lower - outputChar) + 1;
    return outputChar;
}