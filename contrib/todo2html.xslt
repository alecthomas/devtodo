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

	<xsl:output xsl:method="xml" xsl:indent="yes"/>
	<xsl:strip-space xsl:elements="item bulletlist"/>
	<xsl:preserve-space xsl:elements="preformatted"/>

	<!-- body -->
	<xsl:template xsl:match="/">
		<html>
		<title>
			<xsl:variable xsl:name="title" select="todo/title"/>
			<xsl:choose> 
				<xsl:when xsl:test = '$title=""'>
					Todo list
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of xsl:select="todo/title"/>
				</xsl:otherwise>
			</xsl:choose>
		</title>
		<body bgcolor="white" color="black">
			<font color="black">
			<center>
			<h1>
			<xsl:variable xsl:name="title" select="todo/title"/>
			<xsl:choose> 
				<xsl:when xsl:test = '$title=""'>
					Todo list
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of xsl:select="todo/title"/>
				</xsl:otherwise>
			</xsl:choose>
			</h1>
			</center>
			<ul>
			<xsl:call-template xsl:name="noteList"/>
			</ul>
			</font>
		</body>
		</html>
	</xsl:template>

	<xsl:template xsl:name="noteList">
		<xsl:for-each xsl:select="todo/note">
			<xsl:call-template xsl:name="noteItem"/>
		</xsl:for-each>
	</xsl:template>

	<xsl:template xsl:name="noteItem">
		<!-- Uncomment this to NOT display completed items -->
<!--		<xsl:if not(@done)">-->
			<li type="disc">
				<xsl:apply-templates xsl:select="."/>
			</li>
<!--		</xsl:if>-->
	</xsl:template>

	<xsl:template xsl:match="note">
		<xsl:call-template xsl:name="baseNote"/>
		<xsl:for-each xsl:select="./note">
			<ul>
			<xsl:call-template xsl:name="noteItem"/>
			</ul>
		</xsl:for-each>
	</xsl:template>

	<xsl:template xsl:name="baseNote">
		<xsl:variable xsl:name="priorityColor">
			<xsl:choose>
				<xsl:when xsl:test="@priority = 'veryhigh'">red</xsl:when>
				<xsl:when xsl:test="@priority = 'high'">orange</xsl:when>
				<xsl:when xsl:test="@priority = 'medium'">black</xsl:when>
				<xsl:when xsl:test="@priority = 'low'">blue</xsl:when>
				<xsl:when xsl:test="@priority = 'verylow'">darkblue</xsl:when>
				<xsl:otherwise>black</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<!-- Done -->
		<xsl:if xsl:test="@done">
			<s>
			<font color="{$priorityColor}">
				<xsl:value-of xsl:select="child::text()"/>
			</font>
			</s>
		</xsl:if>

		<!-- Not done -->
		<xsl:if xsl:test="not(@done)">
			<font color="{$priorityColor}">
				<xsl:value-of xsl:select="child::text()"/>
			</font>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
