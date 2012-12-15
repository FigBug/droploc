// Version 1.0.0
// Roland Rabien
// roland@rabien.com

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
#endif

#ifdef __APPLE__
#define MAX_PATH 1024
#include <wordexp.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define TABLELEN        64
#define BUFFFERLEN      128

#define ENCODERLEN      4

#define PADDINGCHAR     '='
#define BASE64CHARSET   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int decodeBlock(char* input, char* output, int oplen)
{
   int rc = 0;
   char decodedstr[ENCODERLEN + 1] = "";

   decodedstr[0] = input[0] << 2 | input[1] >> 4;
   decodedstr[1] = input[1] << 4 | input[2] >> 2;
   decodedstr[2] = input[2] << 6 | input[3] >> 0;

   strncat(output, decodedstr, oplen-strlen(output));

   return rc;
}

int base64Decode(char* input, char* output, int oplen)
{
   char *charval = 0;

   char decoderinput[ENCODERLEN + 1]    = "";
   char encodingtabe[TABLELEN + 1]      = BASE64CHARSET;

   int index        = 0;
   int asciival     = 0;
   int computeval   = 0;
   int iplen        = 0;
   int rc           = 0;

   output[0] = 0;

   iplen = strlen(input);

   while (index < iplen)
   {
      asciival = (int)input[index];

      if(asciival == PADDINGCHAR)
      {
         rc = decodeBlock(decoderinput, output, oplen);
         break;
      }
      else
      {
         charval = strchr(encodingtabe, asciival);
         if (charval)
         {
            decoderinput[computeval] = charval - encodingtabe;
            computeval = (computeval + 1) % 4;

            if (computeval == 0)
            {
               rc = decodeBlock(decoderinput, output, oplen);
               decoderinput[0] = 0;
               decoderinput[1] = 0;
               decoderinput[2] = 0;
               decoderinput[3] = 0;
            }
         }
      }
      index++;
   }

   return rc;
}

int getHostDBPath(char* path)
{
#ifdef _WIN32
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path))) 
	{
	    strcat(path, "\\Dropbox\\host.db");
		return 1;
	}
	return 0;
#endif
#ifdef __APPLE__
	strcpy(path, "~/.dropbox/host.db");
	
	wordexp_t exp_result;
	wordexp(path, &exp_result, 0);
	strcpy(path, exp_result.we_wordv[0]);
	wordfree(&exp_result); 
	
	return 1;
#endif
}

int findDropboxFolder(char* dropboxPath, int sz)
{
    char path[MAX_PATH];

	if (getHostDBPath(path))
    {
        FILE* fp = fopen(path, "rt");
        if (fp)
        {
            char buff[1024];
            fgets(buff, sizeof(buff), fp);
            fgets(buff, sizeof(buff), fp);

            base64Decode(buff, dropboxPath, sz);
            
            fclose(fp);

#ifdef _WIN32
            DWORD attr = GetFileAttributes(dropboxPath);

            if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
                return 1;
#endif
#ifdef __APPLE__
			struct stat dirStat;
			if ((stat(dropboxPath, &dirStat) == 0) && S_ISDIR(dirStat.st_mode))
				return 1;
#endif
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    char path[MAX_PATH];
    if (findDropboxFolder(path, sizeof(path)))
    {
        printf("%s", path);
    	return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}

