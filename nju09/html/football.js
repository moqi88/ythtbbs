var graph;
var secstr;
var rootstr;

function graphitem() {
	this.name = name;
	this.pos = pos;
	this.team = team;
	this.subs = 0;
	this.subgraph = new Array();
}

function freegraph(graph) {
}

function admin_print_title(sec) {
	var retv;

	retv  = '<h1>管理界面</h1>';
	retv += '<table><tr><td><input type="button" value="管理首页" ';
	retv += 'onclick="location.href=\'bbsfb?A=9\'"></td>';
	retv += '<form action="bbsfb?A=9&T=7&G=' + sec + '" method="post">';
	retv += '<td><input type="submit" value="更新赛图">';
	retv += '</td></form><td><input type="button" value="导入赛队" ';
	retv += 'onclick="bbsfb_import_teams();"></td>';
	retv += '<td><input type="button" value="添加战报" ';
	retv += 'onclick="location.href=\'bbsfb?A=9&T=9&G=' + sec + '\'"></td>';
	retv += '<td>发布新闻</td></tr></table>';

	document.write(retv);
}

function gi(sec, root) {
	var obj = document.getElementById('bbsfb_graph_main');

	if (!obj)
		return;

	obj.innerHTML = '';
	secstr = sec;
	rootstr = root;
}

function bbsfb_admin_addgraph(pos) {
	var retv;

	retv  = '<form action="bbsfb?A=9&T=1" method="post">';
	retv += '<div align="center" class="annbox">';
	retv += '<h3>增加子节点</h3>';
	retv += '此项操作将为当前节点增加一个子节点。<br /><br />';
	retv += '<input type="hidden" name="posstr" value="' + pos + '">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '请输入节点名称　<input type="text" maxlength="10" ';
	retv += 'name="graphname">　　';
	retv += '<input type="button" onclick="document.getElementById(';
	retv += '\'bbsfb_admin_main\').innerHTML=\'\';" value="放弃">　　';
	retv += '<input type="submit" value="新增"><br /><br />';
	retv += '</div></form><br />';

	return retv;
}

function bbsfb_admin_delgraph(pos) {
	var retv;

	retv  = '<form action="bbsfb?A=9&T=2" method="post"';
	retv += 'onsubmit="return confirm(\'确定删除？\')">';
	retv += '<div align="center" class="annbox">';
	retv += '<h3>删除节点</h3>';
	retv += '这项操作将删除这个节点，并删除属于这个节点的';
	retv += '所有赛队，请慎重使用。<br /><br />';
	retv += '<input type="hidden" name="posstr" value="' + pos + '">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '<input type="button" onclick="document.getElementById(';
	retv += '\'bbsfb_admin_main\').innerHTML=\'\';" value="放弃">　　';
	retv += '<input type="submit" value="删除"><br /><br />';
	retv += '</div></form><br />';

	return retv; 

}

function bbsfb_admin_renamegraph (pos) {
	var retv;

	retv  = '<form action="bbsfb?A=9&T=3" method="post">';
	retv += '<div align="center" class="annbox">';
	retv += '<h3>重命名节点</h3>';
	retv += '这项操作将重新命名这个节点，但不会影响这个节点的子节点';
	retv += '或参赛队伍及赛程安排。<br /><br />';
	retv += '请输入新的名称　'
	retv += '<input type="text" name="newname" maxlength="40">　　';
	retv += '<input type="hidden" name="posstr" value="' + pos + '">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '<input type="button" onclick="document.getElementById(';
	retv += '\'bbsfb_admin_main\').innerHTML=\'\';" value="放弃">　　';
	retv += '<input type="submit" value="改名"><br /><br />';
	retv += '</div></form><br />';

	return retv; 

}

function bbsfb_admin_addteam(pos) {
	var retv;

	retv  = '<form action="bbsfb?A=9&T=4" method="post">';
	retv += '<div align="center" class="annbox">';
	retv += '<h3>增加赛队</h3>';
	retv += '这项操作将在当前位置增加一个直属赛队，如果当前位置的子节点';
	retv += '与直属赛队之和大于等于2，那么该操作将失败。<br /><br />';
	retv += '<input type="hidden" name="posstr" value="' + pos + '">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '<input type="hidden" name="tnum" value="0">';
	retv += '赛队名称　'
	retv += '<input type="text" name="tname" maxlength="15">　　';
	retv += '学校名称　'
	retv += '<input type="text" name="tuniv" maxlength="15"><br />';
	retv += '球员一　　'
	retv += '<input type="text" name="tmember1" maxlength="6">　　';
	retv += '球员七　　'
	retv += '<input type="text" name="tmember7" maxlength="6"><br />';
	retv += '球员二　　'
	retv += '<input type="text" name="tmember2" maxlength="6">　　';
	retv += '球员八　　'
	retv += '<input type="text" name="tmember8" maxlength="6"><br />';
	retv += '球员三　　'
	retv += '<input type="text" name="tmember3" maxlength="6">　　';
	retv += '球员九　　'
	retv += '<input type="text" name="tmember9" maxlength="6"><br />';
	retv += '球员四　　'
	retv += '<input type="text" name="tmember4" maxlength="6">　　';
	retv += '球员十　　'
	retv += '<input type="text" name="tmember10" maxlength="6"><br />';
	retv += '球员五　　'
	retv += '<input type="text" name="tmember5" maxlength="6">　　';
	retv += '球员十一　'
	retv += '<input type="text" name="tmember11" maxlength="6"><br />';
	retv += '球员六　　'
	retv += '<input type="text" name="tmember6" maxlength="6">　　';
	retv += '球员十二　'
	retv += '<input type="text" name="tmember12" maxlength="6"><br/><br/>';
	retv += '<input type="button" onclick="document.getElementById(';
	retv += '\'bbsfb_admin_main\').innerHTML=\'\';" value="放弃">　　';
	retv += '<input type="submit" value="新增"><br /><br />';
	retv += '</div></form><br />';

	return retv; 
}

function bbsfb_admin_auto(pos) {
	var retv;


	retv  = '<form action="bbsfb?A=9&T=6" method="post">';
	retv += '<div align="center" class="annbox">';
	retv += '<h3>自动安排赛程</h3>';
	retv += '这项操作将在当前位置自动安排赛程。<br /><br />';
	retv += '<input type="hidden" name="posstr" value="' + pos + '">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '最终队伍数量　　<input type="text" name="result"><br />';
	retv += '初始队伍数量　　<input type="text" name="total"><br />';
	retv += '<input type="button" onclick="document.getElementById(';
	retv += '\'bbsfb_admin_main\').innerHTML=\'\';" value="放弃">　　';
	retv += '<input type="submit" value="生成"><br /><br />';
	retv += '</div></form><br />';

	return retv;
}

function bbsfb_admin(pos, name) {
	var obj = document.getElementById('bbsfb_admin_main');
	var retv;

	if (!obj)
		return;

	retv  = '<h2>管理节点</h2><br />';
	retv += '<div class="annbox" align="center">当前节点是[' + name + ']，';
	retv += '内部标识[' + pos + ']</div>';
	obj.innerHTML  = retv;
	obj.innerHTML += bbsfb_admin_addgraph(pos);
	obj.innerHTML += bbsfb_admin_delgraph(pos);
	obj.innerHTML += bbsfb_admin_renamegraph(pos);
	obj.innerHTML += bbsfb_admin_addteam(pos);
	obj.innerHTML += bbsfb_admin_auto(pos);
}

function bbsfb_admin_editteam(num, pos) {
	var retv;

	retv  = '<form action="bbsfb?A=9&T=4" method="post">';
	retv += '<div align="center" class="annbox">';
	retv += '<h3>编辑赛队</h3>';
	retv += '这项操作将在修改当前位置的球队。<br /><br />';
	retv += '<input type="hidden" name="posstr" value="' + pos +'">';
	retv += '<input type="hidden" name="tnum" value="' + num + '">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '赛队名称　'
	retv += '<input type="text" name="tname" maxlength="15">　　';
	retv += '学校名称　'
	retv += '<input type="text" name="tuniv" maxlength="15"><br />';
	retv += '球员一　　'
	retv += '<input type="text" name="tmember1" maxlength="6">　　';
	retv += '球员七　　'
	retv += '<input type="text" name="tmember7" maxlength="6"><br />';
	retv += '球员二　　'
	retv += '<input type="text" name="tmember2" maxlength="6">　　';
	retv += '球员八　　'
	retv += '<input type="text" name="tmember8" maxlength="6"><br />';
	retv += '球员三　　'
	retv += '<input type="text" name="tmember3" maxlength="6">　　';
	retv += '球员九　　'
	retv += '<input type="text" name="tmember9" maxlength="6"><br />';
	retv += '球员四　　'
	retv += '<input type="text" name="tmember4" maxlength="6">　　';
	retv += '球员十　　'
	retv += '<input type="text" name="tmember10" maxlength="6"><br />';
	retv += '球员五　　'
	retv += '<input type="text" name="tmember5" maxlength="6">　　';
	retv += '球员十一　'
	retv += '<input type="text" name="tmember11" maxlength="6"><br />';
	retv += '球员六　　'
	retv += '<input type="text" name="tmember6" maxlength="6">　　';
	retv += '球员十二　'
	retv += '<input type="text" name="tmember12" maxlength="6"><br/><br/>';
	retv += '<input type="button" onclick="document.getElementById(';
	retv += '\'bbsfb_admin_main\').innerHTML=\'\';" value="放弃">　　';
	retv += '<input type="submit" value="修改"><br /><br />';
	retv += '</div></form><br />';

	return retv; 
}

function bbsfb_admin_delteam(num) {
	var retv;

	retv  = '<form action="bbsfb?A=9&T=5" method="post" ';
	retv += 'onsubmit="return confirm(\'确定删除？\');">';
	retv += '<div align="center" class="annbox">';
	retv += '<h3>删除</h3>';
	retv += '这项操作将删除当前位置的球队，应用后将不可恢复';
	retv += '请慎重。<br /><br />';
	retv += '<input type="hidden" name="tnum" value="' + num + '">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '<input type="button" onclick="document.getElementById(';
	retv += '\'bbsfb_admin_main\').innerHTML=\'\';" value="放弃">　　';
	retv += '<input type="submit" value="删除"><br /><br />';
	retv += '</div></form><br />';

	return retv; 
}

function bbsfb_admin_team(name, num, pos) {
	var obj = document.getElementById('bbsfb_admin_main');
	var retv;

	if (!obj)
		return;

	retv  = '<h2>管理赛队</h2><br />';
	retv += '<div class="annbox" align="center">当前队伍是[' + name +']';
	retv += '，内部编号[' + num + ']，位置[' + pos +']<div>';

	obj.innerHTML  = retv;
	obj.innerHTML += bbsfb_admin_editteam(num, pos);
	obj.innerHTML += bbsfb_admin_delteam(num);
}

function bbsfb_show_team(name, num, admin, pos, dot) {
	var obj = document.getElementById('bbsfb_graph_' +pos+ '_right_below');
	var retv;

	if (!obj)
		return;
	if (num == 0)
		return;

	retv  = '<div align="right">';
	retv += '<table cellspacing=0 cellpadding=0><tr>';
	retv += '<td id="team_' + pos + '">';
	retv += '<div class="team_text">' + name;
	if (admin) {
		retv += '<br /><br />[<a href="#" ';
		retv += 'onclick="bbsfb_admin_team(\'';
		retv += name +'\', ' + num + ', \'' + pos + '\');">管理</a>]';
	}
	retv += '</div></td>';
	retv += '<td><img width="8px" src="/football/dot.gif" ';
	retv += 'border=0 height=17 /></td>';
	retv += '<td><img width=1 src="/football/dot' + dot +'.gif" ';
	if (dot) {
		retv += 'onload="this.style.height=document.getElementById(\'';
		retv += 'team_' + pos + '\').offsetHeight;">';
	} else
		retv += '>';
	retv += '</td></tr></table></div>';

	obj.innerHTML += retv;
}

function gs(admin, name, pos, status, dottype, nsub,
		team, teamname, team1, team1name, team2, team2name, score1, 
		score2) {
	var id = 'bbsfb_graph_' + pos;
	var parentid;
	var retv;
	var obj;
	var dot;

	if (status & 0x1)		//deleted
		return;
	retv  = '';
	if (pos == rootstr) {
		retv += '<h2>对阵形式图</h2>';
		retv += '<div align=left>';
	} else 
		retv += '<div align=left>';
	retv += '<table cellpadding=0 cellspacing=0><tr>';
	retv += '<td id="' + id + '_right" class="graph_right">';
	retv += '<div id="' + id + '_right_up"></div>';
	retv += '<div id="' + id + '_right_below"></div></td>';
	retv += '<td><img width="8px" src="/football/dot.gif" ';
	retv += 'border=0 height=17 /></td>';
	retv += '<td id="' + id + '_left" class="graph_left">';
	retv += '<div class="graph_left_text"><div class="graph_left_text2">'
	retv += '<span class="block">';
	if (team)
		retv += teamname + '<br />' + '[' + score1 + ':' + score2 +']';
	else if (name) {
		retv += name;
	} else
		retv += '<img src="/football/unknown.gif">';
	retv += '</span>';
	if (admin && !/^[A-Z].*$/.test(secstr) && pos != '') {
		retv += '<div align=center><br />';
		retv += '<a href="bbsfb?A=9&G=' + pos[0] + '">';
		retv += '[管理]</a></div>';
	} else if (admin) {
		retv += '<div align=center><br />';
		retv += '<a href="#" onclick="bbsfb_admin(\'';
		retv += pos +'\', \'' + name +'\');">[管理]</a></div>';
	}
	retv += '</div></div></td>';
	if (pos != rootstr) {
		retv += '<td><img width="8px" src="/football/dot.gif" ';
		retv += 'border=0 height=17 /></td>';
	}
	retv += '<td id="' + id +'"><img width=1 src="';
	retv += '/football/dot' + dottype + '.gif" ';
	if (dottype) {
		retv += 'onload="this.style.height=';
		retv += 'document.getElementById(\'' + id + '\').offsetHeight"';
	} else
		retv += '/></td>';
	retv += '</tr></table>';
	retv += '</div>';

	if (pos == '') {
		parentid  = 'bbsfb_graph_main';
	} else {
		parentid  = 'bbsfb_graph_' + pos.substring(0, pos.length - 1);
		parentid += '_right_up';
	}


	obj = document.getElementById(parentid);
	if (!obj)
		return;
	obj.innerHTML += retv;
	if (nsub && team2)
		dot = 2;
	else if (!nsub && team2)
		dot = 1;
	else if (!nsub && !team2)
		dot = 0;
	else if (nsub && !team2)
		dot = 3;
	
	bbsfb_show_team(team1name, team1, admin, pos, dot);
	if (team1 || nsub)
		dot = 3;
	else
		dot = 0;
	bbsfb_show_team(team2name, team2, admin, pos, dot);
	return;
}

function change_graph(sec) {
	var obj = document.getElementById('bbsfb_graph_box');

	if (!obj)
		return;

	obj.style.visibility = 'hidden';
	obj.src = 'bbsfb?A=1&G=' + sec;

	return;
}

function bbsfb_index() {
	return;
}

function bbsfb_adjust_frame() {
	var obj = document.getElementById('bbsfb_graph_box');

	if (!obj)
		return;
	obj.style.width = '100%';
	if (obj.contentDocument) {
		obj.style.height = obj.contentDocument.body.offsetHeight + 50
					+ 'px';
		obj.style.width = '100%';
	} else if (obj.Document) {
		obj.style.height = document.body.offsetHeight + 'px';
	}
	obj.style.visibility = '';
}

function bbsfb_import_teams() {
	var obj = document.getElementById('bbsfb_admin_main');
	var retv;

	if (!obj)
		return;

	retv  = '<h3>导入赛队</h3>';
	retv += '<form action="bbsfb?A=9&T=8" method="post" ';
	retv += 'id="bbsfb_import_form" onsubmit="bbsfb_import_filter();">';
	retv += '<p>请输入起始队伍的内部编号 ';
	retv += '<input name="start" maxlength=10></p>';
	retv += '<input name="content" type="hidden">';
	retv += '<input name="G" type="hidden" value="' + secstr +'">';
	retv += '<textarea name="import_textarea" id="bbsfb_import_textarea" ';
	retv += 'style="width: 100%; height: 400px;"></textarea>';
	retv += '<p align=right><input type="submit" value="提交">';
	retv += '</p></form>';

	obj.innerHTML = retv;
}

function bbsfb_import_filter() {
	var obj = document.getElementById('bbsfb_import_form');

	if (!obj) 
		return;
	
	obj.content.value = obj.import_textarea.value.replace(/\r\n/gi, '\n');

	return;
}

function ri(sec, name) {
	var retv = '';
	admin_print_title(sec);

	retv += '<br /><br /><h2>更新 ' + name + ' 赛区战报</h2>';

	retv += '<table width="100%" cellspacing=2 cellpadding=2 border=0>';
	retv += '<form method="post" action="bbsfb?A=9&T=10&G=' + sec + '">';
	retv += '<tr style="background-color: #eee; text-align: center; ';
	retv += 'font-weight: bold; height: 21px;"';
	retv += '<td>强行晋级</td><td>队伍一</td><td>强行晋级</td>';
	retv += '<td>队伍二</td><td>比分</td></tr>';

	document.write(retv);
}

function rf(num, posstr, team1, team1name, team2, team2name) {
	var retv = '';

	retv += '<tr style="background-color: ';
	retv += (num % 2 ? '#eee;' : 'efe');
	retv += '; text-align: center">';
	retv += '<td><input name="force_' + num + '" type="radio" ';
	retv += 'value="' + team1 + '"></td>';
	retv += '<td>' + team1name + '</td>';
	retv += '<td><input name="force_' + num + '" type="radio" ';
	retv += 'value="' + team2 + '"></td>';
	retv += '<td>' + team2name + '</td>';
	retv += '</td><td><input name="score1_' + num + '" maxlengh=2 type=';
	retv += '"text" style="width: 25px"> - <input name="score2_' + num;
	retv += '" maxlengh=2 text="text" style="width: 25px"></td></tr>'
	retv += '<input type="hidden" name="team1_' + num + '" ';
	retv += 'value="' + team1 + '">';
	retv += '<input type="hidden" name="team2_' + num + '" ';
	retv += 'value="' + team2 + '">';
	retv += '<input type="hidden" name="posstr_' + num +'" ';
	retv += 'value="' + posstr + '">';

	document.write(retv);
}

function re(num) {
	var retv = '';

	retv += '<input name="total" type="hidden" value="' + num +'"';
	retv += '<tr style="text-align: center;"><td colspan=4></td><td><br />';
	retv += '<input type="reset" value="清除"> 　　';
	retv += '<input type="submit" value="更新"></td></tr>';
	retv += '</table>';

	document.write(retv);
}
