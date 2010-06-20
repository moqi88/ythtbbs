#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "bbs.h"
#include "ythtlib.h"
#include "ythtbbs.h"


int
main(int argc, char *argv[])
{
    	char boardName[32], *ptr;
	char dirPath[256], delPath[256];
	char tmpDirPath[256], tmpDelPath[256];
	char sysbuf[1024];
	int startNum, endNum, totalDir, totalDel;
	int dirFd, delFd, tmpDirFd, tmpDelFd;
	int i, j;
	struct fileheader dirData, delData;

	if (argc < 3) {
	    printf("Useage: undeletedir boardName startNum endNum.\n"
		    "Where startNum and endNum are sequence number "
		    "in .DELETED file.\n");
	    return 0;
	}

	strncpy(boardName, argv[1], sizeof(boardName));
	startNum = atoi(argv[2]);
	endNum = atoi(argv[3]);

	snprintf(dirPath, sizeof(dirPath), MY_BBS_HOME "/boards/%s/.DIR", 
		boardName);
	snprintf(delPath, sizeof(delPath), MY_BBS_HOME "/boards/%s/.DELETED", 
		boardName);

	if (!file_exist(dirPath) || !file_exist(delPath)) {
	    printf("Cannot open .DIR or .DEL file.\n");
	}
	
	totalDir = file_size(dirPath) / sizeof(struct fileheader);
	totalDel = file_size(delPath) / sizeof(struct fileheader);
	
	snprintf(sysbuf, sizeof(sysbuf), "cp %s %s.bak", dirPath, dirPath);
	system(sysbuf);
	printf("%s has been copied to %s.bak as a backup.\n", dirPath, dirPath);
	snprintf(sysbuf, sizeof(sysbuf), "cp %s %s.bak", delPath, delPath);
	system(sysbuf);
	printf("%s has been copied to %s.bak as a backup.\n", delPath, delPath);

	snprintf(tmpDirPath, sizeof(tmpDirPath), "%s.tmp", dirPath);
	snprintf(tmpDelPath, sizeof(tmpDelPath), "%s.tmp", delPath);
	unlink(tmpDirPath);
	unlink(tmpDelPath);

	if (startNum <= 0) {
	    printf("The startNum must be non-negative.\n");
	    return 0;
	} else if (endNum >= totalDel) {
	    printf("The endNum must be less than %d", totalDel);
	    return 0;
	} else if (startNum >= endNum) {
	    printf("The startNum must be less then %d", endNum);
	    return 0;
	}

	if ((delFd = open(delPath, O_RDWR, 0600)) < 0) {
	    printf("Cannot open .DEL file.\n");
	    return 0;
	} else if ((dirFd = open(dirPath, O_RDWR, 0600)) < 0) {
	    printf("Cannot open .DIR file.\n");
	    close(delFd);
	    return 0;
	} else if ((tmpDirFd = open(tmpDirPath, O_RDWR | O_CREAT, 0600)) < 0) {
	    printf("Cannot create temp .DIR file.\n");
	    close(delFd);
	    close(dirFd);
	    return 0;
	} else if ((tmpDelFd = open(tmpDelPath, O_RDWR | O_CREAT, 0600)) < 0) {
	    printf("Cannot create temp .DEL file.\n");
	    close(delFd);
	    close(dirFd);
	    close(tmpDirFd);
	    return 0;
	}

	for (i = 0; i < startNum; i++) {
	    read(delFd, &delData, sizeof(delData));
	    write(tmpDelFd, &delData, sizeof(delData));
	}

	j = 0;
	for (i = startNum; i < endNum; i++) {
	    read(delFd, &delData, sizeof(delData));
	    lseek(dirFd, j * sizeof(dirData), SEEK_SET);
	    while (read(dirFd, &dirData, sizeof(dirData))) {
		if (dirData.filetime < delData.filetime) {
		    write(tmpDirFd, &dirData, sizeof(dirData));
		    j++;
		} else
		    break;
	    }
	    if (NULL != (ptr = strchr(delData.title, '-')))
		*ptr = 0;
	    write(tmpDirFd, &delData, sizeof(delData));
	}

	for (i = endNum; i < totalDel; i++) {
	    read(delFd, &delData, sizeof(delData)); 
	    write(tmpDelFd, &delData, sizeof(delData));
	}
	for (i = j; i < totalDir; i++) {
	    read(dirFd, &dirData, sizeof(dirData));
	    write(tmpDirFd, &dirData, sizeof(dirData));
	}

	close(dirFd);
	close(delFd);
	close(tmpDirFd);
	close(tmpDelFd);

	rename(tmpDelPath, delPath);
	rename(tmpDirPath, dirPath);

	printf("All done!!\n\n");
	return 0;
}
