bug_change_reasons = new Array(
	new Array(
		new Array(new Array("新建"), 
			  null, 
			  new Array("接受解决方案")),
		new Array(null, null, null),
		new Array(null, null, null)
	),
	new Array(
		new Array(new Array("新建"), 
			  new Array("已修复", 
				    "该现象为设计特性",
				    "重复提交此错误",
                                    "无法重现此错误"),
                          null),
                new Array(new Array("重新激活", "不接受这个解决方案", "错误的修改"),
                          null,
                          null),
		new Array(null, null, null)
	)
);	

function get_index(str)
{
	if(str == "活跃")
		return 0;
	if(str == "解决")
		return 1;
	if(str == "关闭")
		return 2;
	return 0;
}

function fill_owner(selectCtrl, ps, ns, owner, reporter) {
	var i, prev, next;
	var def;
	if(ps == "活跃" && ns == "解决")
		def = reporter;
	else
		def = owner;

	for(i = 0; i < selectCtrl.options.length; i++){
		if( selectCtrl.options[i].value != def )
				selectCtrl.options[i].selected = false;
		else
				selectCtrl.options[i].selected = true;
	}
}

function fill_reason(selectCtrl, type, ps, ns, last_reason) {
	var i, prev, next;
	for (i = selectCtrl.options.length; i >= 0; i--) {
		selectCtrl.options[i] = null; 
	}
	prev =  get_index(ps);
	next =  get_index(ns);
	if(type == 1 && prev == 1 && next == 2){
		if( last_reason == "已修复" ){
			selectCtrl.options[0] = new Option("确认修改");
		}
		else {
			selectCtrl.options[0] = new Option(last_reason);
		}
			
	} else {
		for (i = 0; i < bug_change_reasons[type][prev][next].length; i++) {
			selectCtrl.options[i] = 
				new Option(bug_change_reasons[type][prev][next][i]);
			selectCtrl.options[i].value = 
				bug_change_reasons[type][prev][next][i];
		}
	}
	selectCtrl.options[0].selected = true;
}
