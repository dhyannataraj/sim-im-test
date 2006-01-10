<?xml version="1.0"?> 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>
<xsl:template match="/message">

<!--linka mezi zpravami -->
<hr/>

<!-- hlavicka :: BEGIN -->
<table cellspacing="0" cellpadding="3" border="0" width="100%" align="top">

<!-- pozadi hlavicky prichozi/odchozi zpravy-->
<xsl:if test="@direction='0'">
<xsl:attribute name="bgcolor">#e5e5e5</xsl:attribute>
</xsl:if>
<xsl:if test="@direction='1'">
<xsl:attribute name="bgcolor">#b0b0b0</xsl:attribute>
</xsl:if>

<tr>
<xsl:choose>

<!-- hlavicka se zmenou stavu :: BEGIN -->
<xsl:when test="@direction='2'">
<!--dummy :) -->
</xsl:when>
<!-- hlavicka se zmenou stavu :: END -->

<xsl:otherwise>

<!-- Ikonky :: BEGIN -->
<td nowrap="yes" width="60">

<!-- odkaz&ikonka zpravy :: BEGIN -->
<a>
<xsl:attribute name="href">msg://<xsl:value-of select="id"/></xsl:attribute>
<img><xsl:attribute name="src">icon:<xsl:value-of select="icon"/></xsl:attribute></img>
</a>
<xsl:text>&#xA0;</xsl:text>
<!-- odkaz&ikonka zpravy :: END 
_____________________________________________________________________________-->
<!-- ikonka sifrovani :: BEGIN -->
<xsl:if test="@encrypted='1'">
<img src="icon:encrypted"/>
<xsl:text>&#xA0;</xsl:text>
</xsl:if>
<!-- ikonka sifrovani :: END
_____________________________________________________________________________-->
<!--ikona urgent message (DND etc) :: BEGIN -->
<xsl:if test="@urgent='1'">
<img src="icon:urgentmsg"/>
<xsl:text>&#xA0;</xsl:text>
</xsl:if>
<!--ikona urgent message (DND etc) :: END 
_____________________________________________________________________________-->
<!--ikona listu (???) :: BEGIN -->
<xsl:if test="@list='1'">
<img src="icon:listmsg"/>
<xsl:text>&#xA0;</xsl:text>
</xsl:if>
<!--ikona listu (???) :: END -->
</td>
<!-- Ikonky :: END 
_____________________________________________________________________________-->

<!-- Sender name :: BEGIN -->
<td nowrap="yes" align="center">
<span>

<!-- BOLD header if unread message -->
<xsl:if test="@unread='1'">
<xsl:attribute name="style">font-weight:600</xsl:attribute>
</xsl:if>

<xsl:text> </xsl:text>
<font>
<xsl:choose>
<xsl:when test="@direction='1'">
<xsl:attribute name="color">#800000</xsl:attribute>
</xsl:when>
<xsl:when test="@direction='0'">
<xsl:attribute name="color">#000080</xsl:attribute>
</xsl:when>
</xsl:choose>
<xsl:value-of disable-output-escaping="no" select="from"/>
</font>
<xsl:text> </xsl:text>
</span>
</td>
<!-- Sender name :: END 
_____________________________________________________________________________-->


<!-- Message time :: BEGIN -->
<td align="right" nowrap="yes">
<font>
<xsl:if test="@direction='2'">
<xsl:attribute name="color">#ffffff</xsl:attribute>
</xsl:if>
<xsl:text> </xsl:text>
<xsl:value-of select="time/date"/>
<xsl:text> </xsl:text>
<xsl:value-of select="time/hour"/>:<xsl:value-of select="time/minute"/>:<xsl:value-of select="time/second"/>
</font>
</td>
<!-- Message time :: END
_____________________________________________________________________________-->


</xsl:otherwise>
</xsl:choose>
</tr>
</table>
<!-- hlavicka :: END
_____________________________________________________________________________-->



<xsl:choose>
<!-- hlavicka se zmenou stavu :: BEGIN -->

<xsl:when test="@direction='2'">
<p>
<!-- odkaz&ikonka zpravy :: BEGIN -->
<img><xsl:attribute name="src">icon:<xsl:value-of select="icon"/></xsl:attribute></img>
<xsl:text>&#xA0;</xsl:text>
<!-- odkaz&ikonka zpravy :: END
_____________________________________________________________________________-->
<font>
<!--xsl:attribute name="color">#ffffff</xsl:attribute-->
<xsl:text> </xsl:text>
<xsl:value-of select="time/date"/>
<xsl:text> </xsl:text>
<xsl:value-of select="time/hour"/>:<xsl:value-of select="time/minute"/>:<xsl:value-of select="time/second"/>
<xsl:text> </xsl:text>
<xsl:value-of disable-output-escaping="yes" select="from"/>
<xsl:text> --> </xsl:text><xsl:value-of disable-output-escaping="yes" select="body"/>
</font>
</p>
</xsl:when>
<!-- hlavicka se zmenou stavu :: END
_____________________________________________________________________________-->

<!-- tradicni zprava :: BEGIN -->
<xsl:otherwise>
<p>
<xsl:value-of disable-output-escaping="yes" select="body"/>
</p>
</xsl:otherwise>
</xsl:choose>
<!-- tradicni zprava :: END
_____________________________________________________________________________-->


<!-- FORMATOVACI ENTER -->
<p>
<xsl:text>&#xA0;</xsl:text>
</p>


</xsl:template>
</xsl:stylesheet>