function getCookie(name)
{
	var cookie=document.cookie;
	var index=cookie.indexOf(name + "=");
	if(index==-1)return null;
	index=cookie.indexOf("=",index)+1;
	var endstr=cookie.indexOf(";", index);
	if (endstr==-1)endstr=cookie.length;
	return unescape(cookie.substring(index, endstr));
}

function SetCookie (name, value) {  
	var argv = SetCookie.arguments;  
	var argc = SetCookie.arguments.length;  
	var expireSec = (argc > 2) ? argv[2] : null;  
	var path = (argc > 3) ? argv[3] : null;  
	var domain = (argc > 4) ? argv[4] : null;  
	var secure = (argc > 5) ? argv[5] : false;  
	var expires = null;
	if(expireSec!=null)
		expires=new Date(today.getTime()+1000*expireSec);
	document.cookie = name + "=" + escape (value) +
		((expires == null) ? "" : ("; expires=" + expires.toGMTString())) +
		((path == null) ? "" : ("; path=" + path)) +  
		((domain == null) ? "" : ("; domain=" + domain)) +    
		((secure == true) ? "; secure" : "");
}

function setTZOCookie()
{
	var tzo=getCookie("TZO");
	if(tzo==null) {
		tzo=(new Date().getTimezoneOffset()/60)*(-1);
		setCookie("TZO",tzo,1800);
	}
}

//setTZOCookie();

function getSec(y,m1,d)
{
	return new Date(y,m1-1,d).getTime()/1000;
}

function showCalendar(when,link,daylist)
{
	var aDate = new Date(when*1000);
	var year = aDate.getFullYear();
	var month = aDate.getMonth()+1;
	
	var i, j;
	var today=new Date();
	var thisYear=today.getFullYear();
	var thisMonth=today.getMonth()+1;
	var thisDate=today.getDate();
	var lmonth=month-1;
	var lyear=year;
	if(lmonth==0) {lmonth=12;lyear=year-1;}
	var nmonth=month+1;
	var nyear=year;
	if(nmonth==13) {nmonth=1;nyear=year+1;}
	var nnmonth=nmonth+1;
	var nnyear=nyear;
	if(nnmonth==13) {nnmonth=1;nnyear=year+1;}
	
	var str="<table><caption>";
	str+="<a href="+link+"&start="+getSec(lyear,lmonth,1)
		+"&end="+getSec(year, month, 1)+">&lt;&lt;</a> ";
	str+="<a href="+link+"&start="+getSec(year, month, 1)
		+"&end="+getSec(nyear,nmonth,1)+">"+year+"・"+month+"</a> "
	str+="<a href="+link+"&start="+getSec(nyear,nmonth,1)
		+"&end="+getSec(nnyear,nnmonth,1)+">&gt;&gt;</a></caption>";
	str+="<tr><th>日</th><th>一</th><th>二</th><th>三</th><th>四</th><th>五</th><th>六</th></tr>";

	var days = new Array(32);
	for(i=0;i<32;i++) days[i]=0;
	for(i=0;daylist.length!=null&&i<daylist.length;i++) {
		var aDate = new Date(daylist[i]*1000);
		if(aDate.getFullYear()-year!=0||aDate.getMonth()+1-month!=0)
			continue;
		days[aDate.getDate()-1]++;
	}

	var monthDays = new Array(31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31); 
	if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) monthDays[1] = 29; 
	var nDays=monthDays[month-1];
	var aDate=new Date(year,month-1,1);
	var aDay=aDate.getDay();
	var list=new Array(6*7);
	for(i=0;i<6*7;i++) list[i]=0;
	for(i=aDay;i<aDay+nDays;i++) list[i]=1;
	for(i=0;i<6&&(i==0||list[i*7]!=0);i++) {
		str+="<tr>";
		for(j=0;j<7;j++) {
			if(list[i*7+j]==0) { str+="<td>&nbsp;</td>"; continue;}
			var theDate=i*7+j-aDay+1
			s=""+theDate;
			if(thisYear==year&&thisMonth==month&&thisDate==theDate) s="<b>"+s+"</b>";
			if(days[theDate-1]!=0)
				s="<a href="+link+"&Y="+year+"&M="+month+"&D="+theDate+"><u>"+s+"</u></a>";
			str+="<td align=center>"+s+"</td>";
		}
		str+="</tr>";
	}
	document.writeln(str+"</table>");
}

function movelist(fbox, tbox, maxtbox, targetInput) {
	var arrFbox = new Array();
	var arrTbox = new Array();
	var arrLookup = new Array();
	var i;
	if(maxtbox>0&&tbox.options.length>=maxtbox)
		return;
	for (i = 0; i < tbox.options.length; i++) {
		arrLookup[tbox.options[i].text] = tbox.options[i].value;
		arrTbox[i] = tbox.options[i].text;
	}
	var fLength = 0;
	var tLength = arrTbox.length;
	for(i = 0; i < fbox.options.length; i++) {
		arrLookup[fbox.options[i].text] = fbox.options[i].value;
		if (fbox.options[i].selected && fbox.options[i].value != "") {
			arrTbox[tLength] = fbox.options[i].text;
			tLength++;
		} else {
			arrFbox[fLength] = fbox.options[i].text;
			fLength++;
		}
	}
	arrFbox.sort();
	arrTbox.sort();
	fbox.length = 0;
	tbox.length = 0;
	var c;
	for(c = 0; c < arrFbox.length; c++) {
		var no = new Option();
		no.value = arrLookup[arrFbox[c]];
		no.text = arrFbox[c];
		fbox[c] = no;
	}
	for(c = 0; c < arrTbox.length; c++) {
		var no = new Option();
		no.value = arrLookup[arrTbox[c]];
		no.text = arrTbox[c];
		tbox[c] = no;
	}
	if(targetInput!= null && maxtbox!=0) {
		var str="";
		for(c = 0; c<arrTbox.length;c++)
			str+=arrLookup[arrTbox[c]]+';';
		targetInput.value=str;
	} else if(targetInput!= null) {
		var str="";
		for(c = 0; c<arrFbox.length;c++)
			str+=arrLookup[arrFbox[c]]+';';
		targetInput.value=str;
	}
}

function check_form_condition(v1, v0, tar, msg) {
	var tid = document.getElementById(tar);

	if(tid == null)
		return false;
	if(v1 == v0) {
		tid.disabled = true;
		alert(msg);
		return false;
	}
	tid.disabled = false;
	return true;
}


function submitPostForm(form) {
	form.submit.disabled = true;
	form.submit.value = "请稍候...";
	if(form.title.value.length == 0) {
		form.submit.disabled = false;
		form.submit.value = "发表";
		alert("您还没写标题呢！");		
		return false;
	}
	updateRTE('rte1');
	if(form.rte1.value.length == 0) {
		form.submit.disabled = false;
		form.submit.value = "发表";
		alert("要以洋洋洒洒为荣，以空文灌水为耻。");
		return false;
	}	
	form.content.value=form.rte1.value;
	return true;
}

function submitCommentForm(form, anoy) {
	form.submit.disabled = true;
	form.submit.value = "请稍候...";
	if(anoy == 1) {
		if(form.by.value.length == 0 || form.email.value.length == 0) {
			form.submit.disabled = false;
			form.submit.value = "发表";
			alert("留下您的大名和Email吧。");		
			return false;
		}
	}
	updateRTE('rte1');
	if(form.rte1.value.length == 0) {
		form.submit.disabled = false;
		form.submit.value = "发表";
		alert("怎么也得有两句话吧。");
		return false;
	}	
	form.comment.value=form.rte1.value;
	return true;
}

/*
function load_bgimg(userid) {
	var div = document.getElementById("all2");
	var img_url = new String(self.location);
	var idx;

	idx = img_url.lastIndexOf("/");
	if(idx >= 0)
		img_url = img_url.slice(0, idx+1);
	else
		img_url += "/";
	img_url += "blogmedia?U=" + userid + "&mode=02&n=bg.gif";
	div.style.background = "url('" + img_url + "')";
	return true;
}
*/

function adjust_width() {
	var v = document.getElementById("all2");
	var isNav = (navigator.appName == "Netscape") ? true : false;
	var window_width = isNav ? self.innerWidth : document.body.clientWidth;
        
	if( window_width > 800) {
		v.style.left = ((window_width-800)/2) + "px";
		//v.style.position = "absolute";
	} else {     
		v.style.left = "0px";
	}
	return true;
}

/*
function load_model(model, blog) {
	var top;

	top = document.getElementById("topbar");
	top.style.background = "url('/blog/" + model + "Top.jpg') left repeat-x";
	alert("url('/blog/" + model + "Top.jpg') left repeat-x");
	document.write("<style type=\"text/css\">");
	document.write(".sideBar { background-image: url('/blog/" + model +"Bar.gif'); }");
	if(!blog) {//blogpage
		document.write("</style>");
		return false;
	}
	document.write(".posttitle { background-image: url('/blog/" + model + "Up.gif');}");
	printf("</style>\n");
	return true;
}
*/


function load_model(user) {
	var top, bg, ca;

	document.body.background = "blogmedia?U=" + user + "&M=02&n=Side.jpg";
	
	top = document.getElementById("topbar");
	if(top)
		top.style.background = "url('blogmedia?U=" + user + "&M=03&n=Top.jpg') center";

	bg = document.getElementById("all2");
	if(bg)
		bg.style.background = "url('blogmedia?U=" + user + "&M=04&n=Bg.gif')";
		
	ca = document.getElementById("Calendar");
	if(ca)
		ca.style.background = "url('blogmedia?U=" + user + "&M=08&n=Ca.gif') center";

	document.writeln("<style type=\"text/css\">");
	document.writeln(".sideBar { " +
			" background: url('blogmedia?U=" + user + "&M=07&n=Bar.gif') left; " +
			" }");
//	document.write(".Calendar { " +
//			" background-image: url('blogmedia?U=" + user + "&M=08&n=Ca.gif'); " +
//			" }" + "<br>");
	document.writeln(".posttitle { " +
			" background: url('blogmedia?U=" + user + "&M=05&n=Up.gif') no-repeat left; " +
			" }");	
	document.writeln("</style>");
	return true;
}
	


function switch_postcontent(idx, model, user) {
	var cid, tid, pc, pt;
	var custom = 15;
	
	cid = "pc" + idx;
	tid = "pt" + idx;
	pc = document.getElementById(cid);
	pt = document.getElementById(tid);
	if(pc.style.display == "none") {
		pc.style.display = "block";
		if(model == custom)
			pt.style.background = "url('blogmedia?U=" + user + "&M=05&n=Up.gif') left no-repeat";
		else
			pt.style.background = "url('/blog/" + model + "Up.gif') left no-repeat";
	} else {
		pc.style.display = "none";
		if(model == custom)
			pt.style.background = "url('blogmedia?U=" + user + "&M=06&n=Down.gif') left no-repeat";
		else
			pt.style.background = "url('/blog/" + model + "Down.gif') left no-repeat";
	}
	return false; 
}

function showBlogMypic(url, magic, user, media, pic) {
	var v_list = new Array("swf", 
				"rm", "rmvb", "3gp",
				"wmv", "mpg", "mpeg", "asf", "avi",
				"mov", "mkv");
	var a_list = new Array("mp3", "wma", "wav", "mid", "mpga");
	var i, j, hidden=true;
	var isWindows = (navigator.userAgent.indexOf("Windows") != -1)  ? true : false;
	var isIE = (navigator.appName == "Microsoft Internet Explorer") ? true : false;
	var str = "";
	var ext = url.slice(url.lastIndexOf(".")+1).toLowerCase();

	if(!media) {
		printMypic(magic, user, pic);
		return false;
	}
	for(i = 0; i < v_list.length; i++) {
		if(ext == v_list[i]) {
			hidden=false;
			break;
		}
	}
	for(j = 0; j < a_list.length; j++) {
		if(ext == a_list[j]) {
			hidden=true;
			break;
		}
	}
	if(i == 0) { //flash movie	OS: WindowsXP/Debian	Browser: IE6/firefox/mozilla
		str += "<object classid='clsid:D27CDB6E-AE6D-11cf-96B8-444553540000' " +
			"type='application/x-shockwave-flash'>" +
			"<param name='movie' value='" + url + "'>" +
			"<param name='quality' value='high'>" +
			"<embed src='" + url + "' quality='high' width=180px height=160px " +
			"autostart=true loop=true></embed></object>";
	} else if(i < 4) { //real	OS: WindowsXP(?)/Debian	Browser: IE6/firefox/mozilla
		str += "<embed src='" + url + "' type='audio/x-pn-realaudio-plugin' " +
			"autostart=true loop=true " +
			"controls='imagewindow,statusbara' width=180px height=160px></embed>";
	} else if((i < 9 || j < a_list.length) && isWindows) { //wmplayer or mplayer
		str += "<object classid='clsid:22D6F312-B0F6-11D0-94AB-0080C74C7E95' " + 
			(hidden ? "width=0px height=0px" : "width=180px height=160px") + ">" +
			"<param name='FileName' value='" + url + "'>" +
			"<param name='AutoStart' value=1>" +
			"<param name='PlayCount' value=0>" +
			"<param name='ShowControls' value=0>" +
			"<param name='ShowStatusBar' value=0>" +
			"<embed src='" + url + "' " +
			"type='application/x-mplayer2' showstatusbar=0 " +
			"showcontrols=0 showpositioncontrols=0 invokeurls=1 " +
			(hidden ? "width=0px height=0px" : "width=180px height=160px") + 
			"></embed></object>";
	} else { //other media or OS
		str += "<embed src='" + url + "' autostart=true loop=true " +
			(hidden ? "width=0px height=0px" : "width=180px height=160px") + "></embed>";
	}
	if(hidden)
		printMypic(magic, user, pic);
//	alert('#'+str+'#')
	document.write(str);
	return true;
}

function check_media_form(form) {
	var i;
	if((form.mediaurl.value == "" && form.blogmedia.value == 0)) {
		alert("您还没有设置呢！");
		return false;
	}
	if(form.as == null)
		return true;
	for(i = 0; i < form.as.length; i++) {
		if(form.as[i].checked)
			form.mode.value = "3" + form.as[i].value.toString();
	}
	return true;
}

function blogpage_useNewFace() {
	var cookie = new CJL_CookieUtil("blog", 30, "/", location.host, false);

	if (cookie)
		cookie.setSubValue("blogpage_useoldstyle", 0);

	location.replace('blogpage');
}
