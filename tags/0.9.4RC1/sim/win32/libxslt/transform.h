/*
 * transform.h: Interfaces, constants and types related to the XSLT engine
 *            transform part.
 *
 * See Copyright for the status of this software.
 *
 * daniel@veillard.com
 */

#ifndef __XML_XSLT_TRANSFORM_H__
#define __XML_XSLT_TRANSFORM_H__

#include <libxml/parser.h>
#include <libxml/xmlIO.h>
#include "xsltexports.h"
#include <libxslt/xsltInternals.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * XInclude default processing.
 */
XSLTPUBFUN void XSLTCALL		
		xsltSetXIncludeDefault	(int xinclude);
XSLTPUBFUN int XSLTCALL		
		xsltGetXIncludeDefault	(void);

/**
 * Export context to users.
 */
XSLTPUBFUN xsltTransformContextPtr XSLTCALL 
		xsltNewTransformContext	(xsltStylesheetPtr style,
					 xmlDocPtr doc);

XSLTPUBFUN void XSLTCALL			
		xsltFreeTransformContext(xsltTransformContextPtr ctxt);

XSLTPUBFUN xmlDocPtr XSLTCALL		
		xsltApplyStylesheetUser	(xsltStylesheetPtr style,
					 xmlDocPtr doc,
					 const char **params,
					 const char *output,
					 FILE * profile,
					 xsltTransformContextPtr userCtxt);
/**
 * Private Interfaces.
 */
XSLTPUBFUN void XSLTCALL		
		xsltApplyStripSpaces	(xsltTransformContextPtr ctxt,
					 xmlNodePtr node);
XSLTPUBFUN xmlDocPtr XSLTCALL	
		xsltApplyStylesheet	(xsltStylesheetPtr style,
					 xmlDocPtr doc,
					 const char **params);
XSLTPUBFUN xmlDocPtr XSLTCALL	
		xsltProfileStylesheet	(xsltStylesheetPtr style,
					 xmlDocPtr doc,
					 const char **params,
					 FILE * output);
XSLTPUBFUN int XSLTCALL		
		xsltRunStylesheet	(xsltStylesheetPtr style,
					 xmlDocPtr doc,
					 const char **params,
					 const char *output,
					 xmlSAXHandlerPtr SAX,
					 xmlOutputBufferPtr IObuf);
XSLTPUBFUN int XSLTCALL		
		xsltRunStylesheetUser	(xsltStylesheetPtr style,
					 xmlDocPtr doc,
					 const char **params,
					 const char *output,
					 xmlSAXHandlerPtr SAX,
					 xmlOutputBufferPtr IObuf,
					 FILE * profile,
					 xsltTransformContextPtr userCtxt);
XSLTPUBFUN void XSLTCALL		
		xsltApplyOneTemplate	(xsltTransformContextPtr ctxt,
					 xmlNodePtr node,
					 xmlNodePtr list,
					 xsltTemplatePtr templ,
					 xsltStackElemPtr params);
XSLTPUBFUN void XSLTCALL 		
		xsltDocumentElem	(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltSort		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltCopy		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltText		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltElement		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltComment		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltAttribute		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltProcessingInstruction(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltCopyOf		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltValueOf		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltNumber		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltApplyImports	(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltCallTemplate	(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltApplyTemplates	(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltChoose		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltIf			(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL 		
		xsltForEach		(xsltTransformContextPtr ctxt,
	                                 xmlNodePtr node,
					 xmlNodePtr inst,
					 xsltStylePreCompPtr comp);
XSLTPUBFUN void XSLTCALL		
		xsltRegisterAllElement	(xsltTransformContextPtr ctxt);

XSLTPUBFUN xmlNodePtr XSLTCALL	
		xsltCopyTextString	(xsltTransformContextPtr ctxt,
					 xmlNodePtr target,
					 const xmlChar *string,
					 int noescape);
/*
 * Hook for the debugger if activated.
 */
XSLTPUBFUN void XSLTCALL		
		xslHandleDebugger	(xmlNodePtr cur,
					 xmlNodePtr node,
					 xsltTemplatePtr templ,
					 xsltTransformContextPtr ctxt);

#ifdef __cplusplus
}
#endif

#endif /* __XML_XSLT_TRANSFORM_H__ */

