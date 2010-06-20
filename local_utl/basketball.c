#include <stdio.h>
#include "bbs.h"
#include "ythtlib.h"
#include "www.h"

#define	BOARD	"basketball"

int main () {
	FILE *markfp, *digestfp, *commonfp;
	char boarddir[STRLEN], markarticles[STRLEN], digestarticles[STRLEN], buf[STRLEN];
	int n, total, fd, marked = 0, digested = 0, pos = -1;
	struct fileheader fhdr;
	struct boardmem x;
	struct bbsinfo bbsinfo;
	struct BCACHE *shm_bcache;
	
	if (initbbsinfo(&bbsinfo) < 0)
		return -1;
	shm_bcache = bbsinfo.bcacheshm;
	for (n = 0; n < shm_bcache->number; n++) {
		x = shm_bcache->bcache[n];
		if (!strcmp(x.header.filename, BOARD)) {
			pos = n;
			break;
		}
	}
	if (pos < 0)
		return -1;

	sprintf(boarddir, MY_BBS_HOME "/boards/%s/.DIR", BOARD);
	sprintf(markarticles, MY_BBS_HOME "/ftphome/root/boards/%s/html/marked.js", BOARD);
	sprintf(digestarticles, MY_BBS_HOME "/ftphome/root/boards/%s/html/digested.js", BOARD);
	sprintf(buf, MY_BBS_HOME "/ftphome/root/boards/%s/html/commonjs.js", BOARD);
	if ((fd = open(boarddir, O_RDONLY, 0)) == -1)
		return -1;
	if ((markfp = fopen(markarticles, "w+")) == NULL)
		return -1;
	if ((digestfp = fopen(digestarticles, "w+")) == NULL)
		return -1;
	if ((commonfp = fopen(buf, "w+")) == NULL)
		return -1;
	
	if ((total = file_size(boarddir) / sizeof (fhdr)) <= 0)
		return -1;

	fprintf(commonfp, "<!--\n"
			"function beginbox() {\n"
			"\tdocument.write(\"<table width=\\\"100%%\\\"  border=\\\"0\\\">\\n\");\n"
			"}\n\n"
			"function endbox() {\n"
			"\tdocument.write(\"</table>\\n\");\n"
			"}\n\n"
			"function showitem(title, owner, bnum, ftime) {\n"
			"\tvar sTitle, iCount, i, strTemp, cuttitle;\n"
			"\tiCount = 0;\n"
			"\tsTitle = title.split(\"\");\n"
			"\tfor (i = 0 ; i < sTitle.length ; i ++) {\n"
			"\t\tstrTemp = escape(sTitle[i]);\n"
			"\t\tif (strTemp.indexOf(\"%%u\",0) == -1) // ±íÊ¾ÊÇºº×Ö\n"
			"\t\t\tiCount = iCount + 1 ;\n"
			"\t\telse\n"
			"\t\t\tiCount = iCount + 2 ;\n"
			"\t}\n"
			"\tcuttitle = iCount > 26 ? title.substring(0, 18) + \"...\" : title;\n"
			"\tdocument.write(\"<tr valign=\\\"middle\\\">\\n\"\n"
			"\t\t+\"<td width=\\\"70%%\\\" height=\\\"10\\\">\\n\"\n"
			"\t\t+\"<div align=\\\"left\\\">\\n\"\n"
			"\t\t+\"¡¤\\n\"\n"
			"\t\t+\"<a href = \\\"../../../../con?B=\"+bnum+\"&F=M.\"+ftime+\".A\\\" target=\\\"_parent\\\">\""
			"+cuttitle+\"</a>\\n\"\n"
			"\t\t+\"</div>\\n\"\n"
			"\t\t+\"</td>\\n\"\n"
			"\t\t+\"<td width=\\\"30%%\\\">\\n\"\n"
			"\t\t+\"<div align=\\\"right\\\">\\n\"\n"
			"\t\t+\"by <a href=\\\"../../../../qry?U=\"+owner+\"\\\" target=\\\"_parent\\\">\"+owner+\"</a>\\n\"\n"
			"\t\t+\"</div>\\n\"\n"
			"\t\t+\"</td>\\n\"\n"
			"\t\t+\"<tr valign=\\\"middle\\\">\\n\"\n"
			"\t\t+\"<td class=\\\"dotLine\\\" colspan=\\\"2\\\"></TD>\\n\"\n"
			"\t\t+\"</tr>\\n\"\n"
			"\t);\n"
			"}\n"
			"-->\n");
	fclose(commonfp);

	fprintf(markfp, "<!--\n"
			"\tbeginbox();\n");
	fprintf(digestfp, "<!--\n"
			"\tbeginbox();\n");
	for (n = total - 1; n >= 0 && total - n < 3000 && 
			(marked < 10 || digested < 10); n--) {
		if (lseek(fd, n * sizeof(fhdr), SEEK_SET) < 0)
			return 0;
		if (read(fd, &fhdr, sizeof (fhdr)) != sizeof (fhdr))
			return 0;
		if (fhdr.accessed & FH_MARKED && marked < 10) {
			fprintf(markfp, "\tshowitem(\"%s\", \"%s\", %d, %d);\n",
					fhdr.title, fhdr.owner, pos, fhdr.filetime);
			marked++;
		}
		if (fhdr.accessed & FH_DIGEST && digested < 10) {
			fprintf(digestfp, "\tshowitem(\"%s\", \"%s\", %d, %d);\n",
					fhdr.title, fhdr.owner, pos, fhdr.filetime);
			digested++;
		}
	}
	fprintf(markfp, "\tendbox();\n"
			"-->\n");
	fprintf(digestfp, "\tendbox();\n"
			"-->\n");
	fclose(markfp);
	fclose(digestfp);
	return 1;
}

