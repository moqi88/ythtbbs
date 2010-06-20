var http;
var ajax_userid; 
var ajax_msg;
var smagic;
var anonypid;

function ajax_getHTTPObject() {
	var browser = navigator.appName;

	if (http)
		return http;

	try {
		http = new ActiveXObject("Msxml2.XMLHTTP");
	} catch (e) {
		try {
			http = new ActiveXObject("Microsoft.XMLHTTP");
		} catch (E) {
			http = false;
		}
	}

	if (!http && typeof XMLHttpRequest != 'undefined') {
		http = new XMLHttpRequest();
	}

	return http;
}

function ajax_test_login() {
	function ajax_test_login_print() {
		var div = document.getElementById('ajax_test');

		if (!div)
			return;
		
		if (!ajax_getHTTPObject())
			return;

		switch (http.readyState) {
			case 0:
				ajax_msg = "对象未初始化！";
				break;
			case 1:
				ajax_msg = "正在读取服务器数据。。。";
				break;
			case 2:
				ajax_msg = "数据已读取。。。";
				break;
			case 3:
				ajax_msg = "数据交互中。。。";
				break;
			case 4:
				if (http.status == 200) {
					ajax_uesrid = http.responseText;
					ajax_msg = ajax_uesrid;
				}
				break;
			default:
				ajax_msg = "老婆，快出来看上帝。。。";
				break;
		}
	
		if (!ajax_msg)
			ajax_msg = "老婆，快出来看上帝！";
		div.innerHTML = ajax_msg;
		return;
	}
	var url="bbslogin?ajax=2";

	if(!ajax_getHTTPObject())
		return;
	
	http.open("GET", url, false);
	http.onreadystatechange = ajax_test_login_print;
	http.send(null);

	ajax_userid = http.responseText;

	return;
}

function ajax_login(objname, showsuccess, showbutton) {
	var url = "bbslogin?ajax=1";
	var div = document.getElementById(objname);

	if(!div || !ajax_getHTTPObject())
		return;
	
	ajax_test_login();

	if (ajax_userid == 'guest') {
		ajax_print_login_box(div, showbutton);
	}
	else if (showsuccess) {
		div.innerHTML = '您已经成功登录，目前使用帐号 ' 
			+ ajax_userid + ' 。';
	}
}

function ajax_print_login_box(obj, showbutton) {
	var str;
	
	if (!obj)
		return;
	
	str = '<div id="ajax_login_box"> \
		用户名：<input id="ajax_login_username" \
		type="text" maxlength="14" class="ipnutbox">　　\
		密码：<input id="ajax_login_password" \
		type="password" maxlength="16" class="inputbox">';
	if (showbutton >= 1)
		str += '　　<input id="ajax_login_button" \
		type="button" value="登录" onclick="ajax_do_login();"><br />';
	str += '</div>';
	obj.innerHTML = str;
}

function ajax_do_login(div, obj1, obj2, ipmask) {
	/* div  显示信息对象 */
	/* obj1 用户名对象 */
	/* obj2 密码对象 */
	var url = 'bbslogin?ajax=1';
	var postStr;
	var pattern = /^\/[a-zA-Z0-9\+\*]{39}_[A-Z]{1}\/\?t=[0-9]{10}\/\d/;

	if (!obj1 || !obj2 || !div)
		return;

	if (!ajax_getHTTPObject())
		return;

	div.innerHTML = '正在尝试登录，请等待...';
	postStr = 'id='+obj1.value+'&pw='+obj2.value+'&ipmask='+ipmask;
	postStr += '&utf8=1';
	http.open('POST', url, false);
	http.setRequestHeader('Content-Type', 
		'application/x-www-form-urlencoded');
	/*http.onreadystatechange = function () {
		switch (http.readyState) {
			case 0:
				div.innerHTML = '正在初始化连接。';
				break;
			case 1:
				div.innerHTML = '正在发送用户信息。';
				break;
			case 2:
				div.innerHTML = '用户信息发送成功。';
				break;
			case 3:
				div.innerHTML = '等待服务器回应。';
				break;
		}
	}*/
	http.send(postStr);
	if (pattern.test(http.responseText)) {
		url = http.responseText;
		smagic = url.split('/')[1];
		anonypid = url.split('/')[3];
		ajax_userid = obj1.value;
	} else {
		div.innerHTML = http.responseText;
		ajax_userid = 'guest';
	}
	delete(http);
}
