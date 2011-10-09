#define MAX_ATTACHMENT_NUM 50

/* record.c */
struct ANSI2HTML_UBB {
	const char *ansi;
	const char *html;
	const int htmllen;
	const char *ubb;
};
struct attachmentstruct
{
	int ano;  // attachment number , id is created in insertion to forum_attachment
	int width;  // picture width, if not image , width = 0
	time_t dateline;  // post time of attachment
	char filename[256];  // download name of attachment
	int filesize;  // size of file
	char attachment[1024];   //  real path and name of attachment
	short int isimage;  // is it an image?  isimage = 0 not image = 1 image
};

/* transythtdiscuzx.c */
long get_num_records(char *filename, int size);
void toobigmesg(void);
int get_record(char *filename, void *rptr, int size, int id);
int get_records(char *filename, void *rptr, int size, int id, int number);
int substitute_record(char *filename, void *rptr, int size, int id);
int update_file(char *dirname, int size, int ent, int (*filecheck)(void), void (*fileupdate)(void));
int ythtfiletodiscuzxubb(char* filepath, char* ubbresult, char* useip, struct attachmentstruct *attinfo);

