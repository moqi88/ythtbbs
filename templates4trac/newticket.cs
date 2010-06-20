<?cs set:html.stylesheet = 'css/ticket.css' ?>
<?cs include "header.cs" ?>
<?cs include "macros.cs" ?>
<script type="text/javascript">
addEvent(window, 'load', function() { document.getElementById('summary').focus()}); 
</script>

<div id="ctxtnav" class="nav"></div>

<div id="content" class="ticket">

<h3>报告问题:</h3>
<form id="newticket" action="<?cs var:cgi_location ?>#preview" method="post">
 <div class="field">
  <label for="reporter">你的用户名 或者 email 地址:</label><br />
  <input type="text" id="reporter" name="reporter" size="40" value="<?cs
    var:newticket.reporter ?>" /><br />
 </div>
 <div class="field">
  <label for="summary">简单描述:</label><br />
  <input id="summary" type="text" name="summary" size="80" value="<?cs var:newticket.summary ?>"/>
 </div>
 <div class="field">
  <label for="description">详细说明 (你也可以用 <a tabindex="42" href="<?cs
    var:$trac.href.wiki ?>/WikiFormatting">WikiFormatting</a> ):</label><br />
  <textarea id="description" name="description" rows="10" cols="78"><?cs
    var:newticket.description ?></textarea><?cs
  call:wiki_toolbar('description') ?><?cs
  if:newticket.description_preview ?>
   <fieldset id="preview">
    <legend>Description Preview</legend>
    <?cs var:newticket.description_preview ?>
   </fieldset><?cs
  /if ?>
 </div>

 <fieldset id="properties">
  <legend>Ticket Properties</legend>
  <input type="hidden" name="mode" value="newticket" />
  <input type="hidden" name="action" value="create" />
  <input type="hidden" name="status" value="new" />
  <div class="col1">
   <label for="component">模块:</label><?cs
   call:hdf_select(newticket.components, "components", newticket.component) ?>
   <br />
   <label for="version">版本:</label><?cs
   call:hdf_select(newticket.versions, "versions", newticket.version) ?>
   <br />
   <label for="severity">严重程度:</label><?cs
   call:hdf_select(enums.severity, "severity", newticket.severity) ?>
   <br />
   <label for="keywords">关键词:</label>
   <input type="text" id="keywords" name="keywords" size="20"
       value="<?cs var:newticket.keywords ?>" />
  </div>
  <div class="col2">
   <label for="priority">优先级:</label><?cs
   call:hdf_select(enums.priority, "priority", newticket.priority) ?><br />
   <label for="milestone">里程碑:</label><?cs
   call:hdf_select(newticket.milestones, "milestone", newticket.milestone) ?><br />
   <label for="owner">安排给:</label>
   <input type="text" id="owner" name="owner" size="20" value="<?cs
     var:newticket.owner ?>" /><br />
   <label for="cc">抄送:</label>
   <input type="text" id="cc" name="cc" size="30" value="<?cs var:newticket.cc ?>" />
  </div>
  <?cs if:len(ticket.custom) ?><div class="custom">
   <?cs call:ticket_custom_props(ticket) ?>
  </div><?cs /if ?>
 </fieldset>

 <div class="buttons">
  <input type="submit" value="预览" />&nbsp;
  <input type="submit" name="create" value="提交" />
 </div>
</form>

 <div id="help">
  <strong>Note:</strong> See <a href="<?cs var:$trac.href.wiki
  ?>/TracTickets">TracTickets</a> for help on using tickets.
 </div>
</div>

<?cs include "footer.cs" ?>
