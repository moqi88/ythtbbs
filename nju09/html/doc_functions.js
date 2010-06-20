function openlog()
{
	open('bbslform','','left=255,top=190,width=130,height=100');
}
function eva(board, file) {
var s;
	s=" [<a href='eva?B="+board +"&amp;F="+file+"&amp;star=";
	document.writeln("喜欢这个文章么? 这个文章"+s+"1'>不错</a>]"+s+"3'>很好</a>]"+s+"5'>我顶!</a>]");
}
function docform(a, b){
	document.writeln("<table border=0 cellspacing=0 cellpading=0><tr><td><form name=docform1 method=post action="+a+"><a href="+a+"?B="+b+"&S=1>第一页</a> <a href="+a+"?B="+b+"&S=0>最后一页</a> <input type=hidden name=B value="+b+"><input type=submit value=转到>第<input type=text name=start size=4>篇</form></td><td><form name=docform2 method=post action="+a+"><input type=submit value=转到><input type=text name=B size=7>讨论区</form></td></tr></table>");
}
//tabs显示版面选项卡
function doc_show_tabs_item(cgi, highlight, str)
{
	var tdclass, lnkclass;
	document.write("<li style='display: block; float: left;' class="
		+(highlight ? "doc_tab_li_highlight" : "doc_tab_li")+
		"><a href="+cgi+" class="
		+(highlight ? "doc_tab_a_highlight" : "doc_tab_a")+">"+str+"</a></li>");
}

function doc_show_tabs(board, num, hasindexpage, hasbacknumber, ancpath, total, inboard, hasvote, has_bm_perm)
{
	document.write("<table width=100% cellSpacing=0 border=0 cellPadding=0 \
	class=doc_tab_table><tr class=doc_tab_tr>\
	<td valign=bottom class=doc_tab_td>\
	<div><ul style='clear: left; list-style-type: none; display: inline;'>");
	if(hasindexpage)
		doc_show_tabs_item("home?B="+board, false, "进版页面");
	doc_show_tabs_item("not?B="+board, num==1, "备忘录");
	doc_show_tabs_item("doc?B="+board, num==3, "讨论区");
	doc_show_tabs_item("gdoc?B="+board, num==4, "文摘区");
	if(hasbacknumber)
		doc_show_tabs_item("bknsel?B="+board, num==5, "过刊区");
	doc_show_tabs_item("0an?path="+ancpath, num==6, "精华区");
	if (has_bm_perm)
		doc_show_tabs_item("bbsdenyall?B="+board, num==7, "封禁名单");
	document.write("</ul></div></td><td class=doc_tab_td_info valign=bottom>");
	document.write("版内共有文章 "+total+" 篇，在线 "+inboard+" 人。");
	if(hasvote)
		document.write("<a href=vote?B="+board+" class=red>投票中!</a> ");
	document.write("</td></tr></table>");
}

function phr()
{
	document.writeln("<table width=100% cellspacing=1 cellpadding=0><tr class=hrcolor><td><img height=1 src=/small.gif width=1></td></tr></table>");
}

function phrw()
{
	document.writeln("<table width=100% cellspacing=1 cellpadding=0><tr><td><img height=1 src=/small.gif width=1></td></tr></table>");
}

//打印文章列表条目
var monthStr = new Array("January","February","March","April","May","June","July","August","September", "October","November","December");
var board ="";
var num = 1;
var nowt = 0;
var today;
var tzdiff = (new Date()).getTimezoneOffset()*60 + 8*3600;

function sizeStr(size)
{	
	if(size<1000)
		return "(<font class=tea>"+size+"字</font>)";
	return "(<font class=red>"+Math.floor(size/1000)+"."
		+Math.floor(size/100)%10+"千字</font>)";
}

function shortFDate(fn)
{
	var sec= parseInt(fn.slice(2,-2));
	//var fdate = new Date((sec+tzdiff)*1000);
	var fdate = new Date(sec*1000);
	var retv = "";
	if(nowt-sec<24*3600 && fdate.getDate() == today.getDate()) {
		if(fdate.getHours()<10)
			retv+="0";
		retv+=fdate.getHours()+":";
		if(fdate.getMinutes()<10)
			retv+="0";
		retv+=fdate.getMinutes();
		return retv;
	}
	return monthStr[fdate.getMonth()].substr(0,3)+" "+fdate.getDate();
}

function replaceString(oldS, newS, fullS)
{
	for(var i=0; i<fullS.length; i++) {
		if(fullS.substr(i,oldS.length)==oldS) {
			fullS = fullS.substr(0, i)+newS+fullS.substr(i+oldS.length,fullS.length);
			i+=newS.length-1;
		}
	}
	return fullS;
}

function aStr(title)
{
	title = replaceString("&", "&amp;", title);
	title = replaceString("  ", " &nbsp;", title);
	title = replaceString("<", "&lt;", title);
	title = replaceString(">", "&gt;", title);
	return title;
}

function titleStr(title)
{
	title = aStr(title);
	if(title.substr(0,4)=="Re: ")
		return title;
	return "○ "+title
}

function evaStr(star, neval)
{
	if(star)
		return "<font class=red><nobr>"+star+"/"+neval+"人</nobr></font>";
	else
		return "<nobr>0/0人</nobr>"
}

function docItemInit(aboard, firstnum, anowt)
{
 	board = aboard;
	num = firstnum;
	nowt = anowt;
	today = new Date((nowt + tzdiff)*1000);
}


var lasttitle = '';

function ntItem(num, flag, author, fname, edt, title, size, star, neval)
{
	var str = '';
	if(title.length!=0)
		lasttitle = titleStr(title);
	str = "<tr><td>"+num+"</td>";
	if(flag.charAt(0) == "N")
		str += "<td class=gry>"+flag+"</td>";
	else
		str += "<td>"+flag+"</td>";
	if(author.indexOf('.')==-1)
		str += "<td><a href=qry?U="+author+" >"+author+"</a></td>";
	else
		str += "<td>"+author+"</td>";
	str += "<td><nobr>"+shortFDate(fname)+"</nobr></td>";
	str += "<td><a href=con_"+board+"_"+fname
		+".htm>"+lasttitle+"</a>"+sizeStr(size)+"</td>";
	str += "<td>"+evaStr(star, neval)+"</td></tr>";
	document.write(str);
}

//打印导读条目
function btb(sec, sectitle)
{
	document.write('<table cellpadding=2 cellspacing=0 border=0 width=100%><tr class=tb2_blk><td width=15><font class=star>★</font></td><td><a href=boa?secstr='+sec+' class=blk><B>' + aStr(sectitle) +'</B> &gt;&gt;more</td></tr>');
}

function etb()
{
	document.write('</table>');
}

function itb(board, boardtitle, thread, title)
{
	document.write('<tr><td valign=top>·</td><td><a href=bbsnt?B='+board+'&th='+thread+'>'+aStr(title)+
			'</a> <font class=f1>&lt;<a href=home?B='+board+' class=blk>'+aStr(boardtitle)+
			'</a>&gt;</font></td></tr>');
}

function ltb(sec, list)
{
	var i, count = list.length/2;
	var l = '<tr><td valign=top>·</td><td class=f1>';
	for(i=0;i<count;i++) {
		l += '<nobr>&lt;<a href=home?B='+list[i*2]+' class=blk>'+aStr(list[i*2+1])+'</a>&gt;</nobr> ';
	}
	l+='<a href=boa?secstr='+sec+' class=blk>…</a></td></tr>';
	document.write(l);
}

function replaceStrings(from, to, str) 
{
	var i, len = from.length;
	for(i=0;i<len;i++)
		str= replaceString(from[i], to[i], str);
	return str;
}

//www white -> telnet black; www black--> telnet white
var colors = new Array("white", "red", "green", "yellow", "blue", "purple", "cyan", "black");
var asciiChar = new Array('&', ' ', '<', '>');
var htmlChar = new Array('&amp;', '&nbsp;', '&lt;', '&gt;');
var htmlCharR = new Array(/&amp;/gi, /&nbsp;/gi, /&lt;/gi, /&gt;/gi);

function ansi2html_font(str)
{
	var currfont = new aFont(0);
	var retstr = '', af;
	var fontStack = new Array(new aFont(0));
	fontStack[0].tag='------------';
	var reg1=/^\033\[(<\/[^>]*>)/, reg2=/^\033\[(<[^>]*>)/;
	var reg3=/^\033\[([0-9;]*)m/, reg4=/^\033\[*[0-9;]*\w*/;
	while(true) {
		i = str.indexOf('\033');
		if( i==-1) {
			retstr += str;
			break;
		}
		retstr += str.substr(0, i);
		str = str.slice(i);
		if((myArray = reg1.exec(str))!=null) {
			retstr+=myArray[1];
			tag=myArray[1].replace(/^\033\[<\/(\w*)[^>]*>/, '$1').toLowerCase();
			for(i=0;i<fontStack.length-1;i++) {
				if(tag==fontStack[i].tag)
					break;
			}
			if(i<fontStack.length-1) {
				for(i++;i!=0;i--) fontStack.shift();
				retstr+=fontSwitchHtml(fontStack[0], currfont);
			}
		} else if((myArray = reg2.exec(str))!=null) {
			retstr+=myArray[1];
			tag=myArray[1].replace(/^\033\[<(\w*)[^>]*>/, '$1').toLowerCase();
			if(tag.length!=0&&tag!='br') {
				fontStack.unshift(new aFont(fontStack[0]));
				fontStack[0].tag=tag;
			}
		} else if((myArray = reg3.exec(str))!=null) {
			//reportfont(ansiFont(myArray[1]), 'ansiFont');
			currfont = realfont(currfont, ansiFont(myArray[1]));
			retstr+=fontSwitchHtml(fontStack[0], currfont);
                } else {
			//alert("AA"+str);
			myArray = reg4.exec(str);
		}
		str = str.slice(myArray[0].length);
	}
	return retstr;
}


function ansi2html(str)
{
	var i, len;
	var retv = '';
	str = str.replace(/\r/g, '');
	str = str.replace(/\033\n/g, '');
	str = str.replace(/\033<[^>]>/g, '');
	str = '\033[<>'+str;
	function afunc(s, p1, p2) {
		for(var i = 0; i<htmlChar.length; i++) { 
			p2 = replaceString(asciiChar[i], htmlChar[i], p2);
		}
		return p1+p2;
	}
	//str = ansi2html_font(str);
	str = str.replace(/(\033\[<[^>]*>)([^\033]*)/g, afunc);
	str = str.replace(/(\033\[)([^<][^\033]*)/g, afunc);
	str = str.replace(/(\033[^\[])([^\033]*)/g, afunc);
	str = str.substr(4, str.length-4);
	str = str.replace(/\n/g, '<br>');
	str = ansi2html_font(str);
	return str;
}

function replaceStringR(regstr, to, str)
{
	var regEx=new RegExp(regstr, 'gi');
	return str.replace(regEx, to);
}

function colornum(str)
{
	str = str.toLowerCase();
	for(var i = 0; i<8;i++) {
		if(colors[i]==str)
			return i;
	}
	return -1;
}

function ansiFont(str)
{
	if(str.length==0)
		return new aFont(0);
	var af=new aFont('');
	function num2font(s, p1) {
		var n=parseInt(p1);
		if(n>29&&n<38) af.color=n-30;
		if(n>39&&n<48) af.bg=n-40;
		if(n==0) af=new aFont(0);
		if(n==101) af.b=1;
		if(n==102) af.b=0;
		if(n==111) af.it=1;
		if(n==112) af.it=0;
		if(n==4) af.u=1;
		return '';
	}
	str.replace(/([0-9]*);/g, num2font);
	str.replace(/([0-9]*)/g, num2font);
	return af;
}

function aFont(str)
{
	if(typeof(str)==typeof(0)&&str==0) {
		this.b=0;
		this.u=0;
		this.it=0;
		this.bg=0;
		this.color=7;
		return;
	}
	if(typeof(str)!='string') {
		this.b=str.b;
		this.u=str.u;
		this.it=str.it;
		this.bg=str.bg;
		this.color=str.color;
		return;
	}
	this.b=-1;
	this.u= -1;
	this.it=-1;
	this.bg=-1;
	this.color=-1;
	if(str.length==0)
		return;
	str = str.toLowerCase();
	str = str.replace(/background-color/g, "background");
	if(str.indexOf("bold")!=-1)
		this.b=1;
	if(str.indexOf("underline")!=-1)
		this.u= 1;
	if(str.indexOf("italic")!=-1)
		this.it=1;
	reg = /background[:=\'\" ]+(\w+)/;
	myArray = reg.exec(str);
	if(myArray!=null)
		this.bg=colornum(myArray[1]);
	reg = /color[:=\'\" ]+(\w+)/
	myArray = reg.exec(str);
	if(myArray!=null) {
		this.color = colornum(myArray[1]);
	}
}

function realfont(f1, f2)
{
	var af = new aFont(f1);
	if(f2.b!=-1)
		af.b= f2.b;
	if(f2.it!=-1)
		af.it=f2.it;
	if(f2.u!=-1)
		af.u=f2.u;
	if(f2.color!=-1)
		af.color = f2.color;
	if(f2.bg!=-1)
		af.bg=f2.bg;
	return af;
}

function fontSwitchHtml(f1, f2)
{
	var str = '</font><font style="';
	if(f1.b==f2.b&&f1.it==f2.it&&f1.u==f2.u&&f1.color==f2.color&&f1.bg==f2.bg)
		return '</font>';
	if(f2.u!=f1.u)
		str+=(f2.u==1)?"text-decoration: underline;":"text-decoration: none;";
	if(f2.b!=f1.b)
		str+=(f2.b==1)?"font-weight: bold;":"font-weight: normal;";
	if(f2.it!=f1.it)
		str+=(f2.it==1)?"font-style: italic;":"font-style: normal;";
	if(f2.color!=f1.color) 
		str+="color: "+colors[f2.color]+";";
	if(f2.bg!=f1.bg)
		str+="background-color: "+colors[f2.bg]+";";
	return str+'">';
}

function fontSwitchStr(f1, f2)
{
	var str='\033[';
	if(f1.b==f2.b&&f1.it==f2.it&&f1.u==f2.u&&f1.color==f2.color&&f1.bg==f2.bg)
		return '';
	if(f2.b==0&&f2.u==0&&f2.it==0&&f2.color==7&&f2.bg==0)
		return '\033[m';
	if(f2.u<f1.u) {
		str += '0;';
		f1 = new aFont(0);
	}
	if(f2.u==1)
		str+='4;';
	if(f2.b!=f1.b)
		str+=(102-f2.b)+';';
	if(f2.it!=f1.it)
		str+=(112 -f2.it)+';';
	if(f2.color!=f1.color)
		str += f2.color + 30 + ';';
	if(f2.bg!=f1.bg)
		str+=f2.bg+40+';';
	if(str.charAt(str.length-1)==';')
		return str.substr(0, str.length-1)+'m';
	return str+'m';
}

function html2ansi_font(str)
{
	var fontStack = new Array(new aFont(0));
	var reg = /<font([^>]*)>/ig, repstr, af;
	function toansifont(s, p) {
		if(p.indexOf('/')!=-1) {
			af = fontStack[0];
			for(var i=0;i<p.length&&fontStack.length>1;i++)
				if(p.charAt(i) == '/')
					fontStack.shift();
			repstr = fontSwitchStr(af, fontStack[0]);
		} else {
			af = realfont(fontStack[0], new aFont(p));
			repstr = fontSwitchStr(fontStack[0], af);
			fontStack.unshift(af);
		}
		return repstr;
	}
	return str.replace(reg, toansifont);
}

function reportfont(af, title)
{
	alert(title +'b:'+af.b+' i:'+af.it+' u:'+af.u+' '+af.color+' '+af.bg);
}

function lowerTag(str)
{
	function tolower(s, s1, s2, s3) {
		s3=s3.replace(/(onAbort|onBlur|onChange|onClick|onDblClick|onDragDrop|onError|onFocus|onKeyDown|onKeyPress|onKeyUp|onLoad|onMouseDown|onMouseMove|onMouseOut|onMouseOver|onMouseUp|onMove|onReset|onResize|onSelect|onSubmit|onUnload)/gi, '');
		return '<'+s2.toLowerCase()+s3+'>';
	};
	str=str.replace(/<(\s*)(\w*)([^>]*)>/gi, tolower);
	return str.replace(/<(\s*)(\/\w*)([^>]*)>/gi, tolower);
}

function aafunc(s) {
	if(s.indexOf('&')==-1)
		return s;
	for(var i = 0; i<htmlChar.length; i++)
		s = s.replace(htmlCharR[i], asciiChar[i]);
	return s;
}

function html2ansi(str)
{
	//var l = str;
	str=str.replace(/\s*\n\s+/g, ' ');
	str=str.replace(/\n/g, '');
	str=str.replace(/\r/g, '');
	str=lowerTag(str);
	str=str.replace(/-->/g, "-->\n");
	str=str.replace(/<!--.*-->\n/g, '');
	str=str.replace(/\n/g, '');
	str=str.replace(/<form\s[^>]>/g, '<form>');
	str=str.replace(/<\/script[^>]*>/g, "</script>\n");
	str=str.replace(/<script[^>]*>[^\n]*<\/script>\n/g, "");
	str=str.replace(/\n/g, '');
	str=str.replace(/<\/iframe[^>]*>/g, "</iframe>\n");
	str=str.replace(/<iframe.*<\/iframe>\n/g, ''); 
	str=str.replace(/\n/g, '');
	//str=str.replace(/<\/object[^>]*>/g, "</object>\n");
	//str=str.replace(/<object.*<\/object>\n/g, '');
	str=str.replace(/\n/g, '');
	str=str.replace(/<a([^>]*)>/g, '<a target=_blank$1>');
	str=str.replace(/<strong>/g, '<font bold>');
	str=str.replace(/<\/strong>/g, '<font />');
	str=str.replace(/<em>/g, '<font italic>');
	str=str.replace(/<\/em>/g, '<font />');
	str=str.replace(/<i>/g, '<font italic>');
	str=str.replace(/<\/i>/g, '<font />');
	str=str.replace(/<b>/g, '<font bold>');
	str=str.replace(/<b\s[^>]*>/g, '<font bold>');
	str=str.replace(/<\/b>/g, '<font />');
	str=str.replace(/<u>/g, '<font underline>');
	str=str.replace(/<\/u>/g, '<font />');

	str=str.replace(/<span/g, '<font');
	str=str.replace(/<\/span>/g, '<font />');
	str=str.replace(/<\/font>/g, '<font />');

	str=str.replace(/<font (\/+)><font (\/+)>/g, '<font $1$2>');
	str=str.replace(/<font (\/+)><font (\/+)>/g, '<font $1$2>'); //twice
	str=html2ansi_font(str);
	str=str.replace(/<\/p>/g, '</p>\n');
	str=str.replace(/<p>(.*)<\/p>\n/g, '$1<br>');
	str=str.replace(/\n/, '');

	str=str.replace(/<\/(tr|table|center|div|h1|h2|h3|h4|h5|h6|ol|ul|dl)>/g, '<\/$1><tmpbr>');
	str=str.replace(/<(p|li|dt|dd|hr|table)>/g, '<$1><tmpbr>');
	str=str.replace(/<(p|li|dt|dd|hr|table)\s([^>]*)>/g, '<$1 $2><tmpbr>');
	str=str.replace(/<tmpbr>(<[^>]*>)/, '$1<tmpbr>');
	str=str.replace(/<tmpbr>(<[^>]*>)/, '$1<tmpbr>');
	str=str.replace(/<tmpbr><tmpbr>/g, '<tmpbr>');
	str=str.replace(/<tmpbr><tmpbr>/g, '<tmpbr>');
	str=str.replace(/<tmpbr><br>/g, '<br>');
	str=str.replace(/<tmpbr>/g, '\033\n');
	str=str.replace(/<br[^>]*>/g, '\n');
	str=str.replace(/(<[^>]*>)/g, '\033[$1');
	str=str.replace(/(<img [^>]*smilies\/icon_)(\w*)(.gif[^>]>)/g, '$1$2$3\033<\/$2>');
	str= '>'+str+'<';
	str=str.replace(/>[^<]*</g, aafunc);
	str=str.substr(1, str.length-2);
	str=str.replace(/(\033\[<[^>]*>)(#attach )/g, '$1\n$2');
	str=str.replace(/(\n#attach [^\n])(\033)/g, '$1\n$2');
	return str;
}

//Whether String.replace(reg, function) is supported?
function
testReplace()
{
	var s='1';
	function af(s) {
		return '2';
	}
	s=s.replace(/1/g, af);
	if(s!='2')
		return false;
	return true;
}

function
saferDoc(str)
{
	str=lowerTag(str);
	str=str.replace(/<\/script[^>]*>/g, "</script>\n");
	str=str.replace(/<script[^>]*>[^\n]*<\/script>\n/g, "");
	return str;
}

//新导读
function
cmpBoardName(abrd1, abrd2)
{
	var n1=abrd1.board.toLowerCase(), n2=abrd2.board.toLowerCase();
	if(n1>n2)
		return 1;
	return -1;
}

function
cmpBoardScore(abrd1, abrd2)
{
	if(abrd1.isclose!=abrd2.isclose)
		return abrd1.isclose-abrd2.isclose;
	return abrd2.score-abrd1.score; //逆序排列
}

function
lm2str(bl)
{
	var lm=bl.lastmark, retv='';
	if(bl.isclose==1)
		return '<font class=gry>封闭俱乐部</font>';
	if(bl.isclose==2)
		retv+='<b>本版只读</b><br>';
	for(var i=0;i<lm.length;i++)
		retv+='<a href=bbsnt?B='+bl.bnum+'&th='+lm[i].th+'>'+lm[i].title+'</a>'
		 +' <font class=f1>作者[<a href=qry?U='+lm[i].author+'>'+lm[i].author+'</a>]</font><br>';
	if(retv.length==0)
		return '&nbsp;';
	return retv;
}

function
oneBoard(bl)
{
	var icon='/defaulticon.gif', retv='', a='';
	//if(bl[i].hasicon==1)
	//	icon='/'+SMAGIC+'/home/boards/'+bl[i].board+'/html/icon.gif';
	//if(bl[i].hasicon==2)
	if(bl.hasicon!=0)
		icon='icon?B='+bl.bnum;
	else
		if(bl.lastmark.length>4)
			a='<br><br>';
	retv+='<tr><td valign=top width=160><a name='+bl.board+'></a>'
	 +'<a href=home?B='+bl.bnum+'><b>'+bl.title+'</b>'
	 +((bl.title.lenght+bl.board.length)>20?'<br>':'')
	 +'<font class=f1>('+bl.board+')</font></a><br>'
	 +'<center><a href=home?B='+bl.bnum+'><img border=0 alt="" src='+icon
	 +' onload="javascript:if(this.width>180)this.width=180;if(this.height>100)this.height=100;">'
	 +'</a></center>'+a
	 +'<font class=f1>人气：'+bl.score+'</font> &nbsp;'
	 +'<font class=f1>文章数：'+bl.total+'</font><br>'
	 +'<font class=f1>版主：'+bl.bm+'</font><br>'
	 +'</td><td valign=top><div class=colortb1 style="line-height:1.3"><font class=f1>'+bl.intro+'</font></div>'
	 +'<div style="line-height:1.3">'+lm2str(bl)+'</div>'
	 +'</td></tr>';
	 return retv;
}

function
fullBoardList(bl)
{
	var retv='<table border=1 width=100% cellspacing=0 cellpadding=4>';
	bl.sort(cmpBoardScore);
	for(var i=0;i<bl.length;i++) {
		retv+=oneBoard(bl[i]);
	}
	return retv+'</table>';
}

function
boardIndex(bl)
{
	if(bl.length<5)
		return '';
	var retv="<fieldset><legend align=center>版面索引</legend><div align=left class=f1>";
	bl.sort(cmpBoardName);
	for(var i=0; i<bl.length;i++) {
		retv+='<a href=#'+bl[i].board+'>'+bl[i].board+'</a><br>';
	}
	return retv+='</div></fieldset>';
}

function abrd(bnum, board, title, hasicon, total, score, vote, isclose, bm, intro, lm)
{
	this.bnum=bnum;
	this.board=board;
	this.title=aStr(title);
	this.hasicon=hasicon;
	this.total=total;
	this.score=score;
	this.vote=vote;
	this.isclose=isclose;
	this.bm=bm;
	this.intro=aStr(intro);
	this.lastmark=lm;
}

function alm(th,title,au)
{
	this.title=aStr(title);
	this.th=th;
	this.author=au;
}

function
chooseBoard(bl, list)
{
	var nbl = new Array();
	var len = list.length;
	var i, j;
	for(i=0;i<bl.length;i++) {
		for(j=0;j<len;j++) if(list[j]==bl[i].board) break;
		if(j!=len)
			nbl.push(bl[i]);
	}
	return nbl;
}

function
unchooseBoard(bl, list)
{
	var nbl = new Array();
	var len = list.length;
	var i, j;
	for(i=0;i<bl.length;i++) {
		for(j=0;j<len;j++) if(list[j]==bl[i].board) break;
		if(j==len)
			nbl.push(bl[i]);
	}
	return nbl;
}

function
replaceGreek(str)
{
	return str.replace(/\\Lambda/g,'Λ').replace(/\\gamma/g,'γ')
	.replace(/\\delta/g,'δ').replace(/\\Upsilon/g,'Υ').replace(/\\Sigma/g,'Σ')
	.replace(/\\pm/g,'±').replace(/\\circ/g,'°').replace(/\\bigcirc/g,'○')
	.replace(/\\ast/g, '*');
}

function
NoFontMessage()
{
	document.writeln('<CENTER><DIV STYLE="padding: 10; border-style: solid; border-width:3;'
	+' border-color: #DD0000; background-color: #FFF8F8; width: 75%; text-align: left">'
	+'<SMALL><FONT COLOR="#AA0000"><B>提示:</B>\n'
	+'没能在您的机器上找到 TeX 数学字体，因此本页的公式可能看起来并不正确/美观。'
	+'请下载安装 <a href=http://www.math.union.edu/~dpvc/jsMath/download/TeX-fonts.zip>TeX-fonts.zip</a>，'
	+'或者到 <A HREF="http://www.math.union.edu/locate/jsMath/" TARGET="_blank">jsMath Home Page</a> 查看详细信息。'
	+'</FONT></SMALL></DIV></CENTER><p><HR><p>');
}

function
showdigest(maxitem, showowner) {
	var step;
	var start;
	var total = digestlist.length - 1;
	var count = 0;
	var max = maxitem == 0 ? total : maxitem;
	var i;
	
	if (maxitem != 0) {
		max = maxitem;
		step = parseInt(total / max);
		step = ((step == 0)?1:step);
		start = (total > max) ? (parseInt(0.5 + (total - step * max) * Math.random())) : 0;
	} else {
		max = total;
		step = 1;
		start = 0;
	}
	document.write("<table width=100%>");
	for (i = 0; i < max; i++) {
		if (i * step + start < total) {
			if (i % 2 - 1)
				document.write("<tr>");
			document.write("<td width=50%>");
			showdigestitem(digestlist[i * step + start], showowner);
			count++;
			if (i % 2)
				document.write("</tr>");
		}
		if (count == total) {
			break;
		}
	}
	document.write("</table>");
	return;
}

function
showdigestitem(item, showowner) {
	var sTitle, iCount, i, strTemp, cuttitle;
	iCount = 0;
	sTitle = item.title.split("");
	for (i = 0; i < sTitle.length; i++) {
		strTemp = escape(sTitle[i]);
		if (strTemp.indexOf("%u", 0) == -1)
			iCount = iCount + 1;
		else
			iCount = iCount + 2;
	}
	cuttitle = iCount > 26 ? item.title.substring(0, 18) + "..." : item.title;
	document.write("· <a href=con_"+(item.bnum - 1)+"_M."+item.ftime+".A.htm>"+cuttitle+"</a> ");
	document.write("&lt;<a href=home?B="+(item.bnum - 1)+"><font class=blk>"+item.bname+"</font></a>&gt;");
	if (showowner)
		document.write(" [<a href=qry?U="+item.owner+">"+item.owner+"</a>]");
	return;
}

var links = new Array();
function aLink(sitename, url, logo) {
	this.sitename = sitename;
	this.url = url;
	this.logo = logo;
}

function printExLinks(bidx, num) {
	var lineNum, i, hasLogo;

	if(num == 0)
		return;
	lineNum = Math.max(Math.floor(screen.width / 150.0), 4)

	for(i = 1; i < links.length; i++) {
		if(links[i].url.indexOf("http://") == -1)
			links[i].url = "http://" + links[i].url;
		document.write("<a href=" + links[i].url + " target=_blank>");
		if(links[i].logo == "none")
			document.write(links[i].sitename + "</a>");
		else
			document.write("<img src=icon?B=" + bidx + "&L=" + links[i].logo + " title=" + links[i].sitename + " id='exlnk" + i + "' onLoad='script:(this.width=(this.width>120)?120:this.width)' border=0></a>");
		if(i % lineNum == 0)
			document.write("<br><br>");
		else
			document.write("&nbsp;&nbsp;&nbsp;&nbsp;");
	}
	if(num < links.length) {
		document.write("<br><a href=\"javascript:printExLinks(" + bidx + ", 9999);\" target=_self>更多链接...</a>");
		for(i = num + 1; i < links.length; i++)
			document.getElementById("exlnk" + i).style.display = 'none';
	}
	return;
}

function open_manop(sum, has_prev_page, has_next_page, mode) {
	var i;
	document.getElementById('manop').style.display = '';
	for (i = 0; i < sum; i++)
		document.getElementById("manop" + i).style.display = '';
	if (mode == 0)
		open_manop_href("mode1");
	else if (mode == 1)
		open_manop_href("mode0");
	if (has_prev_page) {
		open_manop_href("go_first_page");
		open_manop_href("go_prev_page");
	}
	if (has_next_page) {
		open_manop_href("go_next_page");
		open_manop_href("go_end_page");
	}
	return;
}

function shut_manop(sum, has_prev_page, has_next_page, mode) {
	var i;
	document.getElementById('manop').style.display = 'none';
	for (i = 0; i < sum; i++)
		document.getElementById("manop" + i).style.display = 'none';
	if (mode == 0)
		shut_manop_href("mode1");
	else if (mode == 1)
		shut_manop_href("mode0");
	if (has_prev_page) {
		shut_manop_href("go_first_page");
		shut_manop_href("go_prev_page");
	}
	if (has_next_page) {
		shut_manop_href("go_next_page");
		shut_manop_href("go_end_page");
	}
	return;
}

function open_manop_href(id) {
	var href;
	href = 	document.getElementById(id).href;
	if (href.substr(href.length - 8, href.length) != "&manop=1")
		document.getElementById(id).href = href + "&manop=1";
}

function shut_manop_href(id) {
	var href;
	href = document.getElementById(id).href;
	while (href.substr(href.length - 8, href.length) == "&manop=1")
		href = href.substr(0, href.length - 8);
	document.getElementById(id).href = href;
	return;
}

function switch_manop(sum, has_prev_page, has_next_page, mode) {
	if (document.getElementById("manop").style.display != '')
		open_manop(sum, has_prev_page, has_next_page, mode);
	else
		shut_manop(sum, has_prev_page, has_next_page, mode);
	return;
}

function doc_show_title(sec_str, board_title, bnum, bms_1, sec_title, board_name, 
	bms_2, ban,bmhelp) {
	document.write("<table width=100% class=doc_title_table style='table-layout:\
		fixed'><tr valign=center><td width=33%>\
		<div style='overflow: hidden; \
		white-space: nowrap;'><a href=boa?secstr="+sec_str+
		"class=blu>"+sec_title+"</a> - <a href=home?B="+bnum+" class=blu>"
		+board_name+"版</a> [<a href=bbsmybrd?mode=1>预定管理</a>]<br>"
		+bmhelp+
    "\</div></td><td class=doc_title_td_title\
		align=center width=34%>"+board_title+"</td><td align=right>\
		<div style='overflow: hidden; \
		white-space: nowrap;'>版主 [ "
		+(bms_1 == "" ? "暂无" : bms_1)+" ]");
	if (ban)
		document.write("<a href=bbsshowfile?F=banAlert \
		class=red>"+(ban == 1 ? "!!!" : "<b>!!!</b>")+"</a>");
	if (bms_2 != "")
		document.write("<br>小版主 [ "+bms_2+" ]");
	document.write("</div></td></tr></table>\n");
}

function doc_show_head(mode, has_bm_perm, aboard, firstnum, anowt) {
 	/*doc information init*/
	bard = aboard;
	num = 0;
	nowt = anowt;
	today = new Date((nowt + tzdiff)*1000);
	
	/*print doc head*/
	if (has_bm_perm)
		width = new Array('8%', '7%', '12%', '8%', '40%', '7%', '18%');
	else
		width = new Array('8%', '7%', '12%', '8%', '58%', '7%');
	document.write("<table width=100% cellspacing=0 \
		cellpadding=2 style='table-layout: fixed;'>\n");
	print_hr();
	document.write("<tr align=center class=doc_head_tr>\
		<td width="+width[0]+" align=center>"
		+(mode == 0 ? "" : "原")+"序号</td> \
		<td width="+width[1]+" align=center>状态</td> \
		<td width="+width[2]+">作者</td>\
		<td width="+width[3]+">时间</td>\
		<td width="+width[4]+">标题("
		+(mode == 0 ? "字数" : "回贴/未读回贴")+")</td>\
		<td width="+width[5]+">"+"推荐"+"</td>");
	if (has_bm_perm)
		document.write("<td width="+width[6]+" id=manop \
			style='display: none;'>管理操作</td></tr>\n");
	return;
}

function doc_show_item(mode, flag, noreply, author, 
	fname, edt, title, size, star, neval, manager, 
	bnum, start, reply, unread, fnum, direction, thread)
{
	var imgsrc;

	if (neval > 5 || star > 2)
		imgsrc = "/images/hot.gif";
	else if (!(title.substr(0, 4) == 'Re: ' || title.substr(0, 4) == 're: '
			|| title.substr(0, 4) == 'rE: ' 
			|| title.substr(0, 4) == 'RE: ')) {
		if (noreply)
			imgsrc = "/images/lock_folder.gif";
		else
			imgsrc = "/images/folder.gif";
	}
	else if (noreply)
		imgsrc = "/images/lock_text.gif";
	else
		imgsrc = "/images/text.gif";
	document.write("<tr class=doc_item_tr><td align=center>"+fnum+"</td>");
// just for test
	document.write("<td align=center><font title=\"%s\"");
	if(flag.charAt(0) == "N")
		document.write("<font color=#333333>"
			+replaceString(" ", "&nbsp;", flag)+"</font></td>");
	else
		document.write(""+replaceString(" ", "&nbsp;", flag)+"</td>");
	if(author.indexOf('.')==-1)
		document.write("<td><a href=qry?U="+author+" title='查询 "+
		author+" 的资料'>"+author+"</a></td>");
	else
		document.write("<td>"+author+"</td>");
	document.write("<td align=center><nobr>"+shortFDate(fname)+"</nobr></td>");
	document.write("<td class=doc_item_td_title><div style='overflow: hidden; \
		white-space: nowrap;'>");
	if (mode == 0) {
		document.write("<a href='tcon_"+bnum+"_"+thread+".htm?start="+
			fnum+"' title='"+
			titleStr(title)+"'><img src="+imgsrc+" alt='' border=0></a>  \
			<a href='con_"+bnum+"_"+fname
			+".htm' title='"+titleStr(title)+"'>"
			+titleStr(title)+"</a>");
		document.write(sizeStr(size)+"</div></td> \
			<td align=center>"+star+"</td>");
	} else if (mode == 1) {
		document.write("<a href='tcon_"+bnum+"_"+thread+".htm?start="+
			(fnum - 1)+"' title='"+titleStr(title)+
			"'><img src="+imgsrc+" alt='' border=0></a> \
			<a href=bbsnt?B="+bnum+"&start="+fnum
			+"&th="+fname2ftime(fname)+" title='"+titleStr(title)+"'>"
			+titleStr(title)+"</a>");
		document.write("("+reply+"/"+
			(unread == 0 ? unread : "<font class=red>"+unread+"</font>")
			+")</div></td> \
			<td align=center>"+star+"</td>");
	}
	if(manager) {
		document.write("<td align=center id='manop"+num+"' \
			style='display: none;'>\
			<a href=bbsman?B="+bnum+"&S="+start+"&D="+direction+"&M="+mode+"&");
		document.write("F="+fname+"&O=1 title=\"标记到次日删除列表，用于不宜立即删除的水文\">水</a> ");
		document.write("<a href=bbsman?B="+bnum+"&S="+start+
			"&D="+direction+"&M="+mode+"&F="+fname+"&O=2 title=\"精华文章版面保留（并不是加入精华区）\">精</a> ");
		document.write("<a href=bbsman?B="+bnum+"&S="+start+
			"&D="+direction+"&M="+mode+"&F="+fname+"&O=3 title=\"加入本版文摘区\">摘</a> ");
		document.write("<a href=bbsman?B="+bnum+"&S="+start+
			"&D="+direction+"&M="+mode+"&F="+fname+"&O=4 title=\"禁止回复\">锁</a> ");
		document.write("<a href=bbsman?B="+bnum+"&S="+start+
			"&D="+direction+"&M="+mode+"&F="+fname+"&O=5 title=\"清除所有标记\">清</a> ");
		document.write("<a href=bbsdenyadd?B="+bnum+"&u="+author+" title=\"封禁文章作者\">封</a></td>");
	}
	document.write("</tr>");
	num++;
}

function doc_show_end() {
	document.write("</table>\n");
	print_hr();
}

function doc_show_top_item(author, time, bnum, fname, title, size) {
	document.write("<tr class=doc_top_tr><td align=center>[提示]</td>\
		<td></td><td>"+author+"</td><td align=center>"+time+"</td>\
		<td><div style='overflow: hidden; \
		white-space: nowrap;'><a href=con_"+bnum+"_"+fname+".htm>"
		+titleStr(title)+"</a>"+sizeStr(size)+"</div></td></tr>");
	return;
}

function fname2ftime (fname) {
	ftime = fname.split('.');
	return ftime[1];
}

function print_hr() {
	document.write("<hr class=doc_hr> \n");
}

function doc_show_nav(mode, bnum, hastmpl, hasbmperm, start, last, 
		pagesize, has_prev_page, has_next_page, smagic) {
	document.write("<table align=center cellspaing=0 \
		cellpadding=2px border=0><tr valign=bottom>");
	document.write("<td>[ ");
	if (mode == 0)
		document.write("一般模式 <a id='mode1' href=doc?B="+bnum+"&M=1>主题模式</a> ");
	else if (mode == 1)
		document.write("<a id='mode0' href=doc?B="+bnum+"&M=0>\
		一般模式</a> 主题模式 ");
	if (hasbmperm)
		document.write("<a href=# onclick='switch_manop("+num+","
			+has_prev_page+","+has_next_page+","+mode+");'> 管理模式</a> ");
	document.write("]</td>");
	document.write("<td><a href=pst?B="+bnum+" class=red>发表文章</a></td>");
	//document.write("<td><a href='http://yjrg.welan.cn' target=_blank class=red>定购糊涂站衫</a></td>");
	if (hastmpl)
		document.write("<td><a href=psttmpl?B="+bnum+"&action=view class=red> \
			模板发文</a></td>");
	else if (hasbmperm)
		document.write("<td><a href=psttmpl?B="+bnum+"&action=view class=red> \
			创建模板</a></td>");
	document.write("<td><a href=bfind?B="+bnum+">搜索</a></td>");
	document.write("<td><a href=clear?B="+bnum+"&S="+start+">清除未读</a></td>");
	document.write("<td><a href=# onclick='location=location; return;' class=blu>\
		刷新</a></td></tr></table>");
	document.write("<table align=center cellspacing=0 cellpading=0 border=0> \
		<tr><td><a href='/"+smagic+"/bbsrss?rssid="+bnum
		+"&m="+mode+"' \
		target='_blank'><img src='/rss.gif' border='0'></a></td>\
		<form name=docform1 method=post action=doc> \
		<td><input type=hidden name=B value="+bnum+"> \
		<input type=hidden name=M value="+mode+"> \
		<input type=hidden name=D value=1> \
		<input type=submit value='转到'> \
		<input type=text name=S size=4 \
		value='篇号' onfocus=\"this.value=''\"></td></form>");
	document.write("<form name=docform2 method=post action=doc> \
		<td><input type=submit value='转到'> \
		<input type=text name=B size=6 \
		value='讨论区' onfocus=\"this.value=''\"></td></form><td>");
	if (mode == 0) {
		if (!has_prev_page)
			document.write("首页 上一页 ");
		else {
			document.write("<a id=go_first_page href=doc?B="+bnum+
				"&M=0&S=1&D=1>首页</a> ");
			document.write("<a id=go_prev_page href=doc?B="+bnum+
				"&M=0&S="+(start - pagesize)+">上一页</a> ");
		}
		if (!has_next_page)
			document.write("下一页 末页");
		else {
			document.write("<a id=go_next_page href=doc?B="+bnum+
				"&M=0&S="+(start+pagesize)+">下一页</a> ");
			document.write("<a id=go_end_page href=doc?B="+bnum+
				"&M=0>末页</a>");
		}
	} else if (mode == 1) {
		if (!has_prev_page)
			document.write("首页 上一页 ");
		else {
			document.write("<a id=go_first_page href=doc?B="+bnum+
				"&M=1&S=1&D=1>首页</a> ");
			document.write("<a id=go_prev_page href=doc?B="+bnum+
				"&M=1&S="+start+"&D=-1>上一页</a> ");
		}
		if (!has_next_page)
			document.write("下一页 末页");
		else {
			document.write("<a id=go_next_page href=doc?B="+bnum+
				"&M=1&S="+last+"&D=1>下一页</a> ");
			document.write("<a id=go_end_page href=doc?B="+bnum+
				"&M=1>末页</a>");
		}		
	}
	document.write("</td></tr></table>");
	return;
}


