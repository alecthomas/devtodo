<?xml version="1.0"?>

<!--

	After being disappointed in the printed output from a color xterm 
	screenshot, I generated an XSLT/FOP stylesheet that dumps out color 
	pdf, suitable for printing. I include the wrapper (for running it on 
	a debian unstable system - note that I haven't found anywhere to get 
	the w3c.jar other than the upstream FOP sources, nor anywhere else to 
	find org/w3c/dom/svg/SVGFitToViewBox...) and the stylesheet here, for 
	your enjoyment; do whatever you'd like with them. (In practice they 
	need more tuning of the colors, and I left out margin settings 
	altogether.) 

	_Mark_ <eichin@thok.org> 
	The Herd Of Kittens 

-->

<xsl:stylesheet xsl:version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xlink="http://www.w3.org/2000/xlink/namespace/"
	xmlns:fo="http://www.w3.org/1999/XSL/Format">

	<xsl:output xsl:method="xml" xsl:indent="yes"/>
	<xsl:strip-space xsl:elements="item bulletlist"/>
	<xsl:preserve-space xsl:elements="preformatted"/>

	<!-- body -->
	<xsl:template xsl:match="/">
		<fo:root xmlns:fo="http://www.w3.org/1999/XSL/Format">
			<fo:layout-master-set>
				<fo:simple-page-master
					master-name="todoPage"
					page-height="11in"
					page-width="8.5in">
					<fo:region-body/>
				</fo:simple-page-master>
			</fo:layout-master-set>
			<fo:page-sequence fo:master-name="todoPage">
				<fo:flow fo:flow-name="xsl-region-body">
					<fo:list-block>
						<xsl:call-template xsl:name="noteList"/>
					</fo:list-block>
				</fo:flow>
			</fo:page-sequence>
		</fo:root>
	</xsl:template>

	<xsl:template xsl:name="noteList">
		<xsl:for-each xsl:select="todo/note">
			<xsl:call-template xsl:name="noteItem"/>
		</xsl:for-each>
	</xsl:template>

	<xsl:template xsl:name="noteItem">
		<xsl:if xsl:test="not(@done)">
			<fo:list-item>
				<fo:list-item-label>
					<fo:block>
						<xsl:value-of xsl:select="position()" />
					</fo:block>
				</fo:list-item-label>
				<fo:list-item-body>
					<xsl:apply-templates xsl:select="."/>
				</fo:list-item-body>
			</fo:list-item>
		</xsl:if>
	</xsl:template>

	<xsl:template xsl:match="note">
		<xsl:call-template xsl:name="baseNote"/>
		<fo:list-block>
			<xsl:for-each xsl:select="./note">
				<xsl:call-template xsl:name="noteItem"/>
			</xsl:for-each>
		</fo:list-block>
	</xsl:template>

	<xsl:template xsl:name="baseNote">
		<xsl:variable xsl:name="priorityColor">
			<xsl:choose>
				<xsl:when xsl:test="@priority = 'veryhigh'">red</xsl:when>
				<xsl:when xsl:test="@priority = 'high'">yellow</xsl:when>
				<xsl:when xsl:test="@priority = 'medium'">orange</xsl:when>
				<xsl:when xsl:test="@priority = 'low'">#000090</xsl:when>
				<xsl:when xsl:test="@priority = 'verylow'">blue</xsl:when>
				<xsl:otherwise>black</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<fo:block fo:color="{$priorityColor}">
			<xsl:value-of xsl:select="child::text()[position()=1]"/>
		</fo:block>
	</xsl:template>

</xsl:stylesheet>
