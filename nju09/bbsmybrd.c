#include "bbslib.h"
#include <json/json.h>

//secstr == NULL: all boards
//secstr == "": boards that doesnn't belong to any group

struct FolderSet *FavorFolder = NULL;
static int loadFavorBoard();
static int unloadFavorBoard();
static int checkFavorFolder(struct FolderSet *FavorFolder);
static int favorFolder2JSON();
static int boardList2JSON();
static int secTree2JSON();
static int json2Favor();

static int
loadFavorBoard() {
	// WEB 下预定讨论区操作其实不是很频繁，
	// 但是为了统一，还是使用mmap，注意及时mummap
	static time_t loadtime = 0;

	//if (NULL != FavorFolder && loadtime > file_time(filepath))
	//	return 1;

	FavorFolder = _loadFavorFolder(currentuser->userid);
	if (NULL == FavorFolder)
		http_fatal("无法载入收藏夹。");
	loadtime = time(NULL);
	checkFavorFolder(FavorFolder);

	return 1;
}

static int
unloadFavorBoard() {
	if (NULL == FavorFolder)
		return 1;
	_unloadFavorFolder(FavorFolder);
	FavorFolder=NULL;
	return 1;
}

static int
checkFavorFolder(struct FolderSet *FavorFolder) {
	int i;
	for (i = 1; i < FAVOR_BRD_NUM; i++) {
		if (FavorFolder->boards[i][0] == 0)
			continue;
		if (!has_view_perm(currentuser, FavorFolder->boards[i]))
			FavorFolder->boards[i][0] = 0;
	}
	if (FavorFolder->folder[0].boardNum == 0) {
		bzero(FavorFolder, sizeof(struct FolderSet));
		_addFavorBoard(FAVOR_DEFAULT, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_1, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_2, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_3, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_4, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_5, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_6, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_7, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_8, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_9, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_10, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_11, FavorFolder, 0);
		_addFavorBoard(FAVOR_YILU_DEFAULT_12, FavorFolder, 0);
		return 1;
	}
	_fixFavorFolder(FavorFolder);
	return 1;
}

static int
boardList2JSON() {
	struct json_object *jsonBoardList;
	struct json_object *jsonBoard[MAXBOARD];
	struct boardmem *tmpBoard;
	int i;

	jsonBoardList = json_object_new_array();

	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++) {
		jsonBoard[i] = json_object_new_object();
		tmpBoard = &(shm_bcache->bcache[i]);
		if (has_view_perm_x(currentuser, tmpBoard)) {
			json_object_object_add(jsonBoard[i], "filename", 
					json_object_new_string(
						tmpBoard->header.filename));
			json_object_object_add(jsonBoard[i], "title", 
					json_object_new_string(
						tmpBoard->header.title));
			json_object_object_add(jsonBoard[i], "sec1", 
					json_object_new_string(
						tmpBoard->header.sec1));
			json_object_object_add(jsonBoard[i], "sec2", 
					json_object_new_string(
						tmpBoard->header.sec2));
			json_object_array_add(jsonBoardList, jsonBoard[i]);
		}
	}
	printf("%s", json_object_to_json_string(jsonBoardList));
	for (i = 0; i < MAXBOARD && i < shm_bcache->number; i++)
		json_object_put(jsonBoard[i]);
	json_object_put(jsonBoardList);
	return 1;
}

static int
secTree2JSON() {
	const struct sectree *sec;
	struct sectree *subsec;
	struct json_object *secTree, *secName, *secChar;
	int i;

	if (!(sec = getsectree("")))
		return 0;

	secTree = json_object_new_object();
	secName = json_object_new_array();
	secChar = json_object_new_array();
	for (i = 0; i < sec->nsubsec; i++) {
		subsec = (struct sectree *) sec->subsec[i];
		json_object_array_add(secName, 
				json_object_new_string(subsec->title));
		json_object_array_add(secChar,
				json_object_new_string(subsec->basestr));
	}
	json_object_object_add(secTree, "secName", secName);
	json_object_object_add(secTree, "secChar", secChar);
	printf("%s", json_object_to_json_string(secTree));
	json_object_put(secName);
	json_object_put(secChar);
	json_object_put(secTree);
	return 1;
}

static int
favorFolder2JSON() {
	struct json_object *jsonFavor, *jsonSubFolder;
	struct json_object *jsonFolders[FOLDER_NUM];
	struct json_object *jsonFolderBidx[FOLDER_NUM];
	struct json_object *jsonBrdList;
	struct json_object *jsonBNumList;
	struct SubFolder *tmpSubFolder;
	int i, j;
	
	if (NULL == FavorFolder)
		http_fatal("收藏夹错误。");

	jsonFavor = json_object_new_object();
	json_object_object_add(jsonFavor, "folderNum", 
			json_object_new_int(FavorFolder->folderNum));
	json_object_object_add(jsonFavor, "boardNum", 
			json_object_new_int(FavorFolder->boardNum));

	jsonSubFolder = json_object_new_array();
	for (i = 0; i < FOLDER_NUM; i++) {
		tmpSubFolder = &(FavorFolder->folder[i]);
		jsonFolders[i] = json_object_new_object();
		json_object_object_add(jsonFolders[i], "name", 
				json_object_new_string(tmpSubFolder->name));
		json_object_object_add(jsonFolders[i], "desc", 
				json_object_new_string(tmpSubFolder->desc));
		jsonFolderBidx[i] = json_object_new_array();
		for (j = 0; j < FOLDER_BRD_NUM; j++) {
			json_object_array_add(jsonFolderBidx[i],
					json_object_new_int(
						tmpSubFolder->bidx[j]));
		}
		json_object_object_add(jsonFolders[i], "bidx", 
				jsonFolderBidx[i]);
		json_object_object_add(jsonFolders[i], "boardNum", 
				json_object_new_int(tmpSubFolder->boardNum));
		json_object_array_add(jsonSubFolder, jsonFolders[i]);
	}
	json_object_object_add(jsonFavor, "folder", jsonSubFolder);

	jsonBrdList = json_object_new_array();
	jsonBNumList = json_object_new_array();
	for (i = 0; i < FAVOR_BRD_NUM + 1; i++) {
		json_object_array_add(jsonBrdList, 
				json_object_new_string(FavorFolder->boards[i]));
		json_object_array_add(jsonBNumList,
				json_object_new_int(getbnum(
						FavorFolder->boards[i])));
	}
	json_object_object_add(jsonFavor, "boards", jsonBrdList);
	json_object_object_add(jsonFavor, "bNum", jsonBNumList);

	printf("%s", json_object_to_json_string(jsonFavor));
	for (i = 0; i < FOLDER_NUM; i++) {
		json_object_put(jsonFolderBidx[i]);
		json_object_put(jsonFolders[i]);
	}
	json_object_put(jsonBrdList);
	json_object_put(jsonBNumList);
	json_object_put(jsonSubFolder);
	json_object_put(jsonFavor);
	return 1;
}

static int
json2Favor() {
	struct json_object *tmpJSONFavor;
	struct json_object *tmpJSONFolderArray;
	struct json_object *tmpJSONFolderObject[FOLDER_NUM];
	struct json_object *tmpJSONBoardsArray;
	struct json_object *tmpJSONBidxArray[FOLDER_NUM];
	struct json_object *tmpJSONFolderName[FOLDER_NUM];
	struct json_object *tmpJSONFolderDesc[FOLDER_NUM];
	struct FolderSet tmpFavor;
	int i, j, fd, tmpBidx, nFolderBrd = 0, nTotalBrd = 0, nFolder = 0;
	char filepath[256];
	char *jsonStr, tmpFolderName[32], tmpFolderDesc[32], tmpBoardName[24];

	jsonStr = getparm("saveJSONString");
	tmpJSONFavor = json_tokener_parse(jsonStr);
	bzero(&tmpFavor, sizeof(struct FolderSet));
	if (!json_object_is_type(tmpJSONFavor, json_type_object)) {
		printf("<script>saveMsg = '<span class=\"red\">"
				"数据错误，无法保存。</span>';</script>");
		return 0;
	}
	if (!(tmpJSONFolderArray = 
				json_object_object_get(tmpJSONFavor, "folder")) 
			|| !json_object_is_type(tmpJSONFolderArray, 
				json_type_array) 
			|| json_object_array_length(tmpJSONFolderArray) 
				!= FOLDER_NUM) {
		printf("<script>saveMsg = '<span class=\"red\">"
				"数据错误，无法保存。</span>';</script>");
		return 0;
	}
	if (!(tmpJSONBoardsArray = 
				json_object_object_get(tmpJSONFavor, "boards"))
			|| !json_object_is_type(tmpJSONBoardsArray,
				json_type_array)
			|| json_object_array_length(tmpJSONBoardsArray)
				!= FAVOR_BRD_NUM + 1) {
		printf("<script>saveMsg = '<span class=\"red\">"
				"数据错误，无法保存。</span>';</script>");
		return 0;
	}
	for (i = 1; i < FAVOR_BRD_NUM + 1; i++) {
		strncpy(tmpBoardName, 
				json_object_get_string(
					json_object_array_get_idx(
						tmpJSONBoardsArray, i)), 
				sizeof(tmpBoardName));
		if (!*tmpBoardName)
			continue;
		if (!has_view_perm(currentuser, tmpBoardName))
			continue;
		strncpy(tmpFavor.boards[i], tmpBoardName, sizeof(
					tmpFavor.boards[i]));
		nTotalBrd++;
	}
	tmpFavor.boardNum = nTotalBrd;
	for (i = 0; i < FOLDER_NUM; i++) {
		tmpJSONFolderObject[i] = NULL;
		tmpJSONFolderName[i] = NULL;
		tmpJSONFolderDesc[i] = NULL;
		tmpJSONBidxArray[i] = NULL;
		nFolderBrd = 0;
		if (!(tmpJSONFolderObject[i] = json_object_array_get_idx(
					tmpJSONFolderArray, i))
				|| !json_object_is_type(tmpJSONFolderObject[i], 
					json_type_object))
			continue;
		if (!(tmpJSONFolderName[i] = json_object_object_get(
					tmpJSONFolderObject[i], "name")) 
				|| !json_object_is_type(tmpJSONFolderName[i], 
					json_type_string))
			continue;
		strncpy(tmpFolderName, 
				json_object_get_string(tmpJSONFolderName[i]), 
				sizeof(tmpFolderName));
		if (!*tmpFolderName)
			continue;
		if (!(tmpJSONFolderDesc[i] = json_object_object_get(
					tmpJSONFolderObject[i], "desc"))
				|| !json_object_is_type(tmpJSONFolderDesc[i], 
					json_type_string))
			continue;
		strncpy(tmpFolderDesc,
				json_object_get_string(tmpJSONFolderDesc[i]), 
				sizeof(tmpFolderDesc));
		if (!(tmpJSONBidxArray[i] = json_object_object_get(
					tmpJSONFolderObject[i], "bidx")) 
				|| !json_object_is_type(tmpJSONBidxArray[i], 
					json_type_array) 
				|| json_object_array_length(
					tmpJSONBidxArray[i]) != FOLDER_BRD_NUM)
			continue;
		for (j = 0; j < FOLDER_BRD_NUM; j++) {
			tmpBidx = json_object_get_int(
					json_object_array_get_idx(
						tmpJSONBidxArray[i], j));
			if (tmpBidx < 0 || tmpBidx > FAVOR_BRD_NUM)
				continue;
			if (!*tmpFavor.boards[tmpBidx])
				continue;
			tmpFavor.folder[i].bidx[j] = tmpBidx;
			nTotalBrd++;
			
		}
		strncpy(tmpFavor.folder[i].name, tmpFolderName, 
				sizeof(tmpFavor.folder[i].name));
		strncpy(tmpFavor.folder[i].desc, tmpFolderDesc, 
				sizeof(tmpFavor.folder[i].desc));
		tmpFavor.folder[i].boardNum = nTotalBrd;
		nFolder++;
	}
	tmpFavor.folderNum = nFolder;
	
	for (i = 0; i < FOLDER_NUM; i++) {
		json_object_put(tmpJSONFolderName[i]);
		json_object_put(tmpJSONFolderDesc[i]);
		json_object_put(tmpJSONBidxArray[i]);
		json_object_put(tmpJSONFolderObject[i]);
	}
	json_object_put(tmpJSONBoardsArray);
	json_object_put(tmpJSONFolderArray);
	json_object_put(tmpJSONFavor);
	
	unloadFavorBoard();
	sethomefile(filepath, currentuser->userid, FAVOR_FILE);
	if ((fd = open(filepath, O_RDWR | O_CREAT, 0600)) < 0) {
		printf("<script>saveMsg = '<span class=\"green\">"
			"保存失败。</span>';</script>");
		return 0;
	}
	flock(fd, LOCK_EX);
	lseek(fd, 0, SEEK_SET);
	write(fd, &tmpFavor, sizeof(struct FolderSet));
	flock(fd, LOCK_UN);
	close(fd);
	loadFavorBoard();
	printf("<script>saveMsg = '<span class=\"green\">"
			"保存成功。</span>';</script>");
	return 1;
}

int
bbsmybrd_show_left(unsigned int mybrdmode) {
	loadFavorBoard();
	printf("<div id = \"favorTree\"></div>\n");
	printf("<script>var jsonFavor = '");
	favorFolder2JSON();
	printf("';\nvar jsonBoardList = '");
	boardList2JSON();
	unloadFavorBoard();
	printf("';\ndisplayFavorTree(%d);</script>\n", mybrdmode);
	return 1;
}

int
bbsmybrd_main()
{
	int action;
	html_header(1);
	//check_msg();
	if (!loginok || isguest)
		http_fatal("尚未登录或者超时");
	//printf("Content-type: text/javascript; charset=%s\r\n\r\n", CHARSET);
	changemode(ZAP);
	action = atoi(getparm("action"));
	loadFavorBoard();
	printf("<script src=\"" BBSJSONJS "\"></script>\n");
	printf("<script src=\"" BBSBRDJS "\"></script>\n");
	printf("</head><body>\n");
	switch (action) {
		case 1:
			json2Favor();
		default:
			printf("<script>\n");
			printf("jsonFavor = '");
			favorFolder2JSON();
			printf("';\n");
			printf("jsonBoardList = '");
			boardList2JSON();
			printf("';\n");
			printf("jsonSecTree = '");
			secTree2JSON();
			printf("';\n");
			printf("displayFavorSetup();\n</script>\n");
			
	}
	unloadFavorBoard();
	http_quit();
	return 0;
}

