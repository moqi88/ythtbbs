#include "ythtlib.h"
#include "bbs.h"
#include "sysrecord.h"
#include <regex.h>

#define STAT_FILE "bbstmpfs/tmp/diskfree"

int main() {
	FILE *fp;
	char title[128], board[30] = "sysop", *buffer;
	char *pattern[16] = {"100%",	"9[5-9]%", 	"9[0-4]%", 	"8.%",		""};
	char *desc[32] =  {"Full", 	"Dangerous", 	"Caution", 	"Notice",	"Fine"};
	regex_t re;
	int i, size, err = -1;
	time_t now_t;
	struct tm *t;

	chdir(MY_BBS_HOME);

	// generate df msg file
	sprintf(title, "df -m > %s", STAT_FILE);
	if(system(title) == -1)
		return err;
	size = file_size(STAT_FILE);
	if(size <= 0)
		return err;

	// open the stat file and read to buferr
	if((fp = fopen(STAT_FILE, "r")) == NULL)
		return err;
	buffer = malloc(size + 1);
	if(buffer == NULL)
		goto END;

	fread(buffer, size, 1, fp);
	buffer[size] = 0;
	
	for(i=0; pattern[i][0]; i++) {
		// compile regular expression
		err = regcomp (&re, pattern[i], REG_NOSUB);
		if(err)
			goto FREE_END;
		// execute pattern match
       		err = regexec (&re, buffer, 0, NULL, 0);
		if (err == REG_NOMATCH)
			continue;
		if(err)
			goto FREE_END;
		break;
	}
	now_t = time(NULL);
	t = localtime(&now_t);
	sprintf(title, "Disk Space: %s @[%d-%d-%d %d:%d]", desc[i], 
		t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
	if(!strcmp(MY_BBS_ID, "yilubbs"))
		strcpy(board, "CI_INFO");
	err = postfile(STAT_FILE, "deliver", board, title);
		
FREE_END:
	free(buffer);
END:
	fclose(fp);
	unlink(STAT_FILE);
	return err;
}
	
		
		
		

	
	

	

	

