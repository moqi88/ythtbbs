function openlog()
{
	open('bbslform','','left=255,top=190,width=130,height=100');
}
function eva(board, file) {
var s;
	s=" [<a href='eva?B="+board +"&amp;F="+file+"&amp;star=";
	document.writeln("喜歡這個文章麼? 這個文章"+s+"1'>不錯</a>]"+s+"3'>很好</a>]"+s+"5'>我頂!</a>]");
}
function docform(a, b){
	document.writeln("<table border=0 cellspacing=0 cellpading=0><tr><td><form name=docform1 method=post action="+a+"><a href="+a+"?B="+b+"&S=1>第一頁</a> <a href="+a+"?B="+b+"&S=0>最後一頁</a> <input type=hidden name=B value="+b+"><input type=submit value=轉到>第<input type=text name=start size=4>篇</form></td><td><form name=docform2 method=post action="+a+"><input type=submit value=轉到><input type=text name=B size=7>討論區</form></td></tr></table>");
}
//tabs顯示版面選項卡
function atab(cgi, highlight, str)
{
	var tdclass, lnkclass;
	tdclass="sec1";
	lnkclass="blu";
	if(highlight){
		tdclass="sec2";
		lnkclass="whi";
	}
	document.writeln("<td class="+tdclass+" align=center><a href="+cgi+" class="+lnkclass+
		"><nobr>"+str+"</a></td>");
}

function tabs(board, num, hasindexpage, hasbacknumber, ancpath, infostr, inboard, hasvote)
{
	document.writeln("<table width=100% cellSpacing=0 border=0 cellPadding=0><tr><td valign=bottom><table width=100% border=0 cellspacing=0 cellpadding=3><tr><td class=sec0>&nbsp;</td>");
	if(hasindexpage!=0)
		atab("home?B="+board, false, "進版頁面");
	atab("not?B="+board, num==1, "備忘錄");
	atab("doc?B="+board, num==3, "討論區");
	atab("gdoc?B="+board, num==4, "文摘區");
	if(hasbacknumber!=0)
		atab("bknsel?B="+board, num==5, "過刊區");
	atab("0an?path="+ancpath, num==6, "精華區");
	document.writeln("</td></tr></table></td><td width=80% align=right class=sec0 valign=bottom>");
	document.writeln(""+infostr+" 在線"+inboard+"人");
	if(hasvote)
		document.writeln(" <a href=vote?B="+board+" class=red>投票中!</a> ");
	document.writeln("</td></tr>"+
		"<TR class=hrcolor><TD colspan=2><IMG height=2 src=/small.gif width=1></TD></TR></table>");
}

function phr()
{
	document.writeln("<table width=100% cellspacing=1 cellpadding=0><tr class=hrcolor><td><img height=1 src=/small.gif width=1></td></tr></table>");
}

function phrw()
{
	document.writeln("<table width=100% cellspacing=1 cellpadding=0><tr><td><img height=1 src=/small.gif width=1></td></tr></table>");
}

//打印文章列表條目
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
	return "○ "+title;
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

function docItem(flag, noreply, author, fname, edt, title, size, star, neval, manager, bnum, start)
{
	var imgsrc;
	if (neval > 5 || star > 2)
		imgsrc = "/images/hot.gif";
	else if (!(title.substr(0, 4) == 'Re: ' || title.substr(0, 4) == 're: '
			|| title.substr(0, 4) == 'rE: ' || title.substr(0, 4) == 'RE: ')) {
		if (noreply)
			imgsrc = "/images/lock_folder.gif";
		else
			imgsrc = "/images/folder.gif";
	}
	else if (noreply)
		imgsrc = "/images/lock_text.gif";
	else
		imgsrc = "/images/text.gif";
	document.write("<tr><td align=center>"+num+"</td>");
	document.write("<td align=center>");
	if(flag.charAt(0) == "N")
		document.write("<font color=#333333>"+replaceString(" ", "&nbsp;", flag)+"</font></td>");
	else
		document.write(""+replaceString(" ", "&nbsp;", flag)+"</td>");
	if(author.indexOf('.')==-1)
		document.write("<td><a href=qry?U="+author+" >"+author+"</a></td>");
	else
		document.write("<td>"+author+"</td>");
	document.write("<td align=center><nobr>"+shortFDate(fname)+"</nobr></td>");
	document.write("<td><div style='overflow: hidden; text-overflow: ellipsis; white-space: nowrap;'>");
	document.write("<img src="+imgsrc+" alt=''>  <a href=con_"+board+"_"
		+fname
		+".htm?N="+num+"&T="+edt +">"+titleStr(title)+"</a>"+sizeStr(size)+"</div></td>");
	document.write("<td align=center>"+evaStr(star, neval)+"</td>");
	if(manager) {
		document.write("<td align=center id='manop"+num+"' style='display: none;'><a href=bbsdoc?B="+bnum+"&S="+start+"&m=1&mode=1&");
		document.write("fname="+fname+">X</a> ");
		document.write("<a href=bbsdoc?B="+bnum+"&S="+start+"&m=1&mode=2&");
		document.write("fname="+fname+">M</a> ");
		document.write("<a href=bbsdoc?B="+bnum+"&S="+start+"&m=1&mode=3&");
		document.write("fname="+fname+">G</a> ");
		document.write("<a href=bbsdoc?B="+bnum+"&S="+start+"&m=1&mode=4&");
		document.write("fname="+fname+">鎖</a> ");
		document.write("<a href=bbsdoc?B="+bnum+"&S="+start+"&m=1&mode=5&");
		document.write("fname="+fname+">清</a> ");
		document.write("<a href=bbsdenyadd?B="+bnum+"&u="+author+">封</a></td>");
	}
	document.write("</tr>");
	num = num +1;
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
	function gt(s, s1, s2) {
		return '<'+s1;
	}
	function url(s, s1, s2) {
		var pos = s2.indexOf(' src'), tmppos;
		var url, newstr, tmpstr, tmppattern;
		if (pos == -1)
			return s;
		url = s2.substring(pos, s2.length);
		pos = url.indexOf('http');
		if (pos == -1)
			return s;
		url = url.substring(pos, s2.length);
		url = url.replace(/([^\s\"\']*)([\s\"\'>].*)/, '$1');
		newstr = '<a href="' + url + '" target="_blank">';
		newstr +='<img src="' + url + '" border="0" ';
		newstr +='onload="con_resize(this);"></a>';
		newstr +='<br><div style="display: none;">';
		newstr +='<font color=green>外部圖片: </font>'; 
		newstr += url + "<br></div>";
		return newstr;
	}
	pattern1 = /<([^><]*)(\.width>)/i;
	pattern2 = /<(\s*)(\w*)([^>]*)>/gi;
	pattern3 = /<(\s*)(\/\w*)([^>]*)>/gi;
	while(pattern1.test(str)) {
		str = str.replace(pattern1, gt);
	}
	str = str.replace(pattern2, tolower);
	str = str.replace(pattern3, tolower);
	str = str.replace(/<(img|input)([^>]*)>/gi, url);
	return str;
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
//	str=str.replace(/\[cc\]([^\[]*)\[\/cc\]/g, '<embed allowScriptAccess=always src="http://union.bokecc.com/$1" width=438 height=387>');
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

//新導讀
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
		return '<div class=gry>封閉版面</div>';
	if(bl.isclose==2)
		retv+='<div class=gry>只讀版面</div>';
	for(var i=0;i<lm.length;i++)
		retv+='<li><a href=bbsnt?B='+bl.bnum+'&th='+lm[i].th+' '
		+(lm[i].marked?'class=gre ':'')+'title='+lm[i].title+'>'+lm[i].title+'</a>'
		 +' [<a href=qry?U='+lm[i].author+'>'+lm[i].author+'</a>]</li>';
	if(retv.length==0)
		return '目前沒有熱點或被標記的文章';
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
	else if(bl.lastmark.length>4)
			a='<br><br>';
	retv+="<div class\"clear\"></div>\
		<div class=\"boardlist_box\">\
		<div class=\"boardlist_box_left\">\
		<div class=\"boardlist_box_left_padding\">\
		<h3><a name=\""+bl.board+"\" title=\""+bl.title+"\" \
		href=\"home?B="+bl.bnum+"\">"+bl.title+" ["+
		bl.board+"]</a></h3>\
		<div align=\"center\">\
		<a href=\"home?B="+bl.bnum+"\" title=\""+bl.title+"\">\
		<img border=0 alt=\"版面圖標\" src=\""+icon+"\"></a>\
		</div>"+a+"\
		<li>人　氣﹕"+bl.score+"</li>\
		<li>文章數﹕"+bl.total+"</li>\
		<li>版　主﹕"+bl.bm+"</li>\
		</div></div>\
		<div class=\"boardlist_box_right\">\
		<div class=\"boardlist_box_right_padding\">";
	if (bl.intro != '')
		retv += "<div class=\"boardlist_box_intro\">"
			+bl.intro+"</div>"
	retv += lm2str(bl)+"</div></div></div>";
	return retv;
}

function
fullBoardList(bl)
{
	bl.sort(cmpBoardScore);
	for(var i = 0; i < bl.length; i++)
		document.write(oneBoard(bl[i]));
	return;
}

function
boardIndex(bl)
{
	bl.sort(cmpBoardName);
	for(var i = 0; i < bl.length; i++) {
		document.write("<li><a href=#"+bl[i].board+
			" title=\"跳到"+bl[i].board+"所在的位置\">"+bl[i].board+"</a></li>");
	}
	return;
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

function alm(th,title,au,marked)
{
	this.title=aStr(title);
	this.th=th;
	this.author=au;
	this.marked=marked
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
	+'沒能在您的機器上找到 TeX 數學字體﹐因此本頁的公式可能看起來並不正確/美觀。'
	+'請下載安裝 <a href=http://www.math.union.edu/~dpvc/jsMath/download/TeX-fonts.zip>TeX-fonts.zip</a>﹐'
	+'或者到 <A HREF="http://www.math.union.edu/locate/jsMath/" TARGET="_blank">jsMath Home Page</a> 查看詳細信息。'
	+'</FONT></SMALL></DIV></CENTER><p><HR><p>');
}

function check_manop(search, start, pagesize) {
	var i;
	var parm = new Array();
	manopstart = start;
	manoppagesize = pagesize;
	parm = search.substring(1).split('&');
	for (i = 0; i < parm.length; i++) {
		if (parm[i] == 'manop=1') {
			open_manop();
			return;
		}
	}
	return;
}

function open_manop() {
	var i;
	document.getElementById('manop').style.display = '';
	for (i = manopstart; i < manopstart + manoppagesize; i++)
		document.getElementById('manop' + i).style.display = '';
	return;
}

function shut_manop() {
	var i;
	document.getElementById('manop').style.display = 'none';
	for (i = manopstart; i < manopstart + manoppagesize; i++)
		document.getElementById('manop' + i).style.display = 'none';
	return;
}

function switch_manop() {
	if (document.getElementById('manop').style.display == '') 
		shut_manop();
	else
		open_manop();
	return;
}

function status_manop(hasbmperm) {
	if (!hasbmperm)
		return 0;
	if (document.getElementById('manop').style.display == '')
		return 1;
	else
		return 0;
}

function showdocnav(mode, bnum, hastmpl, hasbmperm, start, pagesize, total) {
	document.write("<table align=center cellspaing=0 cellpadding=2px border=0><tr valign=bottom>");
	document.write("<td>[ ");
	if (mode == 0)
		document.write("一般模式 <a href=tdoc?B="+bnum+">主題模式</a> ");
	else
		document.write("<a href=doc?B="+bnum+">一般模式</a> 主題模式 ");
	if (hasbmperm)
		document.write("<a href=# onclick=switch_manop()> 管理模式</a> ");
	document.write("]</td>");
	//document.write("<td><a href=home?B=Tshirt class=red>糊塗站衫</a></td>");
	document.write("<td><a href=pst?B="+bnum+" class=red>發表文章</a></td>");
	if (hastmpl)
		document.write("<td><a href=psttmpl?B="+bnum+"&action=view class=red> \
			模板發文</a></td>");
	else if (hasbmperm)
		document.write("<td><a href=psttmpl?B="+bnum+"&action=view class=red> \
			創建模板</a></td>");
	document.write("<td><a href=bfind?B="+bnum+">搜索</a></td>");
	document.write("<td><a href=clear?B="+bnum+"&S="+start+">清除未讀</a></td>");
	document.write("<td><a href=# onclick='location=location; return false;' class=blu>\
		刷新</a></td></tr></table>");
	document.write("<table align=center cellspacing=0 cellpading=0 border=0> \
		<tr><form name=docform1 method=post action="+(mode==0?'doc':'tdoc')+"> \
		<td><input type=hidden name=B value="+bnum+"> \
		<input type=submit value='轉到'> \
		<input type=text name=start size=4 \
		value='篇號' onfocus=\"this.value=''\"></td></form>");
	document.write("<form name=docform2 method=post action="+(mode==0?'doc':'tdoc')+"> \
		<td><input type=submit value='轉到'> \
		<input type=text name=B size=6 \
		value='討論區' onfocus=\"this.value=''\"></td></form>");
	document.write("<td><a href=# onclick=doc_go_begin("+mode+","+bnum+","+hasbmperm+")>首頁</a> ");
	if (mode == 0 && start <= 1)
		document.write("上一頁 ");
	else
		document.write("<a href=# onclick=\
			doc_go_prev("+mode+","+bnum+","+start+","+pagesize+","+hasbmperm+")>上一頁</a> ");
	if (mode == 0 && start < total - pagesize + 1)
		document.write("<a href=# onclick=\
			doc_go_next("+mode+","+bnum+","+start+","+pagesize+","+total+","+hasbmperm+")>下一頁</a> ");
	else
		document.write("下一頁 ");
	document.write("<a href=# onclick=doc_go_end("+mode+","+bnum+","+hasbmperm+")>末頁</a></td>");
	document.write("</td></tr></table>");
	return;
}

function doc_go_prev(mode, bnum, start, pagesize, hasbmperm) {
	var manop, newstart;
	
	manop = status_manop(hasbmperm);
	newstart = start - pagesize <= 0 ? 1 : start - pagesize;
	
	if (mode == 0 && start > 1)
		location.replace('doc?B='+bnum+'&S='+newstart+'&manop='+manop);
	else
		alert("已經是第一頁了。");
	return;
}

function doc_go_next(mode, bnum, start, pagesize, total, hasbmperm) {
	var manop, newstart;
	
	manop = status_manop(hasbmperm);
	newstart = start + pagesize;

	if (mode == 0 && start < total - pagesize + 1)
		location.replace('doc?B='+bnum+'&S='+newstart+'&manop='+manop);
	else
		alert("已經是最後一頁了。");
	return;
}

function doc_go_begin(mode, bnum, hasbmperm) {
	var manop;

	manop = status_manop(hasbmperm);
	
	if (mode == 0)
		location.replace('doc?B='+bnum+'&S=1&manop='+manop);
	else
		alert("已經是第一頁了");

	return;
}

function doc_go_end(mode, bnum, hasbmperm) {
	var manop;

	manop = status_manop(hasbmperm);

	if (mode == 0)
		location.replace('doc?B='+bnum+'&S=0&manop='+manop);
	else
		alert("已經是最後一頁了");

	return;
}


function checkFrame(redirect)
{
	if(self.location!=top.location) return;
	var d=document.location.href;
	var temp=d.substring(0,d.lastIndexOf('?'));
	// 注釋掉 for SEO 流量   Do not know why use this function....

	if (!redirect)
	//修改了框架﹐去掉了顯示完整界面
//		document.writeln("<a href=bbsindex?t=1&b="+d.substring(temp.lastIndexOf('/')+1, d.length)+">顯示完整界面</a> ");
		;
	else
		this.location.replace('bbsindex?t=1&b='+d.substring(temp.lastIndexOf('/')+1, d.length));
}

function bbspst_ctrlenter(event) {
	if(event.ctrlKey && event.keyCode == 13) {
		document.getElementById('bbspst_button_submit').click();
		event.returnValue = false;
		event.preventDefault();
	}
}

function adv_resize(obj, pos) {
	if (obj.width > document.body.offsetWidth)
		obj.width = document.body.offsetWidth * .9;
	if (obj.height > 60)
		obj.height = 60;
}

function bbspst_insertBokeccUrl(url) {
	var obj = document.getElementById('rte1').contentWindow;

	if (!obj)
		return;

	url += '<br /><div style="display: none;"><font underline>';
	url += '這裡有一個視頻﹐請用WEB方式觀看。</font></div>';
	
	obj.document.body.innerHTML += url;
}
