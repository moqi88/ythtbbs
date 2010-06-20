/* BBSLIB.c */
void filter(char *line);
int junkboard(char *board);
int file_has_word(char *file, char *word);
int f_write(char *file, char *buf);
int f_append(char *file, char *buf);
int get_record(void *buf, int size, int num, char *file);
int put_record(void *buf, int size, int num, char *file);
int del_record(char *file, int size, int num);
long get_num_records(char *filename, int size);
int insert_record(char *fpath, void *data, int size, int pos, int num);
char *nohtml_textarea(const char *s);
char *nohtml(const char *s);
char *strright(char *s, int len);
char *getsenv(char *s);
int http_quit(void);
void ajax_fatal(char *fmt, ...);
void http_fatal(char *fmt, ...);
void strnncpy(char *s, int *l, const char *s2);
void ansifont(const char *ansi, int len, struct bbsfont *bf);
char *fontstr(struct bbsfont *bf, char *buf);
void hsprintf(char *s, char *s0);
char *titlestr(const char *s);
int hprintf(char *fmt, ...);
void fqhprintf(FCGI_FILE * output, char *str);
int fhhprintf(FCGI_FILE * output, char *fmt, ...);
void parm_add(char *name, char *val);
int isaword(char *dic[], char *buf);
int url_parse(void);
void http_parm_free(void);
void http_parm_init(void);
int cache_header(time_t t, int age);
void html_header(int mode);
void xml_header(void);
int __to16(char c);
void __unhcode(char *s);
char *getparm(char *var);
char *getparm2(char *v1, char *v2);
char *getparmboard(char *buf, int size);
struct parm_file *getparmfile(char *var);
int shm_init(struct bbsinfo *bbsinfo);
int addextraparam(char *ub, int size, int n, int param);
void extraparam_init(unsigned char *extrastr);
int user_init(struct userec **x, struct user_info **y, unsigned char *ub);
int post_mail(char *userid, char *title, char *file, char *id, char *nickname,
	      char *ip, int sig, int mark);
int post_imail(char *userid, char *title, char *file, char *id, char *nickname,
	       char *ip, int sig);
int post_article_1984(char *board, char *title, char *file, char *id,
		      char *nickname, char *ip, int sig, int mark, int outgoing,
		      char *realauthor, int thread);
int post_article(char *board, char *title, char *file, char *id, char *nickname,
		 char *ip, int sig, int mark, int outgoing, char *realauthor,
		 int thread);
int securityreport(char *title, char *content);
void sig_append(FCGI_FILE * fp, char *id, int sig);
char *anno_path_of(char *board);
int has_BM_perm(struct userec *user, struct boardmem *x);
int has_read_perm(struct userec *user, char *board);
int has_read_perm_x(struct userec *user, struct boardmem *x);
int has_view_perm(struct userec *user, char *board);
int has_view_perm_x(struct userec *user, struct boardmem *x);
int hideboard(char *bname);
int hideboard_x(struct boardmem *x);
int innd_board(char *bname);
int njuinn_board(char *bname);
int political_board(char *bname);
int anony_board(char *bname);
int noadm4political(char *bname);
int has_post_perm(struct userec *user, struct boardmem *x);
int has_vote_perm(struct userec *user, struct boardmem *x);
struct boardmem *numboard(const char *numstr);
struct boardmem *getbcache(const char *board);
int getbnumx(struct boardmem *x1);
int getbnum(char *board);
struct boardmem *getboard(char board[80]);
struct boardmem *getboard2(char board[80]);
int send_msg(char *myuserid, int i, char *touserid, int topid, char *msg,
	     int offline);
char *horoscope(int month, int day);
int modify_mode(struct user_info *x, int newmode);
int user_perm(struct userec *x, int level);
int count_online(void);
int loadfriend(char *id);
int cmpfuid(unsigned *a, unsigned *b);
int initfriends(struct user_info *u);
int isfriend(char *id);
int loadbad(char *id);
int isbad(char *id);
void changemode(int mode);
char *encode_url(unsigned char *s);
char *noquote_html(unsigned char *s);
char *flag_str_bm(int access, int has_read);
char *flag_str(int access);
char *flag_str2(int access, int has_read);
char *userid_str(char *s);
int fprintf2(FCGI_FILE * fp, char *s);
int set_my_cookie(void);
int cmpboard(struct boardmem **b1, struct boardmem **b2);
int cmpboardscore(struct boardmem **b1, struct boardmem **b2);
int cmpboardinboard(struct boardmem **b1, struct boardmem **b2);
int cmpuser(struct user_info **a, struct user_info **b);
struct fileheader *findbarticle(struct mmapfile *mf, char *file, int *num,
				int mode);
char *utf8_decode(char *src);
void fdisplay_attach(FCGI_FILE * output, FCGI_FILE * fp, char *currline,
		     char *nowfile);
void printhr(void);
void updatelastboard(void);
void updateinboard(struct boardmem *x);
int countln(char *fname);
double *system_load(void);
int setbmstatus(struct userec *u, int online);
int count_uindex(int uid);
int cachelevel(int filetime, int attached);
int dofilter(char *title, char *fn, int level);
int dofilter_edit(char *title, char *buf, int level);
int search_filter(char *pat1, char *pat2, char *pat3);
char *setbfile(char *buf, char *boardname, char *filename);
void ansi2ubb(const char *s, int size, FCGI_FILE * fp);
int ubb2ansi(const char *s, const char *fn);
void printubb(const char *form, const char *textarea);
int mmap_getline(char *ptr, int max_size);
char *makeurlbase(int uent, int uid);
void updatewwwindex(struct boardmem *x);
void changeContentbg(void);
void print_caution_box(int mode, char *msg);
inline void strnncpy2(char *s, int *l, const char *s2, int len);
int isSpider();
/* bbsupdatelastpost.c */
int updatelastpost(char *board);
/* boardrc.c */
void brc_expired(void);
void brc_update(char *userid);
struct brcinfo *brc_readinfo(char *userid);
void brc_saveinfo(char *userid, struct brcinfo *info);
int brc_initial(char *userid, char *boardname);
int brc_uentfinial(struct user_info *uinfo);
void brc_add_read(struct fileheader *fh);
void brc_add_readt(int t);
int brc_un_read(struct fileheader *fh);
void brc_clear(void);
int brc_un_read_time(int ftime);
int brc_board_read(char *board, int ftime);
/* deny_users.c */
void loaddenyuser(char *board);
void savedenyuser(char *board);
/* bbsred.c */
char *bbsred(char *command);
/* bbsmain.c */
void logtimeused(void);
void wantquit(int signal);
struct cgi_applet *get_cgi_applet(char *needcgi);
void get_att_server(void);
int main(int argc, char *argv[]);
/* bbstop10.c */
int bbstop10_main(void);
/* bbsdoc.c */
void printdocform(char *cginame, int bnum);
void noreadperm(char *board, char *cginame);
void nosuchboard(char *board, char *cginame);
void printhrwhite(void);
void printboardhot(struct boardmem *x);
void printrelatedboard(char *board);
void printboardtop(struct boardmem *x, int num, char *infostr);
int getdocstart(int total, int lines);
void bbsdoc_helper(char *cgistr, int start, int total, int lines);
int bbsdoc_main(void);
char *size_str(int size);
char *short_Ctime(time_t t);
void docinfostr(struct boardmem *brd, int mode, int haspermm);
int doc_print_title(struct boardmem *x, int highlight);
/* bbscon.c */
int showbinaryattach(char *filename);
char *binarylinkfile(char *f);
void fprintbinaryattachlink(FCGI_FILE * fp, int ano, char *attachname, int pos,
			    int size);
int ttid(int i);
int fshowcon(FCGI_FILE * output, char *filename, int show_iframe);
int mem2html(FCGI_FILE * output, char *str, int size, char *filename, int showhead, int show_iframe);
int showcon(char *filename);
int showconxml(char *filename, int viewertype);
int testmozilla(void);
int testxml(void);
int bbscon_main(void);
int bbscon_show_article_link(int board, char *file);
void processMath(void);
void con_print_main_right(struct boardmem *bx, struct fileheader *x);
/* bbsboa.c */
int bbsboa_main(void);
int showsecpage(const struct sectree *sec);
int showdefaultsecpage(const struct sectree *sec);
int showsechead(const struct sectree *sec);
int showstarline(char *str);
int showExLinks(const struct sectree *sec, int num);
int showSecTop10(const struct sectree *sec);
int showsecnav(const struct sectree *sec);
int genhotboard(struct hotboard *hb, const struct sectree *sec, int max);
int showhotboard(const struct sectree *sec, char *s);
int showfile(char *fn);
int showsecintro(const struct sectree *sec);
int showboardlistscript(const char *secstr);
int showboardlist(const char *secstr, int mode);
int showsecmanager(const struct sectree *sec);
int showSecTop10(const struct sectree *sec);
int showmyposts(char *id);
int showadv(char *parm);
/* index2009.c */
int index2009_main(void);
int showsecpage2009(const struct sectree *sec);
int showdefaultsecpage2009(const struct sectree *sec);
int showsechead2009(const struct sectree *sec);
int showstarline2009(char *str);
int showExLinks2009(const struct sectree *sec, int num);
int showSecTop102009(const struct sectree *sec);
int showsecnav2009(const struct sectree *sec);
int genhotboard2009(struct hotboard *hb, const struct sectree *sec, int max);
int showhotboard2009(const struct sectree *sec, char *s);
int showfile2009(char *fn);
int showsecintro2009(const struct sectree *sec);
int showboardlistscript2009(const char *secstr);
int showboardlist2009(const char *secstr, int mode);
int showsecmanager2009(const struct sectree *sec);
int showSecTop102009(const struct sectree *sec);
int showmyposts2009(char *id);
int showadv2009(char *parm);
/* bbsall.c */
int bbsall_main(void);
/* bbsanc.c */
int bbsanc_main(void);
/* bbs0an.c */
int anc_readtitle(FCGI_FILE * fp, char *title, int size);
int anc_readitem(FCGI_FILE * fp, char *path, int sizepath, char *name,
		 int sizename);
int anc_hidetitle(char *title);
int bbs0an_main(void);
int getvisit(int n[2], const char *path);
/* bbslogout.c */
int bbslogout_main(void);
/* bbsleft.c */
void printdiv(int *n, char *str);
void printsectree(const struct sectree *sec);
int bbsleft_main(void);
/* bbslogin.c */
int extandipmask(int ipmask, char *ip1, char *ip2);
int bbslogin_main(void);
int bbscms_main(void);
char *wwwlogin(struct userec *user, int ipmask, int ajax);
/* bbsbadlogins.c */
int bbsbadlogins_main(void);
/* bbsqry.c */
int bbsqry_main(void);
void show_special(char *id2);
int bm_printboard(struct boardmem *bmem, char *who);
int show_onlinestate(char *userid);
/* bbsnot.c */
int bbsnot_main(void);
/* bbsfind.c */
int bbsfind_main(void);
int search(char *id, char *pat, char *pat2, char *pat3, int dt);
/* bbsfadd.c */
int bbsfadd_main(void);
/* bbsfdel.c */
int bbsfdel_main(void);
/* bbsfall.c */
int bbsfall_main(void);
/* bbsusr.c */
int bbsusr_main(void);
/* bbsfriend.c */
int bbsfriend_main(void);
/* bbsfoot.c */
void showmyclass(void);
#ifdef ENABLE_BLOG
void showmyblog(void);
#endif
int bbsfoot_main(void);
int mails_time(char *id);
int mails(char *id, int *unread);
/* bbsform.c */
int bbsform_main(void);
void check_if_ok(void);
void check_submit_form(void);
/* bbspwd.c */
int bbspwd_main(void);
/* bbsplan.c */
int bbsplan_main(void);
int save_plan(char *plan);
/* bbsinfo.c */
int bbsinfo_main(void);
int check_info(void);
/* bbsmybrd.c */
int bbsmybrd_main(void);
int bbsmybrd_show_left(unsigned int mybrdmode);
/* bbsmypic.c */
int bbsmypic_main();
void printmypic(char *userid);
int printmypicbox(char *userid);
/* bbssig.c */
int bbssig_main(void);
void save_sig(char *path);
/* bbspst.c */
int bbspst_main(void);
int printquote(FILE *fp, char *filename, char *board, char *userid, int fullquote);
void printselsignature(void);
void printuploadattach(void);
void printusemath(int checked);
/* bbsgcon.c */
int bbsgcon_main(void);
/* bbsgdoc.c */
int bbsgdoc_main(void);
/* bbsdel.c */
int bbsdel_main(void);
/* bbsdelmail.c */
int bbsdelmail_main(void);
/* bbsmailcon.c */
int bbsmailcon_main(void);
/* bbsmail.c */
int bbsmail_main(void);
/* bbsdelmsg.c */
int bbsdelmsg_main(void);
/* bbssnd.c */
int testmath(char *ptr);
int bbssnd_main(void);
/* bbsnotepad.c */
int bbsnotepad_main(void);
/* bbsmsg.c */
int bbsmsg_main(void);
/* bbssendmsg.c */
int bbssendmsg_main(void);
int checkmsgbuf(char *msg);
/* bbsreg.c */
int bbsreg_main(void);
/* bbsemailreg.c */
int bbsemailreg_main(void);
/* bbsemailconfirm.c */
int doConfirm(struct userec *urec, char *email, int invited);
int bbsemailconfirm_main(void);
/* bbsinvite.c */
int bbsinvite_main(void);
/* bbsinviteconfirm.c */
int bbsinviteconfirm_main(void);
/* bbsmailmsg.c */
int bbsmailmsg_main(void);
void mail_msg(struct userec *user);
/* bbssndmail.c */
int bbssndmail_main(void);
/* bbsnewmail.c */
int bbsnewmail_main(void);
/* bbspstmail.c */
int bbspstmail_main(void);
/* bbsgetmsg.c */
int print_emote_table(char *form, char *input);
int emotion_print(char *msg);
int bbsgetmsg_main(void);
void check_msg(void);
/* bbscloak.c */
int bbscloak_main(void);
/* bbsmdoc.c */
int bbsmdoc_main(void);
/* bbsnick.c */
int bbsnick_main(void);
/* bbstfind.c */
int bbstfind_main(void);
/* bbsadl.c */
int bbsadl_main(void);
/* bbstcon.c */
int bbstcon_main(void);
int bbstopic_main(void);
int fshow_file(FCGI_FILE * output, char *board, struct fileheader *x);
int show_file(struct boardmem *brd, struct fileheader *x, int num, int st);
/* bbstdoc.c */
int bbstdoc_main(void);
/* bbsdoreg.c */
int bbsdoreg_main(void);
int badstr(unsigned char *s);
void newcomer(struct userec *x, char *words);
/* bbsmywww.c */
int bbsmywww_main(void);
/* bbsccc.c */
int bbsccc_main(void);
int do_ccc(struct fileheader *x, struct boardmem *brd1, struct boardmem *brd);
/* bbsclear.c */
int bbsclear_main(void);
/* bbsstat.c */
int search_stat(void *ptr, int size, int key);
int bbsstat_main(void);
/* bbsedit.c */
int bbsedit_main(void);
int Origin2(char text[256]);
int update_form(char *board, char *file, char *title);
int getpathsize(char *path);
/* bbsman.c */
int bbsman_main(void);
int do_del(char *board, char *file);
int do_set(char *dirptr, int size, char *file, int flag, char *board);
int do_manager(int mode, char *board, char *filename);
/* bbsparm.c */
int bbsparm_main(void);
int read_form(void);
/* bbsfwd.c */
int bbsfwd_main(void);
int do_fwd(struct fileheader *x, char *board, char *target);
/* bbsmnote.c */
int bbsmnote_main(void);
void save_note(char *path);
/* bbsdenyall.c */
int bbsdenyall_main(void);
/* bbsdenydel.c */
int bbsdenydel_main(void);
/* bbsdenyadd.c */
int bbsdenyadd_main(void);
/* bbstopb10.c */
int bbstopb10_main(void);
/* bbsbfind.c */
int bbsbfind_main(void);
/* bbsx.c */
int bbsx_main(void);
/* bbseva.c */
int bbseva_main(void);
/* bbsvote.c */
int bbsvote_main(void);
int addtofile(char filename[80], char str[256]);
int valid_voter(char *board, char *name);
/* bbsshownav.c */
int bbsshownav_main(void);
int shownavpart(int mode, const char *secstr);
void shownavpartline(char *buf, int mode);
/* bbsbkndoc.c */
int bbsbkndoc_main(void);
/* bbsbknsel.c */
int bbsbknsel_main(void);
/* bbsbkncon.c */
int bbsbkncon_main(void);
/* bbshome.c */
char *userid_str2(char *s);
int bbshome_main(void);
/* bbsindex.c */
int checkfile(char *fn, int maxsz);
int loadoneface(void);
int showannounce(void);
void loginwindow(void);
int setlastip(void);
void shownologin(void);
void checklanguage(void);
int bbsindex_main(void);
/* bbssechand.c */
void short_stamp(char *str, time_t * chrono);
int showheader(char *grp);
void showwelcome(void);
void showgroup(char *grp);
void showitem(char *grp, char *item);
void postnewslot(char *grp);
int savenewslot(char *grp);
int replymail(char *grp, char *item);
int bbssechand_main(void);
/* bbslform.c */
int bbslform_main(void);
/* regreq.c */
int regreq_main(void);
/* bbsselstyle.c */
int bbsselstyle_main(void);
/* bbscon1.c */
int bbscon1_main(void);
/* bbsattach.c */
int bbsattach_main(void);
/* bbskick.c */
int bbskick_main(void);
/* bbsrss.c */
int bbsrss_main(void);
/* bbsscanreg.c */
int bbsscanreg_main(void);
/* bbsshowfile.c */
int bbsshowfile_main(void);
/* bbsdt.c */
int bbsdt_main(void);
/* bbslt.c */
void print_radio(char *cname, char *name, char *str[], int len, int select);
int bbslt_main(void);
/* bbsincon.c */
int bbsincon_main(void);
/* bbssetscript.c */
int bbssetscript_main(void);
/* bbscccmail.c */
int bbscccmail_main(void);
int do_cccmail(struct fileheader *x, struct boardmem *brd);
/* bbsfwdmail.c */
int bbsfwdmail_main(void);
int do_fwdmail(char *fn, struct fileheader *x, char *target);
/* bbsscanreg_findsurname.c */
int bbsscanreg_findsurname_main(void);
/* bbsnt.c */
int bbsnt_main(void);
/* bbstopcon.c */
int bbstopcon_main(void);
/* bbsdrawscore.c */
int bbsdrawscore_main(void);
int printparam(int n, char *board);
/* bbsmyclass.c */
void showmyclasssetting(void);
void savemyclass(void);
int bbsmyclass_main(void);
/* bbssearchboard.c */
int bbssearchboard_main(void);
/* bbslastip.c */
int bbslastip_main(void);
/* bbsucss.c */
int bbsucss_main(void);
/* bbsdefcss.c */
int bbsdefcss_main(void);
/* bbsself_photo_vote.c */
int bbsself_photo_vote_main(void);
/* bbsspam.c */
int bbsspam_main(void);
/* bbsspamcon.c */
int bbsspamcon_main(void);
/* bbssouke.c */
int bbssouke_main(void);
void printSoukeForm(void);
/* bbsboardlistscript.c */
int bbsboardlistscript_main(void);
int listmybrd(struct boardmem *(data[])); 
int makeboardlist(const struct sectree *sec, struct boardmem *(data[]));
int boardlistscript(struct boardmem *(data[]), int total);
int oneboardscript(struct boardmem *brd);
/* bbsolympic.c */
int bbsolympic_main(void);
int showolympic(void);
int showlastanc(char *buf);
int showfirstanc(char *buf);
int showoneboardscript(char *aboard);
int shownewpost(char *board);
int shownewmark(char *board);
/* bbsicon.c */
int bbsicon_main(void);
/* bbsdlprepare.c */
int bbsdlprepare_main(void);
/* bbsnewattach.c */
int bbsnewattach_main(void);
/* bbsupload.c */
int save_attach(char *path, int totalsize);
int fixfilename(char *str);
void printuploadform(void);
int bbsupload_main(void);
/* bbspsttmpl.c */
int bbspsttmpl_main(void);
/* bbspassport.c */
int bbspassport_main(void);
/* bbslpassport.c */
int bbslpassport_main(void);
char *des3_encode(char *, int);
char *des3_decode(char *, int);
int load_all_key(void);
/* bbsshowdigest.c */
int bbsshowdigest_main();
int showsecdigest(const struct sectree *sec, char *s, int *init_digest);
void digestadjust(char *idname);
/* bbstshirt.c */
int bbstshirt_main(void);
/* bbsspecial.c */
int showspecialall(const struct sectree *sec);
int bbsspecial_main(void);
#ifdef ENABLE_BLOG
/* blogblog.c */
void printXBlogHeader(void);
void printXBlogEnd(void);
void printBlogHeader(struct Blog *blog);
void printBlogSideBox(struct Blog *blog);
void printBlogFooter(struct Blog *blog);
int printAbstract(struct Blog *blog, int n);
void printAbstractsTime(struct Blog *blog, int start, int end);
void printPages(char *link, int count, int currPage);
void printAbstractsSubject(struct Blog *blog, int subject, int page);
int tagedAs(struct BlogHeader *blh, int tag);
void printAbstractsTag(struct Blog *blog, int tag, int page);
void printAbstractsAll(struct Blog *blog, int page);
void setBlogBgImg(struct Blog *blog);
void noBlog(void);
void noAccess(char *item, char *next);
int blogblog_main(void);
/* blogread.c */
int printPrevNext(struct Blog *blog, int n);
int printBlogArticle(struct Blog *blog, int n);
void printCommentFile(struct Blog *blog, int fileTime, int n, int commentTime, int i, int blocked);
void printComments(struct Blog *blog, int n, int fileTime);
void printCommentBox(struct Blog *blog, int fileTime);
void printTimeLogo(time_t filetime);
int blogShowArticle(char *userid, time_t filetime);
int blogread_main(void);
/* blogpost.c */
void printUploadUTF8(void);
void printSelectSubject(struct Blog *blog, struct BlogHeader *blh);
void printSelectTag(struct Blog *blog, struct BlogHeader *blh);
void printTextarea(char *text, int size);
int blogpost_main(void);
/* blogsend.c */
int saveTidyHtml(char *filename, char *content, int filterlevel);
char *readFile(char *filename);
int blogsend_main(void);
/* blogeditabstract.c */
void printBlogEditPostSideBox(struct Blog *blog, time_t fileTime);
void printAbstractBox(struct Blog *blog, int n, int mode);
int doSaveAbstract(struct Blog *blog, time_t fileTime, char *abstract);
int blogeditabstract_main(void);
/* blogeditconfig.c */
void modifyConfig(struct Blog *blog, int echo);
void printBlogSettingSideBox(struct Blog *blog);
int blogeditconfig_main(void);
/* blogeditsubject.c */
int blogeditsubject_main(void);
/* blogeditlink.c */
int blogeditlink_main(void);
/* blogedittag.c */
int blogedittag_main(void);
/* blogeditfriend.c */
int blogeditfriend_main(void);
/* blogdraft.c */
int printDraftAbstract(struct BlogHeader *blh);
int blogdraft_main(void);
/* blogdraftread.c */
int printDraftArticle(struct Blog *blog, int n);
int blogdraftread_main(void);
/* blogcomment.c */
int blogcomment_main(void);
void printNewComment(struct Blog *blog);
/* blogsetup.c */
void printLoginFormUTF8(void);
int blogsetup_main(void);
void printBlogSetupForm(struct Blog *blog);
/* blogpage.c */
int blogpage_main(void);
/* blogrss2.c */
char *gmtctime(const time_t *timep);
int blogrss2_main(void);
/* blogatom.c */
char *gmtctime(const time_t *timep);
int blogatom_main(void);
/* blogmedia.c */
char* getMediaUrl(struct Blog *blog, char *src, int len, int type);
int blogmedia_main(void);
/* blogdelete.c */
int blogdelete_main(void);
/* blogtemplate.c */
void printXBlogModel(struct Blog *blog);
int blogtemplate_main(void);
/* blogblock */
int blogblock_main(void);
void printBlockedMsg(char *userid, int fileTime);
/* blogpic */
int blogpic_main(void);
#endif
/* bbsseo.c */
int bbsseo_main(void);
/* bbsajaxtest.c */
int bbsajaxtest_main(void);
/* bbsadv.c */
int bbsadv_main(void);
int bbsadv_show(int pos);
int bbsadv_cheat(int pos);
/* bbsfootball.c */
int bbsfootball_main(void);
/* bbsbug.c */
int bbsbug_main(void);
/* bbstestssl.c */
int bbstestssl_main(void);
/* bbsregcheck.c */
int bbsregcheck_main(void);
