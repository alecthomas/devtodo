<?xml version="1.0"?>

<!--
	XSLT transform for making a colour HTML file out of a devtodo XML database.

	This was half derived from the XSLT transform from Daniel Patterson and
	half from the transform by Mark Eichin.

	It will output the todo database as colourised HTML, with done items struck
	out.

	If anybody has ANY enhancements to this file, PLEASE send them to me, as I
	have very little clue WRT XSLT and this file is just a hack.

	I generate HTML output with the following line (via libxslt):

		xsltproc todo-html.xslt .todo > ../todo.html
-->

<xsl:stylesheet xsl:version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output xsl:method="html" xsl:indent="yes"/>
	<xsl:strip-space xsl:elements="item bulletlist"/>

	<!-- body -->
	<xsl:template xsl:match="/">
  <!-- <xsl:apply-templates xsl:select="todo/title"/> -->
		<xsl:for-each xsl:select="todo/note|todo/link">
			<xsl:call-template xsl:name="noteItem"/>
		</xsl:for-each>
	</xsl:template>

 <xsl:template xsl:match="todo">
  <xsl:apply-templates xsl:select="title"/>
  <blockquote>
   <xsl:for-each xsl:select="note|link">
    <xsl:call-template xsl:name="noteItem"/>
   </xsl:for-each>
  </blockquote>
 </xsl:template>

 <xsl:template xsl:match="title">
  <p><b><xsl:value-of select="normalize-space(.)"/>:</b></p>
 </xsl:template>

	<xsl:template xsl:name="noteItem">
		<xsl:if test="not(@done)">
   <xsl:apply-templates xsl:select="."/>
		</xsl:if>
	</xsl:template>

	<xsl:template xsl:match="note">
  <xsl:choose>
   <xsl:when test="count(./note|link) > 0">
    <b>
     <xsl:call-template xsl:name="baseNote">
      <xsl:with-param name="append">:</xsl:with-param>
     </xsl:call-template>
    </b>
   </xsl:when>
   <xsl:otherwise>
    -- <xsl:call-template xsl:name="baseNote"/>
   </xsl:otherwise>
  </xsl:choose>
  <blockquote>
		<xsl:for-each xsl:select="note|link">
			<xsl:call-template xsl:name="noteItem"/>
		</xsl:for-each>
  </blockquote>
	</xsl:template>

 <xsl:template xsl:match="link">
  <xsl:apply-templates select="document(@filename)/todo"/>
 </xsl:template>

	<xsl:template xsl:name="baseNote">
  <xsl:param name="append"/>
		<xsl:variable xsl:name="priorityColor">
			<xsl:choose>
				<xsl:when xsl:test="@priority = 'veryhigh'">#FF0000</xsl:when>
				<xsl:when xsl:test="@priority = 'high'">#660000</xsl:when>
				<xsl:when xsl:test="@priority = 'medium'">#000000</xsl:when>
				<xsl:when xsl:test="@priority = 'low'">#0000F0</xsl:when>
				<xsl:when xsl:test="@priority = 'verylow'">#000066</xsl:when>
				<xsl:otherwise>#000000</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<!-- Done -->
		<xsl:if xsl:test="@done">
			<s>
			<font color="{$priorityColor}">
				<xsl:value-of xsl:select="normalize-space(child::text())"/>
    <xsl:value-of xsl:select="$append"/>
			</font>
			</s>
		</xsl:if>

		<!-- Not done -->
		<xsl:if xsl:test="not(@done)">
			<font color="{$priorityColor}">
				<xsl:value-of xsl:select="normalize-space(child::text())"/>
    <xsl:value-of xsl:select="$append"/>
			</font>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
