<?xml version="1.0" encoding="UTF-8"?> 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">

<xsl:for-each select="/messages/message">

<p>
<xsl:text>From: </xsl:text>
<a>
<xsl:attribute name="href">contact://<xsl:value-of select="source/contact_id" /></xsl:attribute>
<xsl:value-of select="source/contact_name" />
</a>
</p>

<p>
<xsl:text>To: </xsl:text>
<a>
<xsl:attribute name="href">contact://<xsl:value-of select="target/contact_id" /></xsl:attribute>
<xsl:value-of select="target/contact_name" />
</a>
</p>

<p>
<xsl:value-of select="messagetext" />
</p>

</xsl:for-each>

</xsl:template>
</xsl:stylesheet>