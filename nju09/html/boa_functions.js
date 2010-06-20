function
init_sec_tree (num, basestr, title, sub) {
	this.num = num;
	this.basestr = basestr;
	this.title = title;
	this.sub = sub;
}

function
show_sec_head (pos) {
	var width = window.screen.width;
	var i;
	var line;
	var itemnum;

	itemnum = sectree.length - 1;
	line = 0;

	if (itemnum <= width / 100)
		line = itemnum;
	else if (width == 800) {
		for (i = 7; i > 0; i--) {
			if (itemnum % i == 0); {
				line = i;
				break;
			}
		}
	} else if (width == 1024) {
		for (i = 9; i >= 6; i--) {
			if (itemnum % i == 0) {
				line = i;
				break;
			}
		}
	} else if (width == 1280) {
		for (i = 12; i >= 8; i--) {
			if (itemnum % i == 0) {
				line = i;
				break;
			}
		}
	}
	if (line == 0) {
		line = parseInt(width / 100);
		line = line >= sectree.length / 2 ?
			parseInt(sectree.length /2) : line;
	}

	document.write("<div id=\"head\">");
	for (i = 1; i < sectree.length; i++) {
		if (i % line == 1)
			document.write("<div>");
		document.write("<a href=\"/HT/boa?secstr="
			+sectree[i - 1].basestr.substring(0, 1)
			+"\" title=\""+sectree[i - 1].title
			+"\">"+sectree[i - 1].title+"</a>");
		if (i != sectree.length - 1 && i % line == 0)
			document.write("</div>");
	}
	document.write("</div></div>")

}

//打印导读条目
function sec_mark_start(sec, sectitle)
{
	document.write("<div class=\"mark_box\">\
		<h2>"+aStr(sectitle)+"[<a href=\"/HT/boa?secstr="
		+sec+"\" title=\""+aStr(sectitle)+"\">more</a>]</h2>");
}

function sec_mark_end()
{
	document.write('</div>');
}

function sec_mark_item(board, boardtitle, thread, title)
{
	document.write("<div class=\"overflow\">\
		<li><a href=\"/HT/bbsnt?B="+board+"&th="
		+thread+"\" title=\""+aStr(title)+"\">"+aStr(title)+"</a> \
		[<a href=\"/HT/home?B="+board+"\" class=blk title=\""+aStr(boardtitle)+"\">"
		+aStr(boardtitle)+"</a>]</li></div>");
}

function sec_mark_boardlist(sec, list)
{
	var i, count = list.length/2;
	var l = "<div class=\"overflow\">";
	var boardtitle;
	for(i = 0; i < count; i++) {
		boardtitle = aStr(list[i * 2 + 1]);
		l += "[<a href=/HT/home?B="+list[i*2]+" class=blk title=\""+boardtitle+"\">"+boardtitle+"</a>] ";
	}
	l+="<a href=/HT/boa?secstr="+sec+" class=blk title=\"更多版面\">…</a></div>";
	document.write(l);
}

function aLink(sitename, url, logo) {
	this.sitename = sitename;
	this.url = url;
	this.logo = logo;
}

function reorderdigest() {
	var i, j, tmp;
	
	for (i = 0; i <digestlist.length; i++)
		eva_order_index[i] = i;
	
	for (i = digestlist.length - 1; i >= 0; i--) {
		for (j = i; j < digestlist.length - 1; j++) {
			if (digestlist[eva_order_index[j]].eva < 
				digestlist[eva_order_index[j + 1]].eva) {
				tmp = eva_order_index[j];
				eva_order_index[j] = eva_order_index[j + 1];
				eva_order_index[j + 1] = tmp;
			}
		}
	}
}

function printExLinks(bidx, num) {
	var i, hasLogo;

	if(num == 0)
		return;

	for(i = 0; i < links.length - 1; i++) {
		if(links[i].url.indexOf("http://") == -1)
			links[i].url = "http://" + links[i].url;
		if(links[i].logo == "none") {
			document.write("<li id=\"exlnk"+i+"\"><a href="
				+links[i].url+" target=_blank title=\""+links[i].sitename+"\">"
				+links[i].sitename + "</a></li>");
		} else {
			document.write("<a href="
				+links[i].url+" target=_blank title=\""+links[i].sitename+"\" \
				id=\"exlnk"+i+"\">\
				<img src=icon?B="+bidx+"&L="+links[i].logo+" title="
				+links[i].sitename+" alt=\""+links[i].sitename+"\" \
				onLoad='script:(this.width=(this.width>120)?120:this.width)'\
				border=0 style=\"display: '';\" /></a><br>");
		}
	}
	/*if(num < links.length - 1) {
		document.write("<a id=\"showalllinks\" href=\"#\" \
			onclick=\"printExlinksall();\" style=\"display: '';\">更多链接...</a>");
		for(i = num; i < links.length - 1; i++)
			document.getElementById("exlnk"+i).style.display = 'none';
	}*/
	return;
}

function
printExlinksall() {
	for (i = 0; i < links.length - 1; i++) {
		document.getElementById("exlnk"+i).style.display = '';
	}
	//document.getElementById("showalllinks").style.display = 'none';
}

function boa_hideright(value) {
	var objleft = document.getElementById('main_left');
	var objright = document.getElementById('main_right');
	var objtitle1 = document.getElementById('show_right');
	var objtitle2 = document.getElementById('hide_right');
	var cookieitem = new CJL_CookieUtil("yjrg", 14, "/", location.host, false);
	
	if (!objleft || !objright || !objtitle1 || !objtitle2)
		return;
	if (value) {
		objright.style.display = 'none';
		objleft.style.width = '100%';
		objtitle1.style.display = '';
		objtitle2.style.display = 'none';
		if (cookieitem)
			cookieitem.setSubValue("boa_hideright", 1);
	} else {
		objleft.style.width = '78%';
		objright.style.display = '';
		objtitle2.style.display = '';
		objtitle1.style.display = 'none';
		if (cookieitem)
			cookieitem.setSubValue("boa_hideright", 0);
	}
	return;
}

function boa_hideright_bycookie() {
	var cookieitem = new CJL_CookieUtil("yjrg", 14, "/", location.host, false);
	if (!cookieitem)
		return;
	boa_hideright(cookieitem.getSubValue("boa_hideright"));
	return;
}

function digest() {
	this.colum = new Array();
	this.item = new Array();

	this.addcolum = function(flag, title, sec, maxitem, order) {
		this.colum[this.colum.length] 
			= new digestcolum(flag, title, sec, maxitem, order);
	}

	this.additem = function(bnum, ftime, title, author, bname, sec, eva) {
		var i;
		var len = this.colum.length;
		for (i = 0; i < len; i++) {
			if (this.colum[i].sec.indexOf(sec) != -1)
				this.colum[i].additem(bnum, ftime, title, 
						author, bname, sec, eva);
			else
				continue;
		}
	}

	this.addheadline = function(flag, title, content, bnum, ftime) {
		var i;
		var len = this.colum.length;
		for(i = 0; i < len; i++) {
			if (this.colum[i].flag != flag)
				continue;
			this.colum[i].addheadline(title, content, bnum, ftime);
			break;
		}
	}

	this.show = function (flag, showauthor, showmore, 
			showeva, showall, smagic) {
		var i;
		var len = this.colum.length;
		for (i = 0; i < len; i++) {
			if (this.colum[i].flag != flag)
				continue;
			this.colum[i].show(showauthor, showmore, 
					showeva, showall, smagic);
			break;
		}
	}

	this.showdefault= function(flag, sitename, smagic) {
		var i;
		var retv = 0;
		var len = this.colum.length;

		document.write('<h1>' + sitename + '推荐文摘</h1>\n\
				<div id="digest">\n<div id="digest_left">');
		if (len == 0) {
			document.write('')
			return;
		}
		if (flag == '#')
			flag = this.colum[0].flag;
		for (i = 0; i < len; i++) {
			if (this.colum[i].flag != flag)
				continue;
			retv = 1;
			this.colum[i].show(1, 0, 1, 1, smagic);
		}
		if (!retv)
			document.write('没有找到相应分区的文摘');
		document.write('</div><div id="digest_right">\n');
		for (i = 0; i < len; i++) {
			if (this.colum[i].flag == flag)
				continue;
			this.colum[i].show(1, 1, 1, 0, smagic);
		}
		document.write('</div></div>');
	}

	this.autofit = function(boxname, showauthor, showeva, showall) {
		var i;
		var len = this.colum.length;
		for (i = 0; i < len; i++) {
			if (this.colum[i].maxitem <= 0) {
				this.colum[i].autofit(
						boxname, showauthor, showall);
				break;
			}
		}
	}
}

function digestcolum(flag, title, sec, maxitem, order, item) {
	/* This function will add a new colum			*/
	/* flagchar	Flag char of the colum			*/
	/* title	Title of the colum			*/
	/* sec		Sec for this colum			*/
	/* maxitem	Maximum items to show			*/
	/* 		maxitem <= 0: auto			*/
	/*		maxitem > 0: normal  			*/
	/* order	How to order this cloum			*/
	/*		0: by decreasing order of adding	*/
	/*		1: by evaluation star			*/
	/* for more details, see ~/wwwtmp/digestcolumn		*/
	this.flag = flag;
	this.title = title;
	this.sec = sec;
	this.maxitem = maxitem;
	this.order = order;
	this.item = new Array();
	this.hasheadline = 0;
	this.headline;
	this.displayeditems = 0;

	function __headline(title, content, bnum, ftime) {
		this.title = title;
		this.content = content;
		this.bnum = bnum;
		this.ftime = ftime;

	}

	/* bnum=board num , ftime=file creation time , title=article title , author=article author, bname= board chinese name, sec=section, eva= evaluation mark*/
	this.additem = function (bnum, ftime, title, author, bname, sec, eva) {
		var tmp;
		var retv = 1;  //retv = 1 good  retv = 0 may be same  retv = -1 must be same
		var total = this.item.length;
		
		tmp = title.substring(0, 4); 
		    // alert("0 title is "+title+"tmp is "+tmp);
		if (tmp=='[转载]')
		{
			title = title.substring(4, title.length);
			tmp = title.substring(0, 4); 
			//alert("1 title is "+title+"tmp is "+tmp);
			retv = 0;
		}
		if (tmp=='[转寄]')
		{
			title = title.substring(4, title.length);
			tmp = title.substring(0, 4);
			//alert("2 title is "+title+"tmp is "+tmp);
			retv = 0;
		}
/* Re title is ok
		tmp = title.substring(0, 4);
		if (tmp == 'Re: ' || tmp == 're: ' || 
				tmp == 'RE: ' || tmp == 'rE: ')
			title = title.substring(4, title.length);
*/
		//tmp = title.substring(0, 8);
		if (tmp == '[BLOG]')
		{
			title = title.substring(4, title.length);
			   tmp = title.substring(0, 1); 
			  // alert("3 title is "+title+"tmp is "+tmp);
			retv = 0;
		}
		while (tmp == ' ')
		{
			title = title.substring(1, title.length);
			tmp = title.substring(0, 1);
			alert("4 title is "+title+"tmp is "+tmp);
			retv = 0;
		}
		if (title.length == 0)
			title = '无标题';

//  These lines are for deleting same topics, i think it's not good enough
/*
		for (i = 0; i < total; i ++) {
			if ((this.item[i].title == title) && (retv == 0)) {
				retv = -1;
				break;
			}
		}
		if (retv == -1)
			return;
*/
//  But do not consider same articles from both Digest and Nav, We should put Nav in front of Digest in Digest.js, for Nav is much more important

		this.item[total] 
			= new digestitem(bnum, ftime, title, 
					author, bname, sec, eva);
	}

	this.reorder = function() {
		var tmpitem;
		var i, j;
		var len = this.item.length;

		for (i = 0; i < len; i++) {
			for (j = i + 1; j < len; j++) {
				if (this.item[j].neweva <= 
						this.item[i].neweva)
					continue;
				tmpitem = this.item[j];
				this.item[j] = this.item[i];
				this.item[i] = tmpitem;
			}
		}
	}

	this.addheadline = function (title, content, bnum, ftime) {
		this.hasheadline = 1;
		this.headline = new __headline(title, content, bnum, ftime);
	}

	this.showheadline = function() {
		var retv;
		var content;
		var line = 0;
		var i;
		var testReg = /([^<]*)<br>\s*<br>(.*)/i;
		if (!this.hasheadline)
			return '';
		content = this.headline.content;
		while(testReg.test(content))
			content = content.replace(testReg, '$1$2');
		retv  = '';
		retv += '<div id="digest_' + this.flag + '_detail" ';
		retv += 'class="digest_detail_box_box">';
		retv += '<div class="digest_detail_box">';
		if (this.headline.title.length) {
			retv += '<div class="digest_detail_title">';
			retv += this.headline.title + '</div>';
		}
		retv += '<div class="digest_detail_content">';
		retv += content;
		retv +='<div align="right">[';
		retv += '<a href="/HT/con_' + this.headline.bnum;
		retv += '_M.' + this.headline.ftime + '.A.htm" ';
		retv += 'title="查看全文">查看全文</a>]</div></div>';
		retv += '</div></div>';
		return retv;
	}

	this.show = function(showauthor, showmore, showeva, showall, smagic) {
		var retv;
		var len = this.item.length;
		var max = (this.maxitem > 0) ? this.maxitem : -this.maxitem;
		var step, remainder, start, num;

		if (showall)
			max = len;

		retv  = '';
		retv += '<div id="digest_' + this.flag;
		retv += '" class="digest_box">';
		retv += '<h2 id="digest_' + this.flag + '_title">';
	       	retv += this.title;
		if (showmore) {
			retv += '<span class="digest_more">';
			retv += '<a href="/HT/digest?C=' + this.flag;
			retv += '" title="更多' + this.title;
			retv += '栏目文章">[更多]</a></span>';
		}
		retv += '　<a href="/' + smagic + '/bbsrss?rssid=digest&';
		retv += 'sec=' + this.flag +'" target="_blank">';
		retv += '<img src="/rss.gif" border="0" /></a>';
		retv += '</h2>';
		if (this.hasheadline)
			retv += this.showheadline();
		if (this.item.length == 0) {
			retv += '<br /><div align="center">';
			retv += '当前栏目没有文章。';
			retv += '</div><br /></div>';
			document.write(retv);
			return;
		}
		retv += '<div id="digest_' + this.flag + '_content">';
		if (order == 0) {		//Simple order, by decreasing
			for (i = 0; i < max && i < len; i++) {
				retv += this.item[len - i - 1].show(
						showauthor, showeva);
				this.displayeditems++;
			}		
		} else if (order == 1) {	//By eva star;
			this.reorder();
			step = Math.floor(len / max);
			step = (step <= 0) ? 1 : step;
			remainder = len - max * step;
			remainder = (remainder < 0) ? 0 : remainder;
			start = remainder ? Math.floor(Math.random() 
					* remainder) : 0;
			start = (start == remainder) ? start-- : start;
			start = (start < 0) ? 0 : start;

			for (i = 0; i < max && i < len; i++) {
				num = start + i * step;
				if (num > len)
					break;
				if (num < 0)
					num = 0;
				retv += this.item[num].show(
						showauthor, showeva);
				this.displayeditems++;
			}
		}
		retv += '</div></div>';
		document.write(retv);
	}

	this.autofit = function (boxname, showauthor, showeva) {
		var boxobj = document.getElementById(boxname);
		var digestbox = document.getElementById('digest_' + this.flag);
		var digestcontent = document.getElementById(
				'digest_' + this.flag + '_content');
		var endpos;
		var digestpos;
		var itemheight;
		var i, j, len = this.item.length;

		if (!boxobj || !digestbox || !digestcontent)
			return;
		endpos =  boxobj.offsetHeight + boxobj.offsetTop;
		digestpos = digestbox.offsetTop + digestbox.offsetHeight;
		itemheight = digestcontent.offsetHeight / this.displayeditems;

		i = 0;
		while (digestpos < endpos && i < len) {
			if (endpos - digestpos < itemheight / 2)
				break;
			if (order == 1)
				j = i;
			else if (order == 0)
				j = len - i - 1;
			if (this.item[j].shown) {
				i++;
				continue;
			}
			digestcontent.innerHTML += 
				this.item[j].show(showauthor, showeva, 0);
			digestpos += itemheight;
			i++;
		}
	
	}
}

function digestitem(bnum, ftime, title, author, bname, sec, eva) {
	var now = new Date();
	var ntime = Math.floor(now.getTime());
	var day = Math.max(0, ntime / 1000 - ftime);
	this.bnum = bnum;
	this.ftime = ftime;
	this.title = title;
	this.author = author;
	this.bname = bname;
	this.sec = sec;
	this.oldeva = eva;
	// neweva calculation
	this.neweva = Math.round(eva * 72 / ( 7 + day / 3600.0 ))
//	this.neweva = Math.exp(3 / (1 + 0.7 * day / 3600 / 24)) * (eva + 1) / 4;
	this.shown = false;

	this.show = function (showauthor, showeva) {
		var retv;

		retv  = '';
		if (this.shown)
			return retv;
		retv += '<div class="overflow">';
		retv += '<li><a href="/HT/con_'+ this.bnum + '_M.';
		retv += this.ftime + '.A.htm" title="' + this.title;
		retv += '" class="blu">'+ this.title;
		retv += '</a>&lt;<a href="/HT/home?B='+ this.bnum;
		retv += '" title="' + this.bname + '" class="blk">';
		retv += this.bname + '</a>&gt;';
		if (showauthor) {
			if (this.author != 'deliver' && 
					this.author != 'Anonymous' &&
					this.author != 'post') {
				retv += '[<a href="/HT/qry?U=' + this.author; 
				retv += '" title="' + this.author;
				retv += '" class="blk">';
				retv += this.author + '</a>]';
			} else {
				retv += '<span class="blk">' + this.author;
				retv += '</span>';
			}
		}
		if (showeva) {
			retv += '[' + Math.floor(this.neweva + .5) + '/';
		       	retv += Math.floor(this.oldeva + .5) + ']';
		}
		retv += '</li></div>';
		this.shown = true;
		return retv;
	}
}
