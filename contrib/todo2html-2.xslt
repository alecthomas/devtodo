<?xml version="1.0"?> 

<!--
	XSLT example courtesy of Daniel Patterson, posted to the devtodo forums
	at http://sourceforge.net/forum/forum.php?thread_id=114904&forum_id=69460

	This example, AFAIK, generates HTML from the devtodo XML database.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xsl:version="1.0">
	<xsl:output xsl:method="html" xsl:indent="no"/> 

	<xsl:template xsl:match="todo"> 
		<xsl:call-template xsl:name="note-list"/> 
	</xsl:template> 

	<xsl:template xsl:name="note-list"> 
		<ul> 
			<xsl:apply-templates xsl:select="note"/> 
		</ul> 
	</xsl:template> 

	<xsl:template xsl:match="note"> 
		<li><xsl:value-of xsl:select="."/> 
		<xsl:if xsl:test="count(note) > 0">
			<xsl:call-template xsl:name="note-list"/> 
		</xsl:if>
		</li> 
	</xsl:template> 
</xsl:stylesheet> 
