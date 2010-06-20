doc:=attachment.cs browser.cs changeset.cs error.cs file.cs footer.cs header.cs log.cs log_rss.cs \
macros.cs milestone.cs newticket.cs query.cs README report.cs report_rss.cs roadmap.cs search.cs \
settings.cs ticket.cs ticket_notify_email.cs timeline.cs timeline_rss.cs wiki.cs
targerdir=/usr/share/trac/templates-my
.PHONY:install
install:
	for i in ${doc};do if [ ${targerdir}/$$i -ot $$i ]; then iconv $$i -f gb2312 -t utf-8 > ${targerdir}/$$i;fi;done
