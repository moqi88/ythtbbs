/* =================================== */
/* | Part 0: 公用 Javascript 脚本    | */
/* =================================== */


/* ----------------------------------- */
/* |Part 0.1: 初始化公用 JSON 对象   | */
/* ----------------------------------- */

var BlogDataJSON = { };
var BlogDataObj;

var BlogCateJSON = [ { "C": 1, "D": "日记" }, { "C": 2, "D": "随笔" }, { "C": 4, "D": "连载" }, { "C": 8, "D": "幽默" }, { "C": 16, "D": "星座" }, { "C": 32, "D": "旅游" }, { "C": 64, "D": "购物" }, { "C": 128, "D": "时尚" }, { "C": 256, "D": "美食" }, { "C": 512, "D": "情感" }, { "C": 1024, "D": "同性" }, { "C": 2048, "D": "健康" }, { "C": 4096, "D": "居家" }, { "C": 8192, "D": "评论" }, { "C": 16384, "D": "体育" }, { "C": 32768, "D": "游戏" }, { "C": 65536, "D": "人物" }, { "C": 131072, "D": "娱乐" }, { "C": 262144, "D": "时事" }, { "C": 524288, "D": "关注" }, { "C": 1048576, "D": "求学" }, { "C": 2097152, "D": "职场" }, { "C": 4194304, "D": "音乐" }, { "C": 8388608, "D": "动漫" }, { "C": 16777216, "D": "影视" }, { "C": 33554432, "D": "财经" }, { "C": 67108864, "D": "科技" }, { "C": 134217728, "D": "军事" }, { "C": 268435456, "D": "读书" }, { "C": 536870912, "D": "相册" }, { "C": 1073741824, "D": "数码" } ];
var BlogPageCateObj;


/* ----------------------------------- */
/* |Part 0.2: 公用 BLOG 函数         | */
/* ----------------------------------- */


/* 计算从 fileTime 到 nowTime 的时间 */
/* 返回 'xx 天 xx 小时 xx 分 xx 秒之前' 格式的字符串 */
function genBlogTimeStr(fileTime, nowTime) {
	var retv = new Array();
	var tagTime, nday, nhour, nmin;

	if (nowTime < fileTime || typeof(fileTime) != 'number' || 
			typeof(nowTime) != 'number') 
		return ("月光宝盒$#~%$@!");

	tagTime = nowTime - fileTime;
	nday = Math.floor(tagTime / (3600 * 24));
	if (nday > 0) 
		retv.push(nday + ' 天');
	tagTime -= nday * 3600 * 24;
	nhour = Math.floor(tagTime / 3600);
	if (nhour > 0)
		retv.push(' ' + nhour + ' 小时');
	else if (nday > 0)
		retv.push(' 0 小时');
	tagTime -= nhour * 3600;
	nmin = Math.floor(tagTime / 60);
	if (nmin > 0)
		retv.push(' ' + nmin + ' 分');
	else if (nday > 0 || nhour > 0)
		retv.push(' 0 分');
	tagTime -= nmin * 60;
	retv.push(' ' + tagTime + ' 秒之前');

	return retv.join('');
}

/* 生成一个随机的颜色样式 */
/* 返回 ' class = "randomColorxx"' 形式的字符串 */ 
function genBlogRandomColor() {
	var retv = new Array();

	retv.push(' class = "randomColor');
	retv.push(Math.round((Math.random() * 100)) % 10);
	retv.push('"');

	return retv.join('');
}

/* 生成查询用户的链接字符串 */
/* 返回如上描述的链接字符串 */
function genBlogQryLink(userid, useridEN, randomColor, className, idName) {
	var retv = new Array();

	retv.push('<a href="qry?U=' + useridEN + '" title = "');
	retv.push('查询 ' + userid + ' 的信息"');
	if (randomColor === true)
		retv.push(genBlogRandomColor());
	else if (typeof(className) == 'string' && className.length)
		retv.push(' class = "' + className +'"');
	if (typeof(idName) == 'string' && idName.length)
		retv.push(' id ="' + idName +'"');
	retv.push('>' + userid + '</a>');

	return retv.join('');
}

/* 生成用户头像的图片及查询链接 */
/* 返回如上描述的链接字符串 */
function genBlogQryImgLink(userid, useridEN, linkClassName, linkIdName, 
		imgClassName, imgIdName) {
	var retv = new Array(); 

	retv.push('<a href="qry?U=' + useridEN + '" title = "');
	retv.push('查询 ' + userid + ' 的信息"');
	if (typeof(linkClassName) == 'string' && linkClassName.length)
		retv.push('class = "' + linkClassName + '" ');
	if (typeof(linkIdName) == 'string' && linkClassName.length)
		retv.push('id = "' + linkIdName + '" ');
	retv.push('>');
	retv.push('<img src = "mypic?U=' + useridEN + '" ');
	if (typeof(imgClassName) == 'string' && imgClassName.length)
		retv.push('class = "' + imgClassName + '" ');
	if (typeof(imgIdName) == 'string' && imgIdName.length)
		retv.push('id = "' + imgIdName + '" ');
	retv.push('alt = "' + userid + ' 的头像" /></a>');

	return retv.join('');
}

/* 生成指向某篇文章的链接 */
/* 返回如上描述的链接字符串 */
function genBlogPostLink(useridEN, fileTime, postTitle, 
		randomColor, className, idName) {
	var retv = new Array();

	retv.push('<a href = "blogread?U=' + useridEN + '&T=' + fileTime);
	retv.push('" title = "阅读文章：' + postTitle + '"');
	if (randomColor === true)
		retv.push(genBlogRandomColor());
	else if (typeof(className) == 'string' && className.length)
		retv.push(' class = "' + className +'"');
	if (typeof(idName) == 'string' && idName.length)
		retv.push(' id = "' + idName +'"');
	retv.push('>' + postTitle + '</a>');

	return retv.join('');
}

/* 生成指向某个 BLOG 中某个栏目的链接 */
/* 返回如上描述的链接字符串 */
function genBlogSubLink(useridEN, subIdx, subTitle, 
		randomColor, className, idName) {
	var retv = new Array();

	retv.push('<a href = "blog?U=' + useridEN + '&subject=' + subIdx);
	retv.push('" title = "访问 ' + subTitle + ' 栏目"');
	if (randomColor === true)
		retv.push(genBlogRandomColor());
	else if (typeof(className) == 'string' && className.length)
		retv.push(' class = "' + className + '"');
	if (typeof(idName) == 'string' && idName.length)
		retv.push(' id = "' + idName + '"');
	retv.push('>' + subTitle + '</a>');

	return retv.join('');
}

/* 生成指向某个 BLOG 的链接 */
/* 返回如上描述的链接字符串 */
function genBlogLink(userid, useridEN, blogTitle, 
		randomColor, className, idName) {
	var retv = new Array();

	retv.push('<a href = "blog?U=' + useridEN);
	retv.push('" title = "访问 ' + userid + ' 的BLOG"');
	if (randomColor === true)
		retv.push(genBlogRandomColor());
	else if (typeof(className) == 'string' && className.length)
		retv.push(' class = "' + className + '"');
	if (typeof(idName) == 'string' && idName.length)
		retv.push(' id = "' + idName + '"');
	retv.push('>' + blogTitle + '</a>');

	return retv.join('');
}

/* 生成一个 span 标签 */
/* 返回如上描述的字符串 */
function genBlogSpanStr(className, idName, content) {
	var retv = new Array();

	retv.push('<span');
	if (typeof(className) == 'string' && className.length)
		retv.push(' class = "' + className + '"');
	if (typeof(idName) == 'string' && idName.length)
		retv.push(' id = "' + idName + '"');
	retv.push('>' + content + '</span>');

	return retv.join('');
}

/* 打印 blogpage_headBox 区域*/
/* 返回如上描述的链接字符串 */
function printBlogHead() {
	var retv = new Array();

	retv.push('<div id = "blog_headBox">');
	retv.push('<div id = "blogpage_nav1">');
	retv.push('<a href = "blogpage" title = "BLOG社区首页" class = "white">');
	retv.push('BLOG社区');
	if (BlogDataObj.CBA.length != 0) {
		retv.push('</a> &gt;&gt; <a href="blog?U=' + BlogDataObj.CBA + '" ');
		retv.push('title = "' + BlogDataObj.CBAE);
		retv.push('的BLOG首页" class = "white"');
		retv.push('>' + BlogDataObj.CBT + "</a>");
	} else 
		retv.push('首页</a>');
	retv.push('</div><div id = "blogpage_nav2">');
	if (BlogDataObj.ID.toLowerCase() != 'guest') {
		if (BlogDataObj.HB === true) {
			retv.push('<a href = "blog?U=' + BlogDataObj.IDE);
			retv.push('" title = "我的BLOG" class = "red">我的BLOG</a> | ');
		} else {
			retv.push('<a href = "blogsetup" title = "申请BLOG" ');
			retv.push('class = "red">我要申请</a> | ');
		}
	}
	retv.push('<a href = "boa" title = "回到 BBS 导读" class = "white">');
	retv.push('BBS导读</a> | ');
	retv.push('<a href = "doc?B=Blog" title = "用户帮助" class = "white">');
	retv.push('用户帮助</a>');
	if (BlogDataObj.CBA.length == 0) {
		retv.push(' | ');
		retv.push(blogpage_genSwitchPreivewStr());
		retv.push(' | ');
		retv.push('<a href = "javascript:blogpage_useOldFace();" ');
		retv.push('title = "切换到经典界面" class = "white">');
		retv.push('经典入口</a>');
	}
	if (BlogDataObj.BM === true && BlogDataObj.CBA.length != 0) {
		retv.push(' | <a href = "blogbolck?A=0&');
		if (BlogDataObj.BB === true)
			retv.push('V=0');
		else
			retv.push('V=1');
		retv.push('&U=' + BlogDataObj.IDE + '" title = "');
		if (BlogDataObj.BB === false)
			retv.push('屏蔽整个BLOG" class = "white">屏蔽BLOG</a>');
		else
			retv.push('解除屏蔽这个BLOG" class = "white">解除屏蔽</a>');
	}
	retv.push('</div>');
	retv.push('</div>');
	return retv.join('');
}

/* 打印 blog_footBox 区域*/
/* 返回如上描述的链接字符串 */
function printBlogFoot() {
	var retv = new Array();

	retv.push('<div id = "blog_footBox">');
	retv.push('<div id = "blog_footLogo1">');
	retv.push('<a href = "http://ytht.net" target = "_blank" ');
	retv.push('title = "永远的一塌糊涂" class = "blog_footLogo1Link">');
	retv.push('<img src = "/ht.gif" alt = "一塌糊涂BBS" class = "');
	retv.push('blog_footLogo1Img"/></a>');
	retv.push('</div>');
	retv.push('<div id = "blog_footLogo2">');
	retv.push('<a href = "http://tttan.com" target = "_blank" ');
	retv.push('title = "天天坛，天天都有好心情。" ');
	retv.push('class = "blog_footLogo2Link">');
	retv.push('<img src = "http://tttan.com/face/ttt-logo-white.gif" ');
	retv.push('alt = "天天坛BBS" class = "blog_footLogo2Img"/></a>');
	retv.push('</div>');
	retv.push('BLOG代码原始作者：ylsdd@ytht.net (ylsdd@tttan.com)。');
	retv.push('<br />部分图片来自新浪BLOG (blog.sina.com.cn)。<br />');
	retv.push('特此感谢。<br />');
	retv.push('<hr noshade size = "1" class = "randomColor');
	retv.push(Math.round(Math.random() * 100) % 10);
	retv.push('">');
	retv.push('一见如故BBS (YJRG.net) &copy; 2005 - 2007');
	retv.push('</div>');
	return retv.join('');
}



/* =================================== */
/* | Part 1: blogpage.c 对应的脚本   | */
/* =================================== */


/* ----------------------------------- */
/* |Part 1.1: 初始化 JSON 对象       | */
/* ----------------------------------- */

/* 新建BLOG */
var blogpage_newBlogJSON = [ ]; 
var blogpage_newBlogObj;
/* 热门BLOG */
var blogpage_hotBlogJSON = [ ];
var blogpage_hotBlogObj;
/* 新的文章 */
var blogpage_newPostJSON = [ ];
var blogpage_newPostObj;
/* 热门文章 */
var blogpage_hotPostJSON = [ ];
var blogpage_hotPostObj;
/* 社区新闻 */
var blogpage_annJSON = [ ];
var blogpage_annObj;
/* 按字母索引的BLOG */
var blogpage_indexByLetterJSON = [ ];
var blogpage_indexByLetterObj;
/* 按分类索引的BLOG */
var blogpage_blogIndexByCateJSON = [ ];
var blogpage_blogIndexByCateObj;
/* BLOG好友 */
var blogpage_friendJSON = [ ];
var blogpage_friendObj;


/* ----------------------------------- */
/* |Part 1.2: 与呈现无关的脚本       | */
/* ----------------------------------- */

function blogpage_useOldFace() {
	var cookie = new CJL_CookieUtil('blog', 30, '/', location.host, false);

	if (cookie)
		cookie.setSubValue('blogpage_useoldstyle', 1);

	location.replace('blogpage?old=1');
}

function blogpage_showPreview(show) {
	var cookie = new CJL_CookieUtil('blog', 30, '/', location.host, false);
	var retv = new Array();
	var spanObj1, spanObj2;
	var divObj = document.getElementById('blogpage_hotAndNewPostBox');
	var max;
	
	if (cookie) {
		if (show) {
			cookie.setSubValue('blogpage_hidepreview', '0');
		} else {
			cookie.setSubValue('blogpage_hidepreview', '1');
		}
	}
	if (show) {
		spanObj1 = document.getElementById('blogpage_hidePreviewSpan');
		spanObj2 = document.getElementById('blogpage_showPreviewSpan');
	} else {
		spanObj1 = document.getElementById('blogpage_showPreviewSpan');
		spanObj2 = document.getElementById('blogpage_hidePreviewSpan');
	}
	if (spanObj1 && spanObj2) {
		spanObj1.style.display = 'inline';
		spanObj2.style.display = 'none';
	}
	
	if (!divObj)
		return;

	if (show) {
		retv.push(blogpage_printHotPost(true, 0));
		retv.push(blogpage_printNewPost(true, 0));
		retv.push('<div class = "clear"></div>');
		divObj.innerHTML = retv.join('');
	} else {
		max = Math.min(blogpage_hotPostObj.length, blogpage_newPostObj.length);
		max = Math.max(max, 10);
		retv.push(blogpage_printHotPost(false, max));
		retv.push(blogpage_printNewPost(false, max));
		retv.push('<div class = "clear"></div>');
		divObj.innerHTML = retv.join('');
	}
	divObj.innerHTML.replace(/\s/g, '');
}

/* ----------------------------------- */
/* |Part 1.2: 非直接呈现的脚本       | */
/* ----------------------------------- */

/* 生成显示/隐藏预览的链接 */
/* 返回如上描述的链接字符串 */
function blogpage_genSwitchPreivewStr() {
	var retv = new Array();	
	var showPreview = true;
	var cookie = new CJL_CookieUtil('blog', 30, '/', location.host, false);

	if (cookie && cookie.getSubValue('blogpage_hidepreview') != 0)
		showPreview = false;
	if (cookie && typeof(cookie.getSubValue('blogpage_hidepreview')) 
			== 'undefined')
		showPreview = true;

	retv.push('<span id = "blogpage_hidePreviewSpan" ');
	if (showPreview) 
		retv.push(' style = "display: inline;">');
	else
		retv.push(' style = "display: none;">');
	retv.push('<a class = "white" ');
	retv.push('id = "blogpage_hidePreviewLink" title = "');
	retv.push('隐藏 BLOG 文章的预览" ');
	retv.push('href = "javascript:blogpage_showPreview(0);">');
	retv.push('隐藏预览</a></span>');
	retv.push('<span id = "blogpage_showPreviewSpan" ');
	if (!showPreview) 
		retv.push(' style = "display: inline;">');
	else
		retv.push(' style = "display: none;">');
	retv.push('<a class = "white" ');
	retv.push('id = "blogpage_showPreviewLink" title = "');
	retv.push('显示 BLOG 文章的预览" ');
	retv.push('href = "javascript:blogpage_showPreview(1);">');
	retv.push('显示预览</a>');
	retv.push('</span>')

	return retv.join('');
}

/* ----------------------------------- */
/* |Part 1.3: 主界面直接呈现的脚本   | */
/* ----------------------------------- */

/* 打印 blogpage_annBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printAnn() {
	var retv = new Array();

	retv.push('<div id = "blogpage_annBox">');
	retv.push('<div id = "blogpage_annLeft">');
	retv.push('<h2>社区新闻</h2>');
	retv.push(blogpage_annObj.join(''));
	retv.push('</div>');
	retv.push('<div id = "blogpage_annRight"><h2>进入BLOG</h2><br />');
	retv.push('<form action = "blog" method = "post" ');
	retv.push('accept-charset = "utf-8" name = "sb">');
	retv.push('<input name = "U" value = "请输入ID" ');
	retv.push('onFocus = "this.value=\'\';" ');
	retv.push('type = "text" maxlength = "12" size = "10"/>');
	retv.push('<input name = "utf8" value = "1" type = "hidden" />');
	retv.push('</form></div>');
	retv.push('<div class = "clear"></div>');
	retv.push('</div>');

	return retv.join('');
}


/* 打印 blogpage_login 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printLogin() {
	var retv = new Array();

	retv.push('<div id = "blogpage_loginBox">');
	retv.push('<h2>登录BLOG</h2>');
	retv.push('<form name = "l" action = "bbslogin" method = "post" ');
	retv.push('target = "_top">');
	retv.push('<input type = "hidden" name = "lastip1" value = "">');
	retv.push('<input type = "hidden" name = "lastip2" value = "">');
	retv.push('账号　<input type = "text" name = "id" maxlength = "14">');
	retv.push('　　密码　');
	retv.push('<input type = "password" name = "pw" maxlength = "32">');
	retv.push('　　');
	retv.push('<a href = "/ipmask.html" title = "了解 IP 验证范围的作用" ');
	retv.push('class = "blue">');
	retv.push('验证范围</a>　<select name = "ipmask">');
	retv.push('<option value = "0" selected>单  IP</option>');
	retv.push('<option value = "6">64  IP</option>');
	retv.push('<option value = "8">256 IP</option>');
	retv.push('<option value = "15">32K IP</option></select>');
	retv.push('<input type = "hidden" name = "utf8" value = "1">');
	retv.push('<input type = "hidden" name = "blog" value = "1">');
	retv.push('　　<input type = "submit" value = "　登　录　">');
	retv.push('　<input type = "submit" value = "　注　册　" ');
	retv.push('onclick="{top.location.href=\'/' + BlogDataObj.SMAGIC);
	retv.push('/bbsmailreg\'; return false;}"></form>');
	retv.push('</div>');

	return retv.join('');
}

/* 打印 blogpage_hotPostBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printHotPost(showPreView, max) {
	var retv = new Array();
	var i;

	function blogpage_genHotPostStr(Obj, i, showPreview) {
		var tmpretv = new Array();
		retv.push('<div class = "blogpage_hotPostItem" ');
		retv.push('id = "blogpage_hotPostItem' + i);
		retv.push('">');

		retv.push(genBlogQryImgLink(Obj.AI, Obj.AH, 
				'blogpage_hotPostItemImgSpan', 
					'blogpage_hotPostItemImgSpan' + i, 
						'blogpage_hotPostItemImg', 
							'blogpage_hotPostItemImg' + i));
		
		tmpretv.push(' [ ');
		tmpretv.push(genBlogSpanStr('blogpage_hotPostItemBlogAuthor', 
				'blogpage_hotPostItemBlogAuthor' + i, 
					genBlogQryLink(Obj.AI, Obj.AH, true, 
						'blogpage_hotPostItemBlogAuthorLink', 
							'blogpage_hotPostItemBlogAuthorLink' + i)));
		tmpretv.push(' | ');
		
		tmpretv.push(genBlogSpanStr('blogpage_hotPostItemBlogTitle', 
				'blogpage_hotPostItemBlogTitle' + i, 
					genBlogLink(Obj.AI, Obj.AH, Obj.BT, true, 
						'blogpage_hotPostItemBlogTitleLink', 
							'blogpage_hotPostItemBlogTitleLink' + i)));
		tmpretv.push(' ] ');
		
		if (showPreview) {
			retv.push(tmpretv.join(''));
		} else {
			retv.push(' ');
		}

		retv.push(genBlogSpanStr('blogpage_hotPostItemPostTitle', 
				'blogpage_hotPostItemPostTitle' + i, 
					genBlogPostLink(Obj.AH, Obj.FT, Obj.AT, false, 
						'blogpage_hotPostItemPostTitleLink', 
								'blogpage_hotPostItemPostTitleLink' + i)));

		if (!showPreview) {
			retv.push(tmpretv.join(''));
			retv.push('</div>');
			return 0;
		}
		retv.push(' ... '); 

		retv.push(genBlogSpanStr('blogpage_hotPostItemPostPreview', 
				'blogpage_hotPostItemPostPreview' + i, Obj.AP));
		retv.push('... ');

		retv.push(genBlogSpanStr('blogpage_hotPostItemPostTime', 
					'blogpage_hotPostItemPostTime' + i, 
							genBlogTimeStr(Obj.FT, BlogDataObj.NOW)));

		retv.push('</div>');
	}

	if (showPreView) {
		max = blogpage_hotPostObj.length;
		retv.push('<div id = "blogpage_hotPostBox">');
	} else {
		retv.push('<div id = "blogpage_hotPostBoxWithoutPreview">');
	}
	retv.push('<h2>热门文章</h2>');
	if (blogpage_hotPostObj.length == 0) {
		retv.push('<div style = "text-align: center;">');
		retv.push('好奇怪啊，一篇热门文章都没有？');
		retv.push('</div></div>');
		return retv.join('');
	}
	for (i = 0; i < blogpage_hotPostObj.length && i < max; i++) {
		blogpage_genHotPostStr(blogpage_hotPostObj[i], i, showPreView);
	}
	retv.push('<div class = "clear"></div>');
	retv.push('</div>')
	return retv.join('');
}


/* 打印 blogpage_newPostBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printNewPost(showPreview, max) {
	var retv = new Array();
	var i;

	function blogpage_genNewPostStr(Obj, i, showPreview) {
		var tmpretv = new Array();
		retv.push('<div class = "blogpage_newPostItem" ');
		retv.push('id = "blogpage_newPostItem' + i);
		retv.push('">');

		retv.push(genBlogQryImgLink(Obj.AI, Obj.AH, 
				'blogpage_newPostItemImgSpan', 
					'blogpage_newPostItemImgSpan' + i, 
						'blogpage_newPostItemImg', 
							'blogpage_newPostItemImg' + i));
		
		tmpretv.push(' [ ');
		tmpretv.push(genBlogSpanStr('blogpage_newPostItemBlogAuthor', 
				'blogpage_newPostItemBlogAuthor' + i, 
					genBlogQryLink(Obj.AI, Obj.AH, true, 
						'blogpage_newPostItemBlogAuthorLink', 
								'blogpage_newPostItemBlogAuthorLink' + i)));
		tmpretv.push(' | ');
		
		tmpretv.push(genBlogSpanStr('blogpage_newPostItemBlogTitle', 
				'blogpage_newPostItemBlogTitle' + i, 
					genBlogLink(Obj.AI, Obj.AH, Obj.BT, true, 
						'blogpage_newPostItemBlogTitleLink', 
							'blogpage_newPostItemBlogTitleLink' + i)));
		tmpretv.push(' ] ');

		if (showPreview)
			retv.push(tmpretv.join('')); 
		else
			retv.push(' ');
		
		retv.push(genBlogSpanStr('blogpage_newPostItemPostTitle', 
				'blogpage_newPostItemPostTitle' + i, 
					genBlogPostLink(Obj.AH, Obj.FT, Obj.AT, false, 
						'blogpage_newPostItemPostTitleLink', 
							'blogpage_newPostItemPostTitleLink' + i)));
		if (!showPreview) {
			retv.push(tmpretv.join(''));
			retv.push('</div>');
			return 0;
		}
		retv.push(' ... '); 

		retv.push(genBlogSpanStr('blogpage_newPostItemPostPreview', 
				'blogpage_newPostItemPostPreview' + i, Obj.AP));
		retv.push('... ');

		retv.push(genBlogSpanStr('blogpage_newPostItemPostTime', 
				'blogpage_newPostItemPostTime' + i, 
					genBlogTimeStr(Obj.FT, BlogDataObj.NOW)));
	
		retv.push('</div>');
	}
	
	if (showPreview) {
		max = blogpage_newPostObj.length;
		retv.push('<div id = "blogpage_newPostBox">');
	} else {
		retv.push('<div id = "blogpage_newPostBoxWithoutPreview">');
	}
	retv.push('<h2>最新文章</h2>');
	if (blogpage_newPostObj.length == 0) {
		retv.push('<div style = "text-align: center;">');
		retv.push('好奇怪啊，一篇新的文章都没有？');
		retv.push('</div></div>');
		return retv.join('');
	}
	for (i = 0; i < blogpage_newPostObj.length && i < max; i++) {
		blogpage_genNewPostStr(blogpage_newPostObj[i], i, showPreview);
	}
	retv.push('<div class = "clear"></div>');
	retv.push('</div>')
	return retv.join('');
}

/* 打印 blogpage_HotAndNewPostBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printHotAndNewPost() {
	var retv = new Array();
	var max;
	var cookie = new CJL_CookieUtil('blog', 30, '/', location.host, false);
	var showPreview = true;

	max = Math.max(10, 
			Math.min(blogpage_hotPostObj.length, blogpage_newPostObj.length));
	if (cookie && cookie.getSubValue('blogpage_hidepreview') != 0)
		showPreview = false;
	if (cookie && typeof(cookie.getSubValue('blogpage_hidepreview'))
			== 'undefined');
		showPreview = true;
	retv.push('<div id = "blogpage_hotAndNewPostBox">');
	retv.push(blogpage_printHotPost(showPreview, max));
	retv.push(blogpage_printNewPost(showPreview, max));
	retv.push('<div class = "clear"></div>');
	retv.push('</div>');

	return retv.join('');
}

/* 打印 blogpage_friendBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printFriend() {
	var retv = new Array();
	var tmpObj;
	var i;

	if (BlogDataObj.ID == 'guest')
		return '';
	retv.push('<div id = "blogpage_friendBox">');
	retv.push('<h2>我的BLOG好友</h2>');
	if (blogpage_friendObj.length == 0) {
		retv.push('<div style = "text-align: center;">');
		retv.push('您还没有添加过BLOG好友，<a href="');
		retv.push('blogeditfriend" title="设置BLOG好友">快点击这里');
		retv.push('进行设置吧</a>！</div>');
		retv.push('</div>');
		return retv.join('');
	}
	retv.push('<ul>');
	for (i = 0; i < blogpage_friendObj.length; i++) {
		tmpObj = blogpage_friendObj[i];
		retv.push('<li><div class = "blogpage_friendImgDiv">');
		retv.push('<a href = "blog?U=' + tmpObj.IDE + '" ');
		retv.push('title = "访问 ' + tmpObj.ID);
		retv.push(' 的BLOG" class = "blogpage_friendImgLink">');
		retv.push('<img src="mypic?U=' + tmpObj.IDE);
		retv.push('" alt = "' + tmpObj.ID +' 的头像" class = "');
		retv.push('blogpage_friendImg"/></a></div>');
		retv.push('<div class = "blogpage_friendTxt">'); 
		retv.push(genBlogLink(tmpObj.ID, tmpObj.IDE, tmpObj.ID, true, 
				'blogpage_friendTxtlink', 'blogpage_friendTxtlink' + i));
		retv.push('</div></li>');
	}
	retv.push('</ul><div class = "clear"></div>');
	retv.push('</div>');

	return retv.join('');
}

/* 打印 blogpage_recommendBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printRecommend() {
	var retv = new Array();
	var tmpObj;
	var i, j, tag, p, count;

	retv.push('<div id = "blogpage_recommendBox">');

	retv.push('<div id = "blogpage_recommendHotBlog">');
	retv.push('<h2>热门BLOG</h2>');
	if (blogpage_hotBlogObj.length == 0) {
		retv.push('<div style = "text-align: center;">');
		retv.push('当前没有热门BLOG。');
		retv.push('</div>');
	}

	// 随机选取10个热门的BLOG
	for (count = 0, i = 0; i < blogpage_hotBlogObj.length; i++) {
		if (blogpage_hotBlogObj.length - i > 10 - count && 
			Math.random() >
				(10 - count) / (blogpage_hotBlogObj.length - i))
			continue;
		if (count >= 10)
			break;
		tmpObj = blogpage_hotBlogObj[i];
		retv.push('<p class = "blogpage_hotBlogItem">[ ');
		retv.push(genBlogQryLink(tmpObj.AI, tmpObj.AH, true, 
				'blogpage_hotBlogItemAuthorLink', 
					'blogpage_hotBlogItemAuthorLink' + i));
		retv.push(' | ');
		retv.push(genBlogLink(tmpObj.AI, tmpObj.AH, tmpObj.BT, true, 
				'blogpage_hotBlogItemBlogLink', 
					'blogpage_hotBlogItemBlogLink' + i));
		retv.push(' ]</p>');
		count++;
	}
	retv.push('</div>');

	retv.push('<div id = "blogpage_recommendNewBlog">');
	retv.push('<h2>新建BLOG</h2>');
	if (blogpage_newBlogObj.length == 0) {
		retv.push('<div style = "text-align: center;">');
		retv.push('最近没有新建的BLOG');
		retv.push('</div>');
	}
	for (i = 0; i < blogpage_newBlogObj.length; i ++) {
		tmpObj = blogpage_newBlogObj[i];
		retv.push('<p class = "blogpage_newBlogItem">[ ');
		retv.push(genBlogQryLink(tmpObj.AI, tmpObj.AH, true, 
				'blogpage_newBlogItemAuhtorLink', 
					'blogpage_newBlogItemAuhtorLink' + i));
		retv.push(' | ');
		retv.push(genBlogLink(tmpObj.AI, tmpObj.AH, tmpObj.BT, true, 
				'blogpage_newBlogItemBlogLink', 
					'blogpage_newBlogItemBlogLink' + i));
		retv.push(' ]</p>');
	}
	retv.push('</div>');

	retv.push('<div id = "blogpage_recommendSubject">');
	retv.push('<h2>BLOG标签</h2>');
	for (i = 0; i < 4; i++) {
		retv.push('<div id = "blogpage_recommendSubject' + (i + 1) +'">');
		for (j = 0; j < 8; j++) {
			tag = i * 8 + j;
			if (tag >= BlogCateObj.length)
				break;
			tmpObj = BlogCateObj[tag];
			retv.push('<p class = "blogpage_recommendSubjectItem">');
			retv.push('<a href="blogpage?cate=');
			retv.push(tmpObj.C + '" title = "');
			retv.push('查看标签为 ' + tmpObj.D + ' ');
			retv.push('的BLOG" id = "blogpage_recommendSubjectItem' + tag);
			retv.push('" ' + genBlogRandomColor());
			retv.push('>' + tmpObj.D + '</a></p>')
		}
		retv.push('</div>');
	}
	retv.push('<div class = "clear"></div>');
	retv.push('</div>')

	retv.push('<div class = "clear"></div>');
	retv.push('</div>');
	return retv.join('');
}

/* 打印 blogpage_indexBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printBlogIndex() {
	var retv = new Array();
	var i, tmpChar;

	retv.push('<div id = "blogpage_indexBox"><ul>');
	retv.push('<h2>BLOG索引</h2>');

	for (i = 65; i < 91; i++) {
		tmpChar = String.fromCharCode(i);
		retv.push('<li><a href = "blogpage?index=' + tmpChar);
		retv.push('" title = "查看所有 ' + tmpChar);
		retv.push(' 开头的ID的BLOG" class = "randomColor');
		retv.push(Math.round(Math.random() * 100) % 10);
		retv.push('" id = "blogpage_blogIndex' + tmpChar + '">');
		retv.push(tmpChar + '</a></li>');
	}

	retv.push('</ul><div class = "clear"></div></div>');
	return retv.join('');
}


/* ----------------------------------- */
/* |Part 1.4: 按字母排列 BLOG 的呈现 | */
/* ----------------------------------- */

/* 打印 blogpage_blogIndexByCateBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printBlogIndexByLetter() {
	var retv = new Array();
	var i, j;
	var tmpObj1, tmpObj2, tmpId;

	function getCurrIndex() {
		var url = document.location.search;
		var pos; 

		pos = url.indexOf('index=');
		if (pos == - 1)
			return false;
		if (url.charAt(pos + 6) == '')
			return false;
		return url.charAt(pos + 6);
	}

	retv.push('<div id = "blogpage_blogIndexByLetterBox">');
	if (getCurrIndex() === false) {
		retv.push('<div style = "text-align: center; color: red;">');
		retv.push('好像出了一点问题哦，从 <a href = "');
		retv.push('javascript: history.go(-1);">这里</a> 后退一下看看？');
		retv.push('</div></div>');
		return retv.join('');
	}
	retv.push('<h2>所有以 ' + getCurrIndex() + ' 开头的网友的BLOG</h2>');
	if (blogpage_indexByLetterObj.length == 0) {
		retv.push('<div style = "text-align: center;">');
		retv.push('好奇怪，一个人都没有。。。</div></div>');
		return retv.join('');
	}
	for (i = 0; i < blogpage_indexByLetterObj.length; i++) {
		if (i == blogpage_indexByLetterObj.length - 1)
			retv.push('<div class = "blogpage_blogIndexByLetterLastItem">');
		else
			retv.push('<div class = "blogpage_blogIndexByLetterItem">');
		
		tmpObj1 = blogpage_indexByLetterObj[i];
		
		//retv.push(genBlogQryImgLink(tmpObj1.ID, tmpObj1.IDE, 
		//		'blogpage_blogIndexByLetterItemLink', 
		//			'blogpage_blogIndexByLetterItemLink' + i, 
		//				'blogpage_blogIndexByLetterItemImg', 
		//					'blogpage_blogIndexByLetterItemImg' + i));
		
		retv.push(' [ ');
		retv.push(genBlogSpanStr('blogpage_blogIndexByLetterItemBlogAuthor', 
				'blogpage_blogIndexByLetterItemBlogAuthor' + i, 
					genBlogQryLink(tmpObj1.ID, tmpObj1.IDE, true, 
						'blogpage_blogIndexByLetterItemBlogAuthorLink', 
							'blogpage_blogIndexByLetterItemBlogAuthorLink' 
								+ i)));
		retv.push(' | ');
		
		retv.push(genBlogSpanStr('blogpage_blogIndexByLetterItemBlogTitle', 
				'blogpage_blogIndexByLetterItemBlogTitle' + i, 
					genBlogLink(tmpObj1.ID, tmpObj1.IDE, tmpObj1.BT, true, 
						'blogpage_blogIndexByLetterItemBlogTitle', 
							'blogpage_blogIndexByLetterItemBlogTitle' + i)));
		retv.push(' ] ');

		for (j = 0; j < tmpObj1.SL.length; j++) {
			tmpObj2 = tmpObj1.SL[j];
			tmpId = 'blogpage_blogIndexByLetterItemSubject';
			retv.push(genBlogSpanStr(tmpId, tmpId + i + '_' + j, 
					genBlogSubLink(tmpObj1.IDE, tmpObj2.IDX, tmpObj2.T, false, 
						tmpId + 'Link', tmpId + 'Link' + i + '_' + j)));
			if (j != tmpObj1.SL.length - 1)
				retv.push(' ... ');
		}
		retv.push('</div>');
	}
	retv.push('</div>')

	return retv.join('');
}

/* ----------------------------------- */
/* |Part 1.5: 按分类排列 BLOG 的呈现 | */
/* ----------------------------------- */

/* 打印 blogpage_blogIndexByCateBox 区域*/
/* 返回如上描述的链接字符串 */
function blogpage_printBlogIndexByCate() {
	var retv = new Array(); 
	var catenum = -1, showall = false;
	var i, j;
	var tmpObj;

	function getCateId() {
		var pos, catetag, i;
		var url = document.location.search;

		pos = url.indexOf('cate=');
		if (pos == -1)
			return;
		catenum = -1;
		if (url.charAt(pos + 5) != '') {
			catetag = parseInt((url.substring(pos + 5, url.length)));
			for (i = 0; i < BlogCateObj.length; i++) {
				if (BlogCateObj[i].C == catetag) {
					catenum = i;
					break;
				}
			}
		}
		pos = url.indexOf('all=');
		if (pos == -1)
			return;
		if (url.charAt(pos + 4) == '1')
			showall = true;
		return;
	}

	function blogpage_printBlogIndexByCateItem(Obj, last, i) {
		var k;
		if (last)
			retv.push('<div class = "blogpage_blogIndexByCateLastItem">');
		else
			retv.push('<div class = "blogpage_blogIndexByCateItem">');

		//retv.push(genBlogQryImgLink(Obj.ID, Obj.IDE, '', '', '', ''));
		//retv.push(genBlogQryImgLink(Obj.ID, Obj.IDE, 
		//		'blogpage_blogIndexByCateItemLink', 
		//			'blogpage_blogIndexByCateItemLink' + i, 
		//				'blogpage_blogIndexByCateItemImg', 
		//					'blogpage_blogIndexByCateItemImg' + i));

		retv.push(' [ ');
		retv.push(genBlogSpanStr('blogpage_blogIndexByCateItemBlogAuthor', 
				'blogpage_blogIndexByCateItemBlogAuthor' + i, 
					genBlogQryLink(Obj.ID, Obj.IDE, true, 
						'blogpage_blogIndexByCateItemBlogAuthorLink', 
							'blogpage_blogIndexByCateItemBlogAuthorLink' + i)));
		retv.push(' | ');
		retv.push(genBlogSpanStr('blogpage_blogIndexByCateItemBlogTitle', 
				'blogpage_blogIndexByCateItemBlogTitle' + i, 
					genBlogLink(Obj.ID, Obj.IDE, Obj.BT, true, 
						'blogpage_blogIndexByCateItemBlogTitleLink', 
							'blogpage_blogIndexByCateItemBlogTitleLink' + i)));
		retv.push(' ] ');
		
		for (k = 0; k < Obj.SL.length; k++) {
			retv.push(genBlogSpanStr('blogpage_blogIndexByCateItemSubject', 
					'blogpage_blogIndexByCateItemSubject' + i, 
						genBlogSubLink(Obj.IDE, Obj.SL[k].IDX, 
							Obj.SL[k].T, false, 
								'blogpage_blogIndexByCateItemSubjectLink', 
									'blogpage_blogIndexByCateItemSubjectLink' 
										+ i)));
			if (k != Obj.SL.length - 1)
				retv.push(' ... ');
		}
		retv.push('</div>');
	}

	function blogpage_printBlogIndexByCateTitle(T, C) {
		if (showall) {
			retv.push('<h2>所有标签包含 ' + T);
			retv.push(' 的BLOG</h2>');
		} else {
			retv.push('<h2>标签包含 ' + T);
			retv.push(' 的文章<span class = "blogpage_blogIndexByCateH2">');
			retv.push(' (<a href = "blogpage?cate=');
			retv.push(C + '&all=1" title = "');
			retv.push('查看所有标签中含有 ' + T);
			retv.push(' 的BLOG" class = "blogpage_blogIndexByCateShowall">');
			retv.push('查看全部</a>)</span></h2>');
		}
	}

	function printBlogCateHor() {
		retv.push('<h2>BLOG标签索引</h2>');
		retv.push('<ul>');
		for (i = 0; i < BlogCateObj.length; i++) {
			retv.push('<li><a href = "blogpage?cate=' + BlogCateObj[i].C);
			retv.push('" title = "查看标签为 ' + BlogCateObj[i].D);
			retv.push(' 的BLOG" ');
			retv.push(genBlogRandomColor());
			retv.push('>' + BlogCateObj[i].D + '</a></li>');
		}
		retv.push('</ul><div class = "clear"></div><br />');
	}

	getCateId();

	if (catenum < 0 && showall)
		showall = false;
	if (showall)
		catenum = 0;
	retv.push('<div id = "blogpage_blogIndexByCateBox">');
	printBlogCateHor();
	if (catenum >= 0) {
		blogpage_printBlogIndexByCateTitle(blogpage_blogIndexByCateObj[catenum].T, 
				blogpage_blogIndexByCateObj[catenum].C);
		if (blogpage_blogIndexByCateObj[catenum].BL.length == 0) {
			retv.push('<div style = "text-align: center;">');
			retv.push('当前标签没有对应的BLOG :-(</div>');
		} else {
			for (i = 0; i < blogpage_blogIndexByCateObj[catenum].BL.length; i++) {
				tmpObj  = blogpage_blogIndexByCateObj[catenum].BL[i];
				blogpage_printBlogIndexByCateItem(tmpObj, 
						i == blogpage_blogIndexByCateObj[catenum].BL.length - 1, i);
			}
		}
	}
	for (i = 0; i < BlogCateObj.length && !showall; i++) {
		if (catenum == i)
			continue;
		blogpage_printBlogIndexByCateTitle(blogpage_blogIndexByCateObj[i].T, 
				blogpage_blogIndexByCateObj[i].C);
		if (blogpage_blogIndexByCateObj[i].BL.length == 0) {
			retv.push('<div style = "text-align: center;">');
			retv.push('当前标签没有对应的BLOG :-(</div>');
		} else {
			for (j = 0; j < blogpage_blogIndexByCateObj[i].BL.length; j++) {
				tmpObj = blogpage_blogIndexByCateObj[i].BL[j];
				blogpage_printBlogIndexByCateItem(tmpObj, 
						j == blogpage_blogIndexByCateObj[i].BL.length - 1, j);
			}
		}
		
	}
	retv.push('</div>');

	return retv.join('');
}

/* ----------------------------------- */
/* |Part 1.6: blogpage.c 的入口      | */
/* ----------------------------------- */

/* 呈现 blogpage.c 的主界面或副界面 */
/* 无返回值 */
function genBlogPage (mode) {
	// mode = 0:	default mode
	// mode = 1:	Category mode
	// mode = 2:	Index mode
	var retv = new Array();
	var cookie = new CJL_CookieUtil('blog', 30, "/", location.host, false);

	if (cookie && cookie.getSubValue('blogpage_useoldstyle') == '1') {
		location.replace('blogpage?old=1');
		return 0;
	}

	BlogDataObj = eval(BlogDataJSON);
	if (typeof(BlogDataObj.ID) == 'undefined') {
		retv.push('<div style = "text-align: center; color: red">');
		retv.push('从服务器读取的数据不完整，');
		retv.push('<a href="javascript: location.reload();">请点击这里');
		retv.push('刷新本页</a>。</div>');
		document.write(retv.join(''));
		return 0;
	}
	retv.push('<div id = "blogpage_mainBox">');
	retv.push(printBlogHead());
		if (mode == 1) {
			BlogCateObj = eval(BlogCateJSON);
			blogpage_blogIndexByCateObj = eval(blogpage_blogIndexByCateJSON)
			retv.push(blogpage_printBlogIndexByCate());
		} else if (mode == 2) {
			blogpage_indexByLetterObj = eval(blogpage_indexByLetterJSON);
			retv.push(blogpage_printBlogIndexByLetter());
			retv.push(blogpage_printBlogIndex());
		} else {
			blogpage_annObj = eval(blogpage_annJSON);
			blogpage_hotPostObj = eval(blogpage_hotPostJSON);
			blogpage_newPostObj = eval(blogpage_newPostJSON);
			blogpage_hotBlogObj = eval(blogpage_hotBlogJSON);
			blogpage_newBlogObj = eval(blogpage_newBlogJSON);
			blogpage_friendObj = eval(blogpage_friendJSON);
			BlogCateObj = eval(BlogCateJSON);
			retv.push(blogpage_printAnn());
			if (BlogDataObj.ID == "guest")
				retv.push(blogpage_printLogin());
			retv.push(blogpage_printHotAndNewPost());
			retv.push(blogpage_printFriend());
			retv.push(blogpage_printRecommend());
			retv.push(blogpage_printBlogIndex());
		}
	retv.push(printBlogFoot());
	retv.push('</div>');
	document.write(retv.join(''));
}

