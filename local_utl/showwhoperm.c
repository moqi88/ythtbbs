#include "../include/bbs.h"
void permtostr(int perm, char* str);
	
int
main(int argc, char *argv[])
{
	int fd1;
	struct userec rec;
	int size1 = sizeof (rec);
	char str[STRLEN], filename[STRLEN];

	sprintf(filename, MY_BBS_HOME "/" PASSFILE);
	if ((fd1 = open(filename, O_RDONLY, 0660)) == -1) {
		perror("open PASSWDFILE");
		return -1;
	}

	while (read(fd1, &rec, size1) == size1)
		if (		(rec.userlevel&PERM_CLOAK||
				   rec.userlevel&PERM_XEMPT||
				   rec.userlevel&PERM_ACCOUNTS||
				   rec.userlevel&PERM_WELCOME||
				   rec.userlevel&PERM_OVOTE||
				   //rec.userlevel&PERM_CHATCLOAK||
				   rec.userlevel&PERM_SYSOP ||
				   //rec.userlevel&PERM_POSTMASK||
				   rec.userlevel&PERM_ANNOUNCE||
				   rec.userlevel&PERM_OBOARDS||
				   rec.userlevel&PERM_ACBOARD||
				   rec.userlevel&PERM_FORCEPAGE||
				   rec.userlevel&PERM_EXT_IDLE||
				   rec.userlevel&PERM_SPECIAL1||
				   rec.userlevel&PERM_SPECIAL2||
				   rec.userlevel&PERM_SPECIAL3||
				   rec.userlevel&PERM_SPECIAL4||
				   rec.userlevel&PERM_SPECIAL5||
				   rec.userlevel&PERM_SPECIAL6||
				   rec.userlevel&PERM_SPECIAL2) &&
			   rec.userlevel & PERM_BASIC &&
			   rec.userid[0]){
			permtostr(rec.userlevel, str);
			printf("%-15s\t%s\n", rec.userid, str);
		};
	close(fd1);
	return 0;
}

void
permtostr(perm, str)
int perm;
char *str;
{
	int num;
	strcpy(str, "bTCPRp#@XWBA#VS-DOM-F012345678");
	for (num = 0; num < 30; num++)
		if (!(perm & (1 << num)))
			str[num] = '-';
	str[num] = '\0';
}

