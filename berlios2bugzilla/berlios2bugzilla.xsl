<?xml version="1.0" encoding="utf-8"?>
<!--
	transformation: bugs from berlios exported list to bugzilla
	processor: msxsl
-->
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output encoding="utf-8" version="1.0" indent="yes" />

<xsl:preserve-space elements="//thetext"/>

<xsl:variable name="site_bugzilla_version">2.22</xsl:variable>
<xsl:variable name="site_urlbase">http://bugs.sim-im.org/</xsl:variable>
<xsl:variable name="site_maintainer">wrar@altlinux.ru</xsl:variable>

<xsl:variable name="importer_bug_summary_header">
  This bug is imported from old bug tracking system at https://developer.berlios.de/bugs/?group_id=4482.
</xsl:variable>
<xsl:variable name="importer_bug_original_link_prefix"><![CDATA[https://developer.berlios.de/bugs/?func=detailbug&group_id=4482&bug_id=]]></xsl:variable>
<xsl:variable name="importer_bug_original_text_prefix">Original bug report: </xsl:variable>
<xsl:variable name="importer_bug_original_link_suffix">.
</xsl:variable>

<xsl:variable name="importer_creation_timestamp">2006-08-01 08:01</xsl:variable>
<xsl:variable name="importer_classification_id">1</xsl:variable>
<xsl:variable name="importer_classification">Unclassified</xsl:variable>
<xsl:variable name="importer_product">SIM Instant Messenger</xsl:variable>
<xsl:variable name="importer_component">sim</xsl:variable>
<xsl:variable name="importer_version">0.9.5 (current)</xsl:variable>
<xsl:variable name="importer_reporter_platform">PC</xsl:variable>
<xsl:variable name="importer_operating_system">All</xsl:variable>
<xsl:variable name="importer_bug_status">NEW</xsl:variable>
<xsl:variable name="importer_bug_file_loc">https://developer.berlios.de/bugs/?group_id=4482</xsl:variable>
<xsl:variable name="importer_priority">P1</xsl:variable>
<xsl:variable name="importer_bug_severity">normal</xsl:variable>
<xsl:variable name="importer_target_milestone">0.9.5</xsl:variable>
<xsl:variable name="importer_reporter">zowers</xsl:variable>
<xsl:variable name="importer_reporter_base">@berlios.de</xsl:variable>
<xsl:variable name="importer_assigned_to"></xsl:variable>

<xsl:template match="/">

  <bugzilla>
    <xsl:attribute name="version"><xsl:value-of select="$site_bugzilla_version"/></xsl:attribute>
    <xsl:attribute name="urlbase"><xsl:value-of select="$site_urlbase"/></xsl:attribute>
    <xsl:attribute name="maintainer"><xsl:value-of select="$site_maintainer"/></xsl:attribute>

    <xsl:apply-templates select="/bugs/bug" />

  </bugzilla>

</xsl:template>

<xsl:template match="/bugs/bug">

  <bug>
    <xsl:variable name="original_bug_id"><xsl:value-of select="@id" /></xsl:variable>
    <bug_id><xsl:value-of select="$original_bug_id" /></bug_id>

    <creation_ts><xsl:value-of select="$importer_creation_timestamp" /></creation_ts>
    <short_desc><xsl:value-of select="summary" /></short_desc>
    <delta_ts><xsl:value-of select="$importer_creation_timestamp" /></delta_ts>
    <reporter_accessible>1</reporter_accessible>
    <cclist_accessible>1</cclist_accessible>
    <classification_id><xsl:value-of select="$importer_classification_id" /></classification_id>
    <classification><xsl:value-of select="$importer_classification" /></classification>
    <product><xsl:value-of select="$importer_product" /></product>
    <component><xsl:value-of select="$importer_component" /></component>
    <version><xsl:value-of select="$importer_version" /></version>
    <rep_platform><xsl:value-of select="$importer_reporter_platform" /></rep_platform>
    <op_sys><xsl:value-of select="$importer_operating_system" /></op_sys>
    <bug_status><xsl:value-of select="$importer_bug_status" /></bug_status>

    <xsl:variable name="bug_original_link">
      <xsl:value-of select="$importer_bug_original_link_prefix" /><xsl:value-of select="$original_bug_id" />
    </xsl:variable>
    
    <bug_file_loc><xsl:value-of select="$bug_original_link" /></bug_file_loc>
    
    <priority><xsl:value-of select="$importer_priority" /></priority>
    <bug_severity><xsl:value-of select="$importer_bug_severity" /></bug_severity>
    <target_milestone><xsl:value-of select="$importer_target_milestone" /></target_milestone>
    
    <everconfirmed>1</everconfirmed>
    <xsl:variable name="reporter"><xsl:value-of select="submitted_by/@name" /><xsl:value-of select="$importer_reporter_base" /></xsl:variable>
    <reporter><xsl:value-of select="$reporter" /></reporter>
    <assigned_to><xsl:value-of select="$importer_assigned_to" /></assigned_to>

    <long_desc isprivate="0">
      <who><xsl:value-of select="$reporter" /></who>
      <bug_when><xsl:value-of select="$importer_creation_timestamp" /></bug_when>
      <thetext>
        <xsl:value-of select="$importer_bug_summary_header" />
...
        <xsl:value-of select="$importer_bug_original_text_prefix" /><xsl:value-of select="$bug_original_link" /><xsl:value-of select="$importer_bug_original_link_suffix" />
...
        <xsl:value-of select="details" />
      </thetext>
    </long_desc>

    <xsl:apply-templates select="history" />

  </bug>

</xsl:template>

<xsl:template match="/bugs/bug/history">

  <long_desc isprivate="0">
    <who><xsl:value-of select="mod_by" /></who>
    <bug_when><xsl:value-of select="date" /></bug_when>
    <xsl:choose>
      <xsl:when test="field_name = 'details'"> 
        <thetext><xsl:value-of select="old_value" /></thetext>
      </xsl:when>
      <xsl:otherwise>
        <thetext>
          Field name: <xsl:value-of select="field_name" />
          Value: <xsl:value-of select="old_value" />
        </thetext>
      </xsl:otherwise>
    </xsl:choose>
  </long_desc>

</xsl:template>

</xsl:stylesheet>
