/**
 * Copyright (C) 2002-2003, CodeHouse.com. All rights reserved.
 * CodeHouse(TM) is a registered trademark.
 *
 * THIS SOURCE CODE MAY BE USED FREELY PROVIDED THAT
 * IT IS NOT MODIFIED OR DISTRIBUTED, AND IT IS USED
 * ON A PUBLICLY ACCESSIBLE INTERNET WEB SITE.
 * 
 * CodeHouse.com JavaScript Library Module: Cookie Utility Class
 *
 * You can obtain this script at http://www.codehouse.com
 */
function CJL_CookieUtil(name, duration, path, domain, secure)
{
   this.affix = "";
   
   if( duration )
   {   	  
      var date = new Date();
	  var curTime = new Date().getTime();

	  date.setTime(curTime + (1000 * 60 * 24 * duration));
	  this.affix = "; expires=" + date.toGMTString();
   }
   
   if( path )
   {
      this.affix += "; path=" + path;
   }
   
   if( domain )
   {
      if (/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/.test(domain))
	      this.affix += "; domain=" + domain;
      else {
	      if (/\.(com|net|org)$/i.test(domain)) {
		      domain = domain.replace(/(.*)(\.[^\.]*)(\.com|\.net|\.org)$/i, '.$2$3');
		      this.affix += "; domain=" + domain;
	      } else if (/\.(com|net|org)\.cn$/.test(domain)) {
		      domain = domain.replace(/(.*)(\.[^\.]*)(\.com|\.net|\.org)(\.cn)$/i, '.$2$3$4');
		      this.affix += "; domain=" + domain;
	      }
      }
   }

   if( secure )
   {
      this.affix += "; secure=" + secure;
   }
   
      
   function getValue()
   {
      var m = document.cookie.match(new RegExp("(" + name + "=[^;]*)(;|$)"));

      return m ? m[1] : null;   
   }
   
   this.cookieExists = function()
   {
      return getValue() ? true : false;
   }
      
   this.expire = function()
   {
      var date = new Date();
	  date.setFullYear(date.getYear() - 1);
	  document.cookie=name + "=noop; expires=" + date.toGMTString(); 
   }
        
   this.setSubValue = function(key, value)
   {
      var ck = getValue();

      if( /[;, ]/.test(value) )
      {
         //Mac IE doesn't support encodeURI
		 value = window.encodeURI ? encodeURI(value) : escape(value);
      }

      
      if( value )
      {
         var attrPair = "@" + key + value;

         if( ck )
         {
             if( new RegExp("@" + key).test(ck) )
	         {
		        document.cookie =
				   ck.replace(new RegExp("@" + key + "[^@;]*"), attrPair) + this.affix;
	         }
	         else
	         {
		        document.cookie =
				   ck.replace(new RegExp("(" + name + "=[^;]*)(;|$)"), "$1" + attrPair) + this.affix;
	         }
         }
         else
         {
	        document.cookie = name + "=" + attrPair + this.affix;
         }
      }
      else
      {      
	     if( new RegExp("@" + key).test(ck) )
	     {
	        document.cookie = ck.replace(new RegExp("@" + key + "[^@;]*"), "") + this.affix;
	     }
      }
   }

      
   this.getSubValue = function(key)
   {
      var ck = getValue();

      if( ck )
      {
         var m = ck.match(new RegExp("@" + key + "([^@;]*)"));

	     if( m )
	     {
	        var value = m[1];

	        if( value )
	        { 
	           //Mac IE doesn't support decodeURI
			   return window.decodeURI ? decodeURI(value) : unescape(value);
	        }
	     }
      }
   }
}
