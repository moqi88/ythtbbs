#include "bbslib.h"
extern char *cginame;

static const int img_fmt_png[148] = {
	0x474e5089, 0x0a1a0a0d, 0x0d000000, 0x52444849, 0xc8000000, 0x14000000,
	0x00000408, 0xbaad6200, 0x02000041, 0x41444917, 0xedda7854, 0xc392d958,
	0x3b430c20, 0xd65ffffd, 0xf821353e, 0xa7da5392, 0x19db33b0, 0x2d921cc0,
	0xb4f18c9b, 0x676d3da7, 0xc9946363, 0x45d83f38, 0x96d7739f, 0xbfef0ac2,
	0x497b67ad, 0x0cdc208b, 0x021c205f, 0x7ee7284c, 0xd1b119db, 0x1213e009,
	0x5c01935d, 0x201d0e16, 0xae76eb4e, 0xb089cb86, 0x77345c05, 0x35011fbf,
	0xd5210bbe, 0x59e86961, 0xe2a13a41, 0x0d938332, 0x4361c392, 0x8dc0f524,
	0x81a72f5c, 0x9905625b, 0xe24cd564, 0x66c73be8, 0xb02d9d6f, 0x4c76cfd8,
	0x53a58724, 0x7a74a360, 0xc6e98c67, 0x57706756, 0xa447b979, 0x05cad381,
	0x5c8ecb43, 0x5aa18031, 0x08f33787, 0xb4d1e21d, 0xd30f6ef4, 0xa0ec50c3,
	0x0cd73522, 0xe199034c, 0x906a6905, 0xe0c2256b, 0xa5a7b67f, 0x7463a992,
	0x79852b12, 0x47354daa, 0xea4dd7c1, 0x213d166f, 0x55848980, 0x980ae842,
	0xadbffda2, 0xc0313a98, 0x62548f25, 0x8d404519, 0x67592baf, 0x152165d2,
	0x8b519cd7, 0x29d8530e, 0x4735fc85, 0x217c2ced, 0xbccd0db8, 0xf8c7b43b,
	0xcaac842e, 0xb5cb838c, 0xf960c25e, 0x5712e062, 0xb44f2b6f, 0xad820120,
	0xbb56aee5, 0xe100155d, 0x9877d053, 0xf0dbe321, 0x783a2e50, 0xbd7832fe,
	0xd1a042aa, 0x63f08f19, 0x78d6583a, 0x5ce2470c, 0xdc15eca1, 0xcc0a4fe2,
	0x625c6e13, 0xdf659703, 0x38fc43ac, 0x3c4e7481, 0x5315248a, 0x207e2594,
	0x9b3e2c68, 0xa294e728, 0xfcd5c19c, 0x83b53a7e, 0x6516008d, 0x74d2a527,
	0xeb7a7b38, 0xa642f853, 0x98c53c79, 0xa834ab62, 0x64220df1, 0xe76211a4,
	0x55d15641, 0x8d3c6199, 0xa4c4d252, 0x0d3abdc2, 0xeba43647, 0xa6e83cc7,
	0x1cf22f9c, 0x5f29aad4, 0xf947850d, 0xe129cdbe, 0xe6286624, 0x5b2e898c,
	0xa214c1c1, 0x229123ca, 0x0315a361, 0x0deb0ef9, 0x1b19f55a, 0x8a5ceafd,
	0xd4ede957, 0xaf77f55b, 0x4d82dc95, 0xb4f69fdc, 0x0ff699a7, 0x9062fe18,
	0x656d662d, 0x00000000, 0x444e4549, 0x826042ae
};

static char baseurl[STRLEN];

void
resizecachename(char *buf, size_t len, char *fname, int ver, int pos)
{
	char *ptr;
	if (ver > 0)
		snprintf(buf, len, "szmcache/%s%d.%d", fname, pos, ver);
	else
		snprintf(buf, len, "szmcache/%s%d", fname, pos);
	ptr = strchr(buf, '/') + 1;
	while ((ptr = strchr(ptr, '/'))) {
		*ptr = '_';
	}
}

int
showresizecache(char *resizefn)
{
      struct mmapfile mf = { ptr:NULL };
	MMAP_TRY {
		if (mmapfile(resizefn, &mf) < 0) {
			MMAP_UNTRY;
			MMAP_RETURN(-1);
		}
		printf("Content-type: image/jpeg\r\n");
		printf("Content-Length: %d\r\n\r\n", mf.size);
		fwrite(mf.ptr, 1, mf.size, stdout);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

int
showbinaryattach(char *filename)
{
	char *attachname;
	int pos;
	int small;
	int ver = 0;
	int fd;
	unsigned int size;
	struct mmapfile mf = { ptr:NULL };
	char resizefn[256];

//       no_outcache();
	if (cache_header(file_time(filename), 864000))
		return 0;
	attachname = getparm("attachname");
	pos = atoi(getparm("attachpos"));
	small = atoi(getparm("S"));
	ver = atoi(getparm("T"));
	if (small) {
		resizecachename(resizefn, sizeof (resizefn), filename, ver,
				pos);
		if (showresizecache(resizefn) == 0) {
			return 0;
		}
	}
	MMAP_TRY {
		if (mmapfile(filename, &mf) < 0) {
			MMAP_UNTRY;
			http_fatal("无法打开附件 1");
			MMAP_RETURN(-1);
		}
		if (pos >= mf.size - 4 || pos < 1) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("无法打开附件 2");
			MMAP_RETURN(-1);
		}
		if (mf.ptr[pos - 1] != 0) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("无法打开附件 3");
			MMAP_RETURN(-1);
		}
		size = ntohl(*(unsigned int *) (mf.ptr + pos));
		if (pos + 4 + size > mf.size) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("无法打开附件 4");
			MMAP_RETURN(-1);
		}
		while (small) {
			int ret, t_size;
			void *out;
			if (memcmp(mf.ptr + pos + 4, "BM", 2))
				ret =
				    szm_box_jpg(tjpg_ctx, mf.ptr + pos + 4,
						size, 480, &out, &t_size);
			else
				ret =
				    szm_box_bmp(tjpg_ctx, mf.ptr + pos + 4,
						size, 480, &out, &t_size);
			switch (ret) {
			case -SZM_ERR_CONNECT:
			case -SZM_ERR_BROKEN:
				printf("Status: 404\r\n\r\n");
				break;
			case 0:
				printf("Content-type: image/jpeg\r\n");
				printf("Content-Length: %d\r\n\r\n", t_size);
				fwrite(out, 1, t_size, stdout);
				fd = open(resizefn, O_WRONLY | O_CREAT, 0600);
				if (fd >= 0) {
					write(fd, out, t_size);
					close(fd);
				}
				free(out);
				break;
			default:
				printf("Content-type: image/png\r\n\r\n");
				fwrite((void *) img_fmt_png, 1,
				       sizeof (img_fmt_png), stdout);
				fd = open(resizefn, O_WRONLY | O_CREAT, 0600);
				if (fd >= 0) {
					write(fd, img_fmt_png,
					      sizeof (img_fmt_png));
					close(fd);
				}
				break;
			}
			break;
		}
		if (!small) {
			printf("Content-type: %s\r\n",
			       get_mime_type(attachname));
			printf("Content-Length: %d\r\n\r\n", size);
			fwrite(mf.ptr + pos + 4, 1, size, stdout);
		}
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

char *
binarylinkfile(char *f)
{
	static char *lf = "";
	if (f)
		lf = f;
	return lf;
}

void
fprintbinaryattachlink(FILE * fp, int ano, char *attachname, int pos, int size)
{
	char *ext, link[256], slink[256], *ptr, board[40];
	int pic = 0;
	int atthttp = 0;
	struct boardmem *brd;
	int resize = 0;

	void1(attachname);
	//check read_perm for guest, refer to has_read_perm()
	getparmboard(board, sizeof (board));
	brd = getboard(board);
	if (brd && !(brd->header.flag & CLOSECLUB_FLAG) && !brd->header.level) {
		ptr = "/" BASESMAGIC "/";
		if (w_info->att_mode == 0 && !via_proxy)
			atthttp = 1;
	} else {
		ptr = "";
	}
	if (!wwwcache->accel_addr.s_addr || !wwwcache->accel_port)
		atthttp = 0;

	if ((ext = strrchr(attachname, '.')) != NULL) {
		if (!strcasecmp(ext, ".bmp") || !strcasecmp(ext, ".jpg")
		    || !strcasecmp(ext, ".gif") || !strcasecmp(ext, ".jpeg")
		    || !strcasecmp(ext, ".png") || !strcasecmp(ext, ".pcx"))
			pic = 1;
		else if (!strcasecmp(ext, ".swf"))
			pic = 2;
		else if (!strcasecmp(ext, ".tiff") || !strcasecmp(ext, ".tif"))
			pic = 3;
		else if (!strcasecmp(ext, ".wmv"))
			pic = 4;
		else
			pic = 0;
	}

	if (tjpg_ctx && 1 == pic
	    && (!strcasecmp(ext, ".bmp") || !strcasecmp(ext, ".jpg")
		|| !strcasecmp(ext, ".jpeg"))
	    && size > 80000)
		resize = 1;

	if (!strcmp(cginame, "bbscon") || !strcmp(cginame, "bbsnt")
	    || !strcmp(cginame, "bbstcon")) {
		//同上
		if (!atthttp) {
			if (!*ptr)
				resize = 0;
#if 0
			//use bbsnewattach
			snprintf(link, sizeof (link), "%sb/%d/%s/%d/%s", ptr,
				 getbnumx(brd), binarylinkfile(NULL), pos,
				 attachname);
#else
			//Or use bbsattach, if want to resize pictures
			snprintf(link, sizeof (link),
				 "%sattach/bbscon/%s?B=%d&amp;F=%s&amp;attachpos=%d&amp;attachname=/%s",
				 ptr, attachname, getbnumx(brd),
				 binarylinkfile(NULL), pos, attachname);
#endif
		} else if (wwwcache->accel_addr.s_addr && wwwcache->accel_port) {
			snprintf(link, sizeof (link),
				 "http://%s:%d%sattach/bbscon/%s?B=%d&amp;F=%s&amp;attachpos=%d&amp;attachname=/%s",
				 MY_BBS_DOMAIN,
				 wwwcache->accel_port,
				 ptr, attachname, getbnumx(brd),
				 binarylinkfile(NULL), pos, attachname);
			//} else {
			//      snprintf(link, sizeof (link),
			//               "http://%s:8080/%s/%s/%d/%s", MY_BBS_IP,
			//               board, binarylinkfile(), pos, attachname);
			//      resize = 0;
		}
	} else {
		snprintf(link, sizeof (link),
			 "attach/%s/%s?%s&amp;attachpos=%d&amp;attachname=/%s",
			 cginame, attachname, getsenv("QUERY_STRING"), pos,
			 attachname);
		resize = 0;
	}

	while (resize) {
		char *t1, *t2;
		if (wwwcache->text_accel_addr.s_addr
		    && wwwcache->text_accel_port) {
			t1 = strstr(link, BASESMAGIC);
			if (!t1)
				break;
			snprintf(slink, sizeof (slink), "http://%s:%d/%s",
				 MY_BBS_DOMAIN,
				 wwwcache->text_accel_port, t1);
		} else
			strcpy(slink, link);
		t1 = strchr(slink, '?');
		t2 = strchr(link, '?');
		if (!t1 || !t2)
			break;
		sprintf(t1 + 1, "S=1&amp;%s", t2 + 1);
		break;
	}

	if (1 == pic && !resize)
		strcpy(slink, link);

	switch (pic) {
	case 1:
		fprintf(fp, "%d 附图: %s (%s%d 字节%s)<br>"
			"<a href='%s' target='_blank'>"
			"<img src='%s' border='0' alt='按此在新窗口浏览图片' "
			"onload='con_resize(this);' /></a>", ano, attachname,
			resize ? "原始图大小 " : "", size,
			resize ? "，点击看大图" : "", link, slink);
		break;
	case 2:
		fprintf(fp,
			"%d Flash动画: "
			"<a href='%s'>%s</a> (%d 字节)<br>"
			"<OBJECT width=560 height=480><PARAM NAME='MOVIE' VALUE='%s'>"
			"<EMBED width=560 height=480 SRC='%s'></EMBED></OBJECT>",
			ano, link, attachname, size, link, link);
		break;
	case 3:
		fprintf(fp, "%d 附图: <a href='%s'>%s</a> (%d 字节)<br>"
			"<object width=560 height=480 classid='CLSID:106E49CF-797A-11D2-81A2-00E02C015623'>"
			"<param name='src' value='%s'>"
			"<param name='negative' value='yes'>"
			"<embed src='%s' type='image/tiff' negative='yes' width=560 height=480>"
			"</object>", ano, link, attachname, size, link, link);
		break;
	case 4:
		fprintf(fp,
			"%d 视频: "
			"<a href='%s'>%s</a> (%d 字节)<br>"
			"<embed width=560 height=480 src='%s%s'></embed>",
			ano, link, attachname, size, strncmp(link, "http://",
							     7) ? "http://"
			MY_BBS_DOMAIN "/" : "", link);
		break;

	default:
		fprintf(fp,
			"%d 附件: <a href='%s'>%s</a> (%d 字节)",
			ano, link, attachname, size);
	}
}

int
ttid(int i)
{
	static int n = 0;
	n += i;
	return n % 2000 + now_t;
}

/* show_iframe: 0  Show "<div id=tt*></div>, and show the content as script.
		1  Show "<div id=tt*></div>, and the link to the script
		2  only show the content of the file as script
		3  show the content as pure html
		4  show the seo content to the search engine
 */
// the show_iframe has been changed because of SEO code optimization

int
fshowcon(FILE * output, char *filename, int show_iframe)
{
      struct mmapfile mf = { ptr:NULL };
#if 0
      int ftime = 0;
      char *ptr;

      ptr = strchr(filename, '/');
      while (ptr) {
	      if (strlen(ptr) > 3) {
		      if (!strncmp(ptr, "/M.", 3)) {
			      ftime = atoi(ptr + 3);
			      break;
		      } else 
			      ptr++;
	      } else {
		      break;
	      }
	      ptr = strchr(ptr, '/');
      }
#endif      
//	  fprintf(output, "<!-- show_iframe= %d -->\n", show_iframe);
      if (show_iframe != 2) {
		fprintf(output, "<div id=\"tt%d\" class=\"con_content_%d\"></div> ", 
				ttid(1), contentbg);
		if (show_iframe == 1) {
			char interurl[256];
			if (via_proxy)
				snprintf(interurl, sizeof (interurl),
					 "/" SMAGIC "/%s+%s1%s", filename,
					 getparm("T"), usingMath ? "m" : "");
			else
				snprintf(interurl, sizeof (interurl),
					 "http://%s:%d/" SMAGIC "/%s+%s1%s",
					 MY_BBS_DOMAIN,
					 wwwcache->text_accel_port, filename,
					 getparm("T"), usingMath ? "m" : "");

			fprintf(output,
				"<script charset=" CHARSET
				" src=\"%s\"></script>", interurl);
			fprintf(output,
				"<script>document.getElementById('tt%d')"
				".innerHTML=docStr;</script>\n</td></tr>"
				"</table>\n",
				ttid(0));
			return 0;
		}
		if (show_iframe != 3 && show_iframe != 4)
// It is forbidden to use script in the content, for they are not friendly to SEO.
#if 0
			fputs("<script>var docStr = \"", output);
#endif
			fputs("",output);
	}
	if (show_iframe != 3 && show_iframe != 4)
		quote_quote = 1;
	if (mmapfile(filename, &mf) < 0)
		goto END;
	MMAP_TRY {
		if (show_iframe == 4)
			mem2html(output, mf.ptr, mf.size, filename, NA, show_iframe);
		else
			mem2html(output, mf.ptr, mf.size, filename, YEA, show_iframe);
	}
	MMAP_CATCH {
	}
	MMAP_END mmapfile(NULL, &mf);
	quote_quote = 0;
      END:
	if (show_iframe != 2 && show_iframe != 3 && show_iframe != 4) {
#if 0
		fprintf(output, "\";\n");
#endif
		fprintf(output, "\n");
#if 0
		fprintf(output, "docStr=lowerTag(docStr);"
			"docStr=docStr.replace(/<\\/script[^>]*>/g, '<\\/script>\\n');"
			"docStr=docStr.replace(/<script[^>]*>[^\\n]*<\\/script>\\n/g, '');\n");
#endif
#if 0		
		fprintf(output,
			"document.getElementById('tt%d').innerHTML"
			" = docStr;\n" 
			"con_hide_load(%d, 0);</script>\n",
			ttid(0), ftime);
#endif
//script is not good for SEO
#if 0
		if (show_iframe != 3 && show_iframe != 4)
		fprintf(output,
			"document.getElementById('tt%d').innerHTML=docStr;"
			"</script>\n", 
			ttid(0));
#endif		
		fprintf(output,	"\n", ttid(0));
	}
	return 0;
}

//iz 
//to drop article's head
char * DropArtiHead(char * source){
	char *buf;
	
	if(NULL==(buf=strstr(source,"发信站")))
		return source;
	
	else 
		
		if(NULL==(buf=strchr(buf,'\n')))
			return source;
	
		else 
			return buf;
}

/*
// multiple definition, same function as in ythtlib.h
// delete redundant words for search engine
void 
delete_redundant_word( char *str, char const *sub_str)
{
	char tmpResult[60];
	char * pTmpResult;
	char * s;
	char * t;
	int hasChange;

	memset(tmpResult,0x00,60);
	pTmpResult=tmpResult;

	t=s=str;
	hasChange=0;

	while (NULL!=(s=strstr(t,sub_str)))
	    {
	    memcpy(pTmpResult, t, s-t);
	    pTmpResult+=s-t;
	    t=s+strlen(sub_str);
	    hasChange=1;
	    }

	memcpy(pTmpResult, t, strlen(t));

	if (hasChange)
	        {
	        memset(str, 0x00, strlen(str));
	        memcpy(str, tmpResult, strlen(tmpResult));
	        memset(tmpResult, 0x00, strlen(tmpResult));
	        }
}
*/

int
mem2html(FILE * output, char *str, int size, char *filename, int showhead, int show_iframe)
{
	char *str0, *ptr, buf[512];
	int ano = 0, len, size0, quoteid = 0;
#if 0		
	int percent = 0;
#endif	
	int fhdiv = 0, quotediv = 0, qmddiv = 0, ftime = 0, quotenow = 0, i;
	int isarticle = NA;
	
	ptr = strchr(filename, '/');
	while (ptr) {
		if (strlen(ptr) > 3) {
			if (!strncmp(ptr, "/M.", 3)) {
				ftime = atoi(ptr + 3);
				break;
			} else
				ptr++;
		} else 
			break;
		ptr = strchr(ptr, '/');
	}
	
	if(!showhead)
		str=DropArtiHead(str);
	
	str0 = str;
	size0 = size;
	underline = 0;
	highlight = 0;
	lastcolor = 37;
	useubb = 1;
	fdisplay_attach(NULL, NULL, NULL, NULL);
	while (size) {

		len = min(size, sizeof (buf) - 1);
		ptr = str;
		str = memchr(ptr, '\n', len);
		if (!str && len == sizeof (buf) - 1) {
			str = memchr(ptr + 1, '\033', len);
			if (str)
				str--;
		}
		if (!str)
			str = ptr + len;
		else {
			str++;
			len = str - ptr;
		}
		size -= len;
		if (len > 2)
			quotenow = 0;

		if (fhdiv == 0 && len > 8 && !strncmp(ptr, "发信人: ", 8)) {
				fprintf(output, "<div id=\"con_%d_contenet_title" "\"	class=\"con_content_title_%d\"> ", 
				ftime, contentbg,fhdiv);
			fhdiv++;
			isarticle=YEA;  // this is for article
		}

		if (fhdiv == 2) {
				fprintf(output, "</div>",fhdiv);
			fhdiv++;
		}

		if (fhdiv == 1 && len > 8 && !strncmp(ptr, "发信站: ", 8))
			fhdiv++;
		if (len > 10 && !strncmp(ptr, "begin 644 ", 10) && filename) {
			FILE *fp;
			int pos;
			strsncpy(buf, ptr, len);
			fp = fopen(filename, "r");
			if (fp) {
				fseek(fp, str - str0, SEEK_SET);
				ano++;
				ptr = strrchr(filename, '/');
				errlog("old attach %s", filename);
				fdisplay_attach(output, fp, buf, ptr + 1);
				pos = ftell(fp);
				str = str0 + pos;
				size = size0 - pos;
				fclose(fp);
			}
			if(showhead == YEA || fhdiv> 2)  //print the contents 
				fprintf(output, "\n<br>",fhdiv);
			continue;
		}
		
		for(i = 0; !qmddiv; i += 2) {
			if (len > 2 && !strncmp(ptr + i, ": ", 2))
				quotenow++;
			else if (len > 6 && !strncmp(ptr + i, "【 在 ", 6)) {
				quotenow++;
				break;
			} else
				break;
		}
		
		while(quotediv < quotenow) {
			if (quotediv == 0) {
				fprintf(output, "<div id=\""
					"con_%d_content_quote_%d\">", 
					ftime, quoteid);
#if 0
				fprintf(output, "\";\n"
					"quotelist[quotelist.length] = '"
					"con_%d_content_quote_%d';\n"
					"docStr += \"", 
					ftime, quoteid);
#endif
				quoteid++;
			}
			fprintf(output, "<div id=\""
				"con_%d_content_quote_%d_box\" "
				"class=\"con_content_quote_%d\">"
				"<div id=\"con_%d_content_quote_%d_padding"
				"\" class=\"con_content_quote_padding_%d"
				"\">", 
				ftime, quoteid - 1, contentbg, 
				ftime, quoteid - 1, contentbg);
			quotediv++;
		}
		while (quotediv > quotenow) {
			fprintf(output, "</div></div>");
			quotediv--;
			if (quotediv == 0)
				fprintf(output, "</div>");
		}
		
		if (len > 18 && !strncmp(ptr, "beginbinaryattach ", 18)
		    && size >= 5 && !*str) {
			unsigned int attlen;
			strsncpy(buf, ptr, len);
			ptr = strchr(buf, '\r');
			if (ptr)
				*ptr = 0;
			ptr = strchr(buf, '\n');
			if (ptr)
				*ptr = 0;
			ano++;
			ptr = buf + 18;
			attlen = ntohl(*(unsigned int *) (str + 1));
			fprintbinaryattachlink(output, ano, ptr,
					       str - str0 + 1, attlen);
			attlen = min(size, (int) (attlen + 5));
			str += attlen;
			size -= attlen;
			continue;
		}

		if (len == 3 && !strncmp(ptr, "--\n", 3) && !qmddiv) {
			while(quotediv > 0) {
				fprintf(output, "</div>");
				quotediv--;
				if (quotediv == 0)
					fprintf(output, "</div>");
			}
			fprintf(output, "<div id=\"con_%d_content_qmd\" "
				"class=\"con_content_qmd_%d\">", 
				ftime, contentbg);
			qmddiv++;
		}
		//不打印标题三行
		if(fhdiv > 2 || !isarticle || show_iframe==2)  //!isarticle: 不是文章页，例如精华或信件    show_iframe==2: 同主题阅读
			fhhprintf(output, "%.*s", len, ptr);
#if 0
		if ((size0 - size) * 100 /size0  - percent >= 5) {
			fprintf(output, "\";\ncon_show_load(%d, %d, %d);"
				"\ndocStr += \"", 
				ftime, size0 - size, size0);
			percent = (size0 - size) * 100 /size0;	
		}
#endif		
	}
	if (qmddiv)
		fprintf(output, "</div>");
	return 0;
}

void con_print_reply_box(struct fileheader *x) {
	printf("<div id=\"con_%d_reply\" class=\"con_reply_%d\" "
		"style=\"display: none;\">\n"
		"</div>\n", 
		x->filetime, contentbg);
}

int
showcon(char *filename)
{
	int retv;
	printf("<div class=line></div>");
	retv = fshowcon(stdout, filename, 0);
	printf("<div class=line></div>");
	return retv;
}

int
testmozilla()
{
	char *ptr = getsenv("HTTP_USER_AGENT");
	if (strcasestr(ptr, "Mozilla") && !strcasestr(ptr, "compatible"))
		return 1;
	return 0;
}

void
processMath()
{
	if (usedMath) {
		printf("<script src=/jsMath/jsMath.js></script>");
		printf("<script>jsMath.ProcessBeforeShowing();</script>");
	}
}

void
con_print_main_right(struct boardmem *bx, struct fileheader *x) {
	struct boardaux *aux = getboardaux(getbnumx(bx));
	const struct sectree *sec = getsectree("");
	char title[40];
	int i;
	
	printf("<div id=\"con_main_right\">\n");
	if ((loginok && !isguest) && (x->accessed & FH_ATTACHED || 
			(((!via_proxy && wwwcache->accel_addr.s_addr && 
			wwwcache->accel_port) || via_proxy) && 
			!w_info->doc_mode)))
		printf("<div class=\"annbox\">\n"
			"<h2>特殊模式</h2>\n"
			"<li><a href='%s/bbsmywww'>看不了文章？看不了图片？</a></li>\n"
			//"<li><a href='%s/bbsmywww'>看不了图片？"
			"</a></li></div>\n", baseurl, baseurl);
	if (file_size("0Announce/focus") > 0) {
		printf("<div class=\"annbox\">\n"
			"<h2>站内公告</h2>\n");
		showfile("0Announce/focus");
		printf("</div>");
	}
	if (aux && aux->nrelate) {
		printf("<div class=\"annbox\">\n"
			"<h2>相关版面</h2>\n");
		for (i = 0; i < aux->nrelate; i++) {
			strsncpy(title, void1((unsigned char *)
					titlestr(aux->relate[i].title)), 40);
			printf("<li><a href=\"%s/doc?B=%d\" name=\"%s\">"
				"%s</a></li>", baseurl,
				getbnum(aux->relate[i].filename), 
				title, title);
		}
		printf("</div>");
	}
/*
	printf("<div class=\"annbox\">\n"
		"<h2>精彩话题</h2>\n"
		"</div>");*/
	//printf("<div class=\"annbox\">\n"
	//	"<h2>热门版面</h2>\n");
	//showhotboard(sec, "12");
	//printf("</div>");
	if (file_exist(MY_BBS_HOME "/wwwtmp/googleads")) {
		printf("<div class=\"annbox\" "
				"style=\"text-align: center;\">\n");
		showfile(MY_BBS_HOME "/wwwtmp/googleads");
		printf("</div>");
	}
	printf("</div>");	//for con_main_right
}

int
bbscon_main(void) {
	char board[80], dir[80], file[80], filename[80], boardtitle[40];
	char seotitle[60];
	char *pathparm;
	char prevfile[80], nextfile[80];
	struct fileheader *x = NULL, *dirinfo = NULL, *ot[CON_MAX_OTHERTHREAD];
	struct boardmem *bx;
	int num, total, sametitle, otnum = 0, otstart;
	int thread, prevnum = -1, nextnum = -1, prevedit, nextedit;
	int outgoing, anony;
	int inndboard;
	int debug = atoi(getparm("debug"));
	int spider = isSpider() || debug ? 1 : 0;
	int myreply_mode=0;
	char buf[256];
	struct mmapfile mf = { ptr:NULL };
	//Add keyword for http meta
	char htmlKeyword[]="一塌糊涂 ytht 一路BBS ";

	/* An old url: http://yjrg.net/HT/con?B=123&F=M.1234567.A&N=1234
	 * A new one: http://yjrg.net/HT/con_123_M_1234567_A.htm?N=1234
	 */

	if (readuservalue(currentuser->userid, 
				"myreply_mode", buf, sizeof (buf)) >= 0)
		myreply_mode = atoi(buf);

	changemode(READING);

	pathparm = getsenv("SCRIPT_URL");
	strncpy(baseurl, pathparm, sizeof(baseurl));
	if (strchr(baseurl + 1, '/'))
		*(strchr(baseurl + 1, '/')) = 0;

	if (strstr(pathparm, "..") || strstr(pathparm, "//"))
		http_fatal("错误的路径 1。");

	if (!(pathparm = strchr(pathparm + 1, '/')))
		http_fatal("错误的路径 2。");
	if (!strncmp(pathparm, "/con/", 5)) {
		bzero(board, sizeof(board));
		strncpy(board, pathparm + 5, sizeof(board));
		if (!(strchr(board, '/')) || 
				!(pathparm = strchr(pathparm + 5, '/')))
			http_fatal("错误的路径 3。");
		*(strchr(board, '/')) = 0;
		*(pathparm++) = 0;
		if ((strncmp(pathparm, "M.", 2) && 
				strncmp(pathparm, "G.", 2)) || 
				strncasecmp(pathparm + 12, ".A.htm", 6))
			http_fatal("错误的路径 4。<br />%s", pathparm);
		bzero(file, sizeof(file));
		strncpy(file, pathparm, 14);
	} else if (!strncmp(pathparm, "/con_", 5)) {
		bzero(board, sizeof(board));
		strncpy(board, pathparm + 5, sizeof(board));
		if (!strchr(board, '_') ||
			!(pathparm = strchr(pathparm + 5, '_')))
				http_fatal("错误的路径 5。");
		*(strchr(board, '_'))= 0;
		*(pathparm++) = 0;
		if ((*pathparm != 'M' && *pathparm != 'G')
				|| strncasecmp(pathparm + 13, "A.htm", 5))
			http_fatal("错误的路径 6。<br />%s", pathparm);
		bzero(file, sizeof(file));
		sprintf(file, "%c.%d.A", *pathparm, atoi(pathparm + 2));
	} else {
		getparmboard(board, sizeof (board));
		strsncpy(file, getparm2("F", "file"), 32);
		if (strncmp(file, "M.", 2) && strncmp(file, "G.", 2))
			http_fatal("错误的参数1");
		if (strstr(file, "..") || strstr(file, "/"))
			http_fatal("错误的参数2");
	}

	num = atoi(getparm2("N", "num")) - 1;
	sametitle = atoi(getparm("st"));

	if ((bx = getboard(board)) == NULL)
		http_fatal("错误的讨论区");
	parm_add("B", board);
	parm_add("F", file);
	sprintf(filename, "boards/%s/%s", board, file);
	if (*getparm("attachname") == '/') {
		showbinaryattach(filename);
		return 0;
	}

	strsncpy(boardtitle, 
		void1((unsigned char *) titlestr(bx->header.title)), 40);
	sprintf(dir, "boards/%s/.DIR", board);
	inndboard = bx->header.flag & INNBBSD_FLAG;
	anony = bx->header.flag & ANONY_FLAG;
	if (!strcmp(board, DEFAULTANONYMOUS))
		anony = 2;
	MMAP_TRY {
		if (mmapfile(dir, &mf) == -1) {
			MMAP_UNTRY;
			http_fatal("此讨论区不存在或者为空");
		}
		total = mf.size / sizeof (struct fileheader);
		if (total <= 0) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("此讨论区不存在或者为空");
		}
		dirinfo = findbarticle(&mf, file, &num, 1);
		if (dirinfo == NULL) {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("本文不存在或者已被删除");
		}

/*	 	if (cache_header(fh2modifytime(dirinfo), 86400)) { 
			mmapfile(NULL, &mf);
			MMAP_RETURN(0);
		}*/
		html_header(1);
		if (dirinfo->accessed & FH_MATH) {
			usingMath = 1;
			usedMath = 1;
			withinMath = 0;
		} else {
			usingMath = 0;
		}
		if (dirinfo->owner[0] == '-') {
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_fatal("本文已被删除");
		}
		//if (spider) {
			memset(seotitle,0x00,60);
			strncpy(seotitle,dirinfo->title,strlen(dirinfo->title));
			//strcpy(seotitle, dirinfo->title);
			delete_redundant_word( seotitle, "[转载]");
			delete_redundant_word( seotitle, "[转寄]");
			delete_redundant_word( seotitle, "[合集]");
			delete_redundant_word( seotitle, "[转贴]");
			delete_redundant_word( seotitle, "(转载)");
			//set meta keywords
			printf
				("<meta name=\"keywords\" content=\"%s %s %s\">\n", 
						boardtitle,htmlKeyword,seotitle);
//暂时去掉description，让Google显示全文内容，按理description应该显示正文前面几十个字，但是这会造成读取两次文件，使得I/O负担增大。
//考虑将来建立数据库后再说
/*			printf
				("<meta name=\"description\" content=\"一路BBS%s区帖子%s全文内容\">\n", 
						boardtitle,seotitle);*/
			printf("<title>%s</title>", seotitle);
		/*	printf("<h1>%s</h1>\n", seotitle);
			printf("<!-- google_ad_section_start -->\n");
			fshowcon(stdout, filename, 4);
			printf("\n<!-- google_ad_section_end -->\n");
			printf("<br /><br />");
			bbscon_show_article_link(getbnumx(bx), file);
			printf("<hr><p>%s</p>", MY_BBS_NAME);
			mmapfile(NULL, &mf);
			MMAP_UNTRY;
			http_quit();
		}*/
//查看信息放在左侧iframe里
//		check_msg();
		printf("<script type=\"text/javascript\">var baseurl = '%s/';</script>", baseurl);
/*
		printf("<script src=\"" BBSCONJS "\"></script>"
			"<script src=\"" BBSAJAXJS "\"></script>"
			"<script src=\"" BBSCOOKIEJS "\"></script>"
			"</head><body onload=\"con_check_width();\" "
			"onUnload = \"con_check_reply();\">\n"
			"<script>con_show_head('"MY_BBS_NAME"', "
			"'%s', '%c', '%s', %d, %d);</script>\n",
				nohtml(getsectree(bx->header.sec1)->title), 
				bx->header.sec1[0],
				boardtitle,
				getbnumx(bx), atoi(getparm("redirect")));
*/
//修改框架，增加左侧
//<body topmargin=0 leftMargin=1 MARGINWIDTH=1 MARGINHEIGHT=0
		printf("<script src=\"" BBSCONJS "\" type=\"text/javascript\"></script>"
			"<script src=\"" BBSAJAXJS "\" type=\"text/javascript\"></script>"
			"<script src=\"" BBSCOOKIEJS "\" type=\"text/javascript\"></script>"
			"</head><body TOPMARGIN=0 LEFTMARGIN=1 MARGINWIDTH=1 MARGINHEIGHT=0 onload=\"con_check_width();\" "
			"onUnload = \"con_check_reply();\">\n" WWWLEFT_DIV 
			"<script type=\"text/javascript\">con_show_head('"MY_BBS_NAME"', "
			"'%s', '%c', '%s', %d, %d);</script>\n",
				nohtml(getsectree(bx->header.sec1)->title), 
				bx->header.sec1[0],
				boardtitle,
				getbnumx(bx), atoi(getparm("redirect")));

		printf("<script type=\"text/javascript\">var defaultsmagic = '%s';</script>", SMAGIC);
#ifdef ENABLE_MYSQL
		printf("<script type=\"text/javascript\">var enmysql = 1;</script>");
#else
		printf("<script type=\"text/javascript\">var enmysql = 0;</script>");
#endif
		printf("<div id=\"con_main\">\n"
				"<div id=\"con_main_left\">\n");
		printf("<div id=\"con_%d_box\" class=\"con_box_%d\">\n", 
				dirinfo->filetime, contentbg);
		if (dirinfo->accessed & FH_ALLREPLY) {
			FILE *fp;
			fp = fopen("bbstmpfs/dynamic/Bvisit_log", "a");
			if (NULL != fp) {
				fprintf(fp,
					"www user %s from %s visit %s %s %s",
					currentuser->userid, fromhost,
					bx->header.filename, fh2fname(dirinfo),
					ctime(&now_t));
				fclose(fp);
			}
		}
		thread = dirinfo->thread;
		*prevfile = 0;
		*nextfile = 0;
		nextedit = 0;
		prevedit = 0;
		if (sametitle) {
			prevnum = num - 1;
			nextnum = num + 1;
			while (prevnum >= 0 && num - prevnum < 100) {
				x = (struct fileheader *) (mf.ptr + 
					prevnum * sizeof (struct fileheader));
				if (x->thread == thread)
					break;
				prevnum--;
			}
			if (prevnum >= 0 && num - prevnum < 100) {
				prevnum++;
				strsncpy(prevfile, fh2fname(x), 32);
				prevedit = feditmark(x);
			} else {
				prevnum = -1;
			}
				
			while (nextnum < total && nextnum - num  < 100) {
				x = (struct fileheader *) (mf.ptr +
					nextnum * sizeof (struct fileheader));
				if (x->thread == thread)
					break;
				nextnum++;
			}
			if (nextnum < total && nextnum - num < 100) {
				nextnum += 1;
				strsncpy(nextfile, fh2fname(x), 32);
				nextedit = feditmark(x);
			} else {
				nextnum = -1;
			}
		} else {
			if (num > 0) {
				x = (struct fileheader *) (mf.ptr + (num - 1) 
						* sizeof (struct fileheader));
				prevnum = num;
				strsncpy(prevfile, fh2fname(x), 32);
				prevedit = feditmark(x);
			}
			if (num < total - 1) {
				x = (struct fileheader *) (mf.ptr + (num + 1) 
						* sizeof (struct fileheader));
				nextnum = num + 2;
				strsncpy(nextfile, fh2fname(x), 32);
				nextedit = feditmark(x);
			}
		}
		otstart = max(min(num + CON_MAX_OTHERTHREAD / 2 + 2, 
				total - 1) - CON_MAX_OTHERTHREAD, 0);
		x = (struct fileheader *) (mf.ptr + otstart * 
				sizeof(struct fileheader));
		for (; otnum < CON_MAX_OTHERTHREAD && otstart + otnum < total; 
				otnum++, x++) {
			ot[otnum] = x;
		}

		if (otnum < CON_MAX_OTHERTHREAD)
			ot[otnum] = NULL;
		if (num >= 0 && num < total) {
			brc_initial(currentuser->userid, board);
			brc_add_read(dirinfo);
			brc_update(currentuser->userid);
		}
		//dirinfo->title[sizeof (dirinfo->title) - 1] = 0;
		outgoing = (dirinfo->accessed & FH_INND) 
			|| strchr(dirinfo->owner, '.');

		printf("<script type=\"text/javascript\">con_show_connav(%d,%d,'%s', %d, %d, %d, %d, "
				"%d, %d, %d, %d, %d, '%s', '%s', %d, %d, "
				"%d, %d, %d, %d, 1)</script>", 
			num,myreply_mode, file, dirinfo->thread, getbnumx(bx), 
			!strncmp(currentuser->userid, 
				dirinfo->owner, IDLEN + 1) 
				|| has_BM_perm(currentuser, bx), 
			dirinfo->staravg50, dirinfo->hasvoted, sametitle, 
			dirinfo->filetime == dirinfo->thread,
			prevnum, nextnum, prevfile, nextfile, prevedit, 
			nextedit, inndboard, dirinfo->accessed & FH_NOREPLY, 
			outgoing, anony);
		binarylinkfile(getparm2("F", "file"));
		printf("<div id=\"con_%d_content_box\" "
			"class=\"con_content_box_%d\">"
			"<div id=\"con_%d_content_msg\" "
			"class=\"con_content_msg_%d\"></div>", 
			dirinfo->filetime, contentbg, 
			dirinfo->filetime, contentbg);
		if(w_info->mypic_mode == 0)
			printmypicbox(fh2owner(dirinfo));
		//在这里print 标题与正文内容
		printf("<h1>%s</h1><br>\n", seotitle);
		printf("%s发表于%s  http://yilubbs.com<br>\n", dirinfo->owner, Ctime(dirinfo->filetime));

		if (hideboard(board) || (!via_proxy
			&& (!wwwcache->accel_addr.s_addr || 
			!wwwcache->accel_port)) || w_info->doc_mode) {
		//if (!strcmp(currentuser->userid, "ylsdd"))
		//      errlog("%s--%s--%d--%s", dirinfo->owner,
		//             fh2owner(dirinfo), nbuf, dirinfo->title);
			fshowcon(stdout, filename, 0);
		} else {
			sprintf(filename, "boards/%d/%s", getbnumx(bx), file);
			fshowcon(stdout, filename, 1);
		}
		// show article link
		printf("<div id=\"con_%d_content_link\" "
				"class=\"con_content_link_%d\">"
				"<div class=\"con_content_link_padding_%d\">", 
				dirinfo->filetime, contentbg, contentbg);
		bbscon_show_article_link(getbnumx(bx), file);
		// show article link end
		printf("</div></div></div>\n");
		printf("<script>con_show_connav(%d,%d, '%s', %d, %d, %d, %d, "
			"%d, %d, %d, %d, %d, '%s', '%s', %d, %d, %d, %d, "
			"%d, %d, 0)</script>\n", 
			num, myreply_mode,file, dirinfo->thread, getbnumx(bx), 
			!strncmp(currentuser->userid, 
				  dirinfo->owner, IDLEN + 1) 
			 	|| has_BM_perm(currentuser, bx), 
			dirinfo->staravg50, dirinfo->hasvoted, sametitle, 
			dirinfo->filetime == dirinfo->thread,
			prevnum, nextnum, prevfile, nextfile, 
			prevedit, nextedit, inndboard, 
			(dirinfo->accessed & FH_NOREPLY), outgoing, anony);
		printf("<script>con_new_title(%d, '%s');</script>", 
				dirinfo->filetime, scriptstr(dirinfo->title));
		con_print_reply_box(dirinfo);
		printf("</div>\n"	//for con_%d_box
			"<div id=\"con_doclist_nav\">\n"
			"<div id=\"con_doclist_nav_padding\">\n"
			"</div></div>\n"
			"<div id=\"con_doclist_ajax\">\n"
			"<div id=\"con_doclist_ajax_padding\">\n"
			"</div></div>\n"
			"<div class=\"clear\"></div>");
		printf("<script>con_docnav_init(%d, %d, %d, %d, 0);</script>\n", 
			dirinfo->filetime, CON_MAX_OTHERTHREAD, 
			(num - 1)/ CON_MAX_OTHERTHREAD + 1, 
			(total - 1)/ CON_MAX_OTHERTHREAD + 1);
		printf("<script>\n"
			"var tzdiff = (new Date()).\n"
			"getTimezoneOffset()*60 + 8*3600;\n"
			"var today = new Date((%d + tzdiff)*1000);\n"
			"var board = %d;\n"
			"var ftime = %d;\n"
			"</script>\n"
			"<div id=\"con_doclist_content\"></div>\n"
			"<script>con_doclist_head('con_doclist_content');"
			"</script>\n",
			(int)time(NULL), getbnumx(bx), dirinfo->filetime);
		otnum = 0;
		while(otnum < CON_MAX_OTHERTHREAD && ot[otnum]) {
			printf("<script>con_doclist_item(%d, '%s', '%s', "
				"'%s', %d, '%s', %d, %d, %d);</script>\n", 
				otstart + otnum + 1, 
				flag_str2(ot[otnum]->accessed, 
					!brc_un_read(ot[otnum])), 
				fh2owner(ot[otnum]), 
				fh2fname(ot[otnum]), feditmark(ot[otnum]), 
				scriptstr(ot[otnum]->title), 
				bytenum(ot[otnum]->sizebyte), 
				ot[otnum]->staravg50 / 50, 
				ot[otnum]->hasvoted);
			otnum++;
		}
		printf("<script>con_doclist_end('con_doclist_content');"
				"</script>"
				"</div>\n");	//for con_main_left
		con_print_main_right(bx, dirinfo);
		printf("</div><div class=\"clear\"></div><br><br><br>");
					//for con_main
		processMath();
		printf("<script>con_switch_right_bycookie();</script>");
		showfile(MY_BBS_HOME "/wwwtmp/googleanalytics");
//		printf("</body></html>\n");
//修改框架，增加底部
		printf(WWWFOOT_DIV "</body></html>\n");

	}
	MMAP_CATCH {
		mmapfile(NULL, &mf);
		MMAP_RETURN(-1);
	}
	MMAP_END mmapfile(NULL, &mf);
	return 0;
}

int
bbscon_show_article_link(int board, char *file)
{
	char link[PATH_MAX];
#ifndef USEBIG5
	snprintf(link, sizeof(link), "http://%s/" BASESMAGIC
		"/con_%d_%s.htm", getsenv("HTTP_HOST"), board, file);
#else
	snprintf(link, sizeof(link), "http://%s/" BASESMAGICBI5
		"/con_%d_%s.htm", getsenv("HTTP_HOST"), board, file);
#endif
	printf("全文链接: %s<br>", link);
	printf("本文内容仅代表作者观点，与本站立场无关。<br>");
	return 0;
}
