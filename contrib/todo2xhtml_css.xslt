<?xml version="1.0"?>

<!--
    todo2xhtml_css - convert a devtodo database to a CSS+XHTML page

    Version:  0.9

    Copyright (c) 2004-2005 Francesco Poli, <frx@firenze.linux.it>
    Copyright (c) 2005 Arthur Korn, <arthur@korn.ch>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:
    
    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    As a special exception, when parts of this file are copied by the XSLT
    transformation into an output file, you may use that output file without
    restriction.
-->

<!--
    Suggested usage:
    a) put todo2xhtml_css.xslt and todo.css in /usr/local/share/devtodo/
    b) put something like the following in your ~/.todorc

    # When saving a database, also create an XHTML+CSS version as .todo.html.
    # This requires the libxslt library from xmlsoft.org in addition to the
    # XSLT and CSS files in the devtodo contrib directory.
    on save {
       echo XHTML+CSS created
       exec if test ! -e $HOME/.todo.css ; then cp /usr/local/share/devtodo/todo.css $HOME/.todo.css ; fi
       exec if test ! -e `dirname $TODODB`/.todo.css ; then cp $HOME/.todo.css `dirname $TODODB`/.todo.css ; fi
       exec xsltproc /usr/local/share/devtodo/todo2xhtml_css.xslt $TODODB > `dirname $TODODB`/.todo.html
       exec chmod 600 `dirname $TODODB`/.todo.html
    }


    Rationale:
    The XSLT transformation generates an XHTML page representing the todo
    list; this XHTML page refers to an external CSS stylesheet to be
    found in the same directory.
    The XHTML page will be updated after every todolist database change.
    A user-specific CSS stylesheet will be copied from the system-wide
    default one, *only* if it is not already in the user's home directory.
    The CSS stylesheet in the current directory will be copied from the
    user-specific one, *only* if it is not already there.
    The user can customize (or even completely replace) his/her ~/.todo.css
    of even some local ./.todo.css : they will not be touched at all, as
    long as they are present with the right filename.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml" indent="yes" />

<xsl:template match="/">
<xsl:text disable-output-escaping = "yes">&lt;!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"&gt;

</xsl:text>
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
  <head>
    <title>Todo list</title>
<style type="text/css">
<xsl:text disable-output-escaping = "yes">
/*&lt;![CDATA[*/
@import "./.todo.css";
/*]]&gt;*/
</xsl:text>
</style>
<script type="text/javascript">
<xsl:text disable-output-escaping = "yes">
//&lt;![CDATA[
&lt;!--
function updateDisplay(tag, tag_class, new_display)
{
    var tags = document.getElementsByTagName(tag);
    for (var i=0; i&lt;tags.length; i++)
    {
        var thing = tags[i];
        if (thing.className == tag_class)
        {
            thing.style.display = new_display;
        }
    }
}
//--&gt;

//]]&gt;
</xsl:text>
</script>
  </head>
  <body onload="updateDisplay('a', 'visibility', 'inline')">
    <div id="outercontainer">
      <div id="innercontainer">
        <div id="outertitle">
          <div id="innertitle">
            <h1>Todo list</h1>
          </div>
        </div>
        <div id="outerlegend">
          <div id="innerlegend">
            <ol>
              <li class="veryhigh"><a class="visibility"
              href="#" title="show done items"
              onclick="updateDisplay('li', 'veryhighdone', 'list-item')">s</a>
              <a class="visibility" href="#" title="hide done items"
              onclick="updateDisplay('li', 'veryhighdone', 'none')">h</a>
              veryhigh</li>
              <li class="high"><a class="visibility"
              href="#" title="show done items"
              onclick="updateDisplay('li', 'highdone', 'list-item')">s</a>
              <a class="visibility" href="#" title="hide done items"
              onclick="updateDisplay('li', 'highdone', 'none')">h</a>
              high</li>
              <li class="medium"><a class="visibility"
              href="#" title="show done items"
              onclick="updateDisplay('li', 'mediumdone', 'list-item')">s</a>
              <a class="visibility" href="#" title="hide done items"
              onclick="updateDisplay('li', 'mediumdone', 'none')">h</a>
              medium</li>
              <li class="low"><a class="visibility"
              href="#" title="show done items"
              onclick="updateDisplay('li', 'lowdone', 'list-item')">s</a>
              <a class="visibility" href="#" title="hide done items"
              onclick="updateDisplay('li', 'lowdone', 'none')">h</a>
              low</li>
              <li class="verylow"><a class="visibility"
              href="#" title="show done items"
              onclick="updateDisplay('li', 'verylowdone', 'list-item')">s</a>
              <a class="visibility" href="#" title="hide done items"
              onclick="updateDisplay('li', 'verylowdone', 'none')">h</a>
              verylow</li>
            </ol>
            <ol>
              <li class="veryhighdone">done veryhigh</li>
              <li class="highdone">done high</li>
              <li class="mediumdone">done medium</li>
              <li class="lowdone">done low</li>
              <li class="verylowdone">done verylow</li>
            </ol>
          </div>
        </div>
        <div id="outerlist">
          <div id="innerlist">
            <xsl:apply-templates />
          </div>
        </div>
      </div>
    </div>
  </body>
</html>
</xsl:template>

<xsl:template match="/todo">
  <xsl:call-template name="list_of_notes" />
</xsl:template>

<xsl:template name="list_of_notes">
<ol>
  <xsl:for-each select="./note">
    <xsl:apply-templates select="." />
  </xsl:for-each>
</ol>
</xsl:template>

<xsl:template match="note">
  <xsl:element name="li">
    <xsl:attribute name="class">
      <xsl:value-of select="@priority" />
      <xsl:if test="@done">done</xsl:if>
    </xsl:attribute>
    <xsl:value-of select="child::text()" />
    <xsl:if test="./note">
      <xsl:call-template name="list_of_notes" />
    </xsl:if>
  </xsl:element>
</xsl:template>

</xsl:stylesheet>
