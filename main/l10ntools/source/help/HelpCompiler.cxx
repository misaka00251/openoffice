/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "HelpCompiler.hxx"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#ifdef __MINGW32__
#include <tools/prewin.h>
#include <tools/postwin.h>
#endif
#include <osl/thread.hxx>

static void impl_sleep( sal_uInt32 nSec )
{
	TimeValue aTime;
	aTime.Seconds = nSec;
	aTime.Nanosec = 0;

	osl::Thread::wait( aTime );
}

HelpCompiler::HelpCompiler(StreamTable &in_streamTable, const fs::path &in_inputFile,
	const fs::path &in_src, const fs::path &in_resEmbStylesheet,
	const std::string &in_module, const std::string &in_lang, bool in_bExtensionMode)
	: streamTable(in_streamTable), inputFile(in_inputFile),
	src(in_src), module(in_module), lang(in_lang), resEmbStylesheet(in_resEmbStylesheet),
	bExtensionMode( in_bExtensionMode )
{
	xmlKeepBlanksDefaultValue = 0;
}

xmlDocPtr HelpCompiler::getSourceDocument(const fs::path &filePath)
{
	static const char *params[4 + 1];
	static xsltStylesheetPtr cur = NULL;

	xmlDocPtr res;
	if( bExtensionMode )
	{
		res = xmlParseFile(filePath.native_file_string().c_str());
		if( !res ){
			impl_sleep( 3 );
			res = xmlParseFile(filePath.native_file_string().c_str());
		}
	}
	else
	{
		if (!cur)
		{
			static std::string fsroot('\'' + src.toUTF8() + '\'');
			static std::string esclang('\'' + lang + '\'');

			xmlSubstituteEntitiesDefault(1);
			xmlLoadExtDtdDefaultValue = 1;
			cur = xsltParseStylesheetFile((const xmlChar *)resEmbStylesheet.native_file_string().c_str());

			int nbparams = 0;
			params[nbparams++] = "Language";
			params[nbparams++] = esclang.c_str();
			params[nbparams++] = "fsroot";
			params[nbparams++] = fsroot.c_str();
			params[nbparams] = NULL;
		}
		xmlDocPtr doc = xmlParseFile(filePath.native_file_string().c_str());
		if( !doc )
		{
			impl_sleep( 3 );
			doc = xmlParseFile(filePath.native_file_string().c_str());
		}

		//???res = xmlParseFile(filePath.native_file_string().c_str());

		res = xsltApplyStylesheet(cur, doc, params);
		xmlFreeDoc(doc);
	}
	return res;
}

HashSet HelpCompiler::switchFind(xmlDocPtr doc)
{
	HashSet hs;
	xmlChar *xpath = (xmlChar*)"//switchinline";

	xmlXPathContextPtr context = xmlXPathNewContext(doc);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (result)
	{
		xmlNodeSetPtr nodeset = result->nodesetval;
		for (int i = 0; i < nodeset->nodeNr; i++)
		{
			xmlNodePtr el = nodeset->nodeTab[i];
			xmlChar *select = xmlGetProp(el, (xmlChar*)"select");
			if (select)
			{
				if (!strcmp((const char*)select, "appl"))
				{
					xmlNodePtr n1 = el->xmlChildrenNode;
					while (n1)
					{
						if ((!xmlStrcmp(n1->name, (const xmlChar*)"caseinline")))
						{
							xmlChar *appl = xmlGetProp(n1, (xmlChar*)"select");
							hs.push_back(std::string((const char*)appl));
							xmlFree(appl);
						}
						else if ((!xmlStrcmp(n1->name, (const xmlChar*)"defaultinline")))
							hs.push_back(std::string("DEFAULT"));
						n1 = n1->next;
					}
				}
				xmlFree(select);
			}
		}
		xmlXPathFreeObject(result);
	}
	hs.push_back(std::string("DEFAULT"));
	return hs;
}

// returns a node representing the whole stuff compiled for the current
// application.
xmlNodePtr HelpCompiler::clone(xmlNodePtr node, const std::string& appl)
{
	xmlNodePtr parent = xmlCopyNode(node, 2);
	xmlNodePtr n = node->xmlChildrenNode;
	while (n != NULL)
	{
		bool isappl = false;
		if ( (!strcmp((const char*)n->name, "switchinline")) ||
			 (!strcmp((const char*)n->name, "switch")) )
		{
			xmlChar *select = xmlGetProp(n, (xmlChar*)"select");
			if (select)
			{
				if (!strcmp((const char*)select, "appl"))
					isappl = true;
				xmlFree(select);
			}
		}
		if (isappl)
		{
			xmlNodePtr caseNode = n->xmlChildrenNode;
			if (appl == "DEFAULT")
			{
				while (caseNode)
				{
					if (!strcmp((const char*)caseNode->name, "defaultinline"))
					{
						xmlNodePtr cnl = caseNode->xmlChildrenNode;
						while (cnl)
						{
							xmlAddChild(parent, clone(cnl, appl));
							cnl = cnl->next;
						}
						break;
					}
					caseNode = caseNode->next;
				}
			}
			else
			{
				while (caseNode)
				{
					isappl=false;
					if (!strcmp((const char*)caseNode->name, "caseinline"))
					{
						xmlChar *select = xmlGetProp(n, (xmlChar*)"select");
						if (select)
						{
							if (!strcmp((const char*)select, appl.c_str()))
								isappl = true;
							xmlFree(select);
						}
						if (isappl)
						{
							xmlNodePtr cnl = caseNode->xmlChildrenNode;
							while (cnl)
							{
								xmlAddChild(parent, clone(cnl, appl));
								cnl = cnl->next;
							}
							break;
						}

					}
					caseNode = caseNode->next;
				}
			}

		}
		else
			xmlAddChild(parent, clone(n, appl));

		n = n->next;
	}
	return parent;
}

class myparser
{
public:
	std::string documentId;
	std::string fileName;
	std::string title;
	HashSet *hidlist;
	Hashtable *keywords;
	Stringtable *helptexts;
private:
	HashSet extendedHelpText;
public:
	myparser(const std::string &indocumentId, const std::string &infileName,
		const std::string &intitle) : documentId(indocumentId), fileName(infileName),
		title(intitle)
	{
		hidlist = new HashSet;
		keywords = new Hashtable;
		helptexts = new Stringtable;
	}
	void traverse( xmlNodePtr parentNode );
private:
	std::string dump(xmlNodePtr node);
};

std::string myparser::dump(xmlNodePtr node)
{
	std::string app;
	if (node->xmlChildrenNode)
	{
		xmlNodePtr list = node->xmlChildrenNode;
		while (list)
		{
			app += dump(list);
			list = list->next;
		}
	}
	if (xmlNodeIsText(node))
	{
		xmlChar *pContent = xmlNodeGetContent(node);
		app += std::string((const char*)pContent);
		xmlFree(pContent);
		// std::cout << app << std::endl;
	}
	return app;
}

void trim(std::string& str)
{
	std::string::size_type pos = str.find_last_not_of(' ');
	if(pos != std::string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != std::string::npos)
			str.erase(0, pos);
	}
	else
		str.erase(str.begin(), str.end());
}

void myparser::traverse( xmlNodePtr parentNode )
{
	// traverse all nodes that belong to the parent
	xmlNodePtr test ;
	for (test = parentNode->xmlChildrenNode; test; test = test->next)
	{
		if (fileName.empty() && !strcmp((const char*)test->name, "filename"))
		{
			xmlNodePtr node = test->xmlChildrenNode;
			if (xmlNodeIsText(node))
			{
				xmlChar *pContent = xmlNodeGetContent(node);
				fileName = std::string((const char*)pContent);
				xmlFree(pContent);
			}
		}
		else if (title.empty() && !strcmp((const char*)test->name, "title"))
		{
			title = dump(test);
			if (title.empty())
				title = "<notitle>";
		}
		else if (!strcmp((const char*)test->name, "bookmark"))
		{
			xmlChar *branchxml = xmlGetProp(test, (const xmlChar*)"branch");
			xmlChar *idxml = xmlGetProp(test, (const xmlChar*)"id");
			std::string branch((const char*)branchxml);
			std::string anchor((const char*)idxml);
			xmlFree (branchxml);
			xmlFree (idxml);

			std::string hid;

			if (branch.find("hid") == 0)
			{
				size_t index = branch.find('/');
				if (index != std::string::npos)
				{
					hid = branch.substr(1 + index);
					// one shall serve as a documentId
					if (documentId.empty())
						documentId = hid;
					extendedHelpText.push_back(hid);
					std::string foo = anchor.empty() ? hid : hid + "#" + anchor;
					HCDBG(std::cerr << "hid pushback" << foo << std::endl);
					hidlist->push_back( anchor.empty() ? hid : hid + "#" + anchor);
				}
				else
					continue;
			}
			else if (branch.compare("index") == 0)
			{
				LinkedList ll;

				for (xmlNodePtr nd = test->xmlChildrenNode; nd; nd = nd->next)
				{
					if (strcmp((const char*)nd->name, "bookmark_value"))
						continue;

					std::string embedded;
					xmlChar *embeddedxml = xmlGetProp(nd, (const xmlChar*)"embedded");
					if (embeddedxml)
					{
						embedded = std::string((const char*)embeddedxml);
						xmlFree (embeddedxml);
						std::transform (embedded.begin(), embedded.end(),
							embedded.begin(), tolower);
					}

					bool isEmbedded = !embedded.empty() && embedded.compare("true") == 0;
					if (isEmbedded)
						continue;

					std::string keyword = dump(nd);
					size_t keywordSem = keyword.find(';');
					if (keywordSem != std::string::npos)
					{
						std::string tmppre =
									keyword.substr(0,keywordSem);
						trim(tmppre);
						std::string tmppos =
									keyword.substr(1+keywordSem);
						trim(tmppos);
						keyword = tmppre + ";" + tmppos;
					}
					ll.push_back(keyword);
				}
				if (!ll.empty())
					(*keywords)[anchor] = ll;
			}
			else if (branch.compare("contents") == 0)
			{
				// currently not used
			}
		}
		else if (!strcmp((const char*)test->name, "ahelp"))
		{
			std::string text = dump(test);
			trim(text);
			std::string name;

			HashSet::const_iterator aEnd = extendedHelpText.end();
			for (HashSet::const_iterator iter = extendedHelpText.begin(); iter != aEnd;
				++iter)
			{
				name = *iter;
				(*helptexts)[name] = text;
			}
			extendedHelpText.clear();
		}

		// traverse children
		traverse(test);
	}
}

bool HelpCompiler::compile( void ) throw( HelpProcessingException )
{
	// we now have the jaroutputstream, which will contain the document.
	// now determine the document as a dom tree in variable docResolved

	xmlDocPtr docResolvedOrg = getSourceDocument(inputFile);

	// now add path to the document
	// resolve the dom
	if (!docResolvedOrg)
	{
		impl_sleep( 3 );
		docResolvedOrg = getSourceDocument(inputFile);
		if( !docResolvedOrg )
		{
			std::stringstream aStrStream;
			aStrStream << "ERROR: file not existing: " << inputFile.native_file_string().c_str() << std::endl;
			throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
		}
	}

	// now find all applications for which one has to compile
	std::string documentId;
	std::string fileName;
	std::string title;
	// returns all applications for which one has to compile
	HashSet applications = switchFind(docResolvedOrg);

	HashSet::const_iterator aEnd = applications.end();
	for (HashSet::const_iterator aI = applications.begin(); aI != aEnd; ++aI)
	{
		std::string appl = *aI;
		std::string modulename = appl;
		if (modulename[0] == 'S')
		{
			modulename = modulename.substr(1);
			std::transform(modulename.begin(), modulename.end(), modulename.begin(), tolower);
		}
		if (modulename != "DEFAULT" && modulename != module)
			continue;

		// returns a clone of the document with switch-cases resolved
		xmlNodePtr docResolved = clone(xmlDocGetRootElement(docResolvedOrg), appl);
		myparser aparser(documentId, fileName, title);
		aparser.traverse(docResolved);

		documentId = aparser.documentId;
		fileName = aparser.fileName;
		title = aparser.title;

		HCDBG(std::cerr << documentId << " : " << fileName << " : " << title << std::endl);

		xmlDocPtr docResolvedDoc = xmlCopyDoc(docResolvedOrg, false);
		xmlDocSetRootElement(docResolvedDoc, docResolved);

		if (modulename == "DEFAULT")
		{
			streamTable.dropdefault();
			streamTable.default_doc = docResolvedDoc;
			streamTable.default_hidlist = aparser.hidlist;
			streamTable.default_helptexts = aparser.helptexts;
			streamTable.default_keywords = aparser.keywords;
		}
		else if (modulename == module)
		{
			streamTable.dropappl();
			streamTable.appl_doc = docResolvedDoc;
			streamTable.appl_hidlist = aparser.hidlist;
			streamTable.appl_helptexts = aparser.helptexts;
			streamTable.appl_keywords = aparser.keywords;
		}
		else
		{
			std::stringstream aStrStream;
			aStrStream << "ERROR: Found unexpected module name \"" << modulename
					   << "\" in file" << src.native_file_string().c_str() << std::endl;
			throw HelpProcessingException( HELPPROCESSING_GENERAL_ERROR, aStrStream.str() );
		}

	} // end iteration over all applications

	streamTable.document_id = documentId;
	streamTable.document_path = fileName;
	streamTable.document_title = title;
	std::string actMod = module;
	if ( !bExtensionMode && !fileName.empty())
	{
		if (fileName.find("/text/") == 0)
		{
			int len = strlen("/text/");
			actMod = fileName.substr(len);
			actMod = actMod.substr(0, actMod.find('/'));
		}
	}
	streamTable.document_module = actMod;

	xmlFreeDoc(docResolvedOrg);
	return true;
}

namespace fs
{
	rtl_TextEncoding getThreadTextEncoding( void )
	{
		static bool bNeedsInit = true;
		static rtl_TextEncoding nThreadTextEncoding;
		if( bNeedsInit )
		{
			bNeedsInit = false;
			nThreadTextEncoding = osl_getThreadTextEncoding();
		}
		return nThreadTextEncoding;
	}

	void create_directory(const fs::path indexDirName)
	{
		HCDBG(
			std::cerr << "creating " <<
			rtl::OUStringToOString(indexDirName.data, RTL_TEXTENCODING_UTF8).getStr()
			<< std::endl
			 );
		osl::Directory::createPath(indexDirName.data);
	}

	void rename(const fs::path &src, const fs::path &dest)
	{
		osl::File::move(src.data, dest.data);
	}

	void copy(const fs::path &src, const fs::path &dest)
	{
		osl::File::copy(src.data, dest.data);
	}

	bool exists(const fs::path &in)
	{
		osl::File tmp(in.data);
		return (tmp.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None);
	}

	void remove(const fs::path &in)
	{
		osl::File::remove(in.data);
	}

	void removeRecursive(rtl::OUString const& _suDirURL)
	{
		{
			osl::Directory aDir(_suDirURL);
			aDir.open();
			if (aDir.isOpen())
			{
				osl::DirectoryItem aItem;
				osl::FileStatus aStatus(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Attributes);
				while (aDir.getNextItem(aItem) == ::osl::FileBase::E_None)
				{
					if (osl::FileBase::E_None == aItem.getFileStatus(aStatus) &&
						aStatus.isValid(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Attributes))
					{
						rtl::OUString suFilename = aStatus.getFileName();
						rtl::OUString suFullFileURL;
						suFullFileURL += _suDirURL;
						suFullFileURL += rtl::OUString::createFromAscii("/");
						suFullFileURL += suFilename;

						if (aStatus.getFileType() == osl::FileStatus::Directory)
							removeRecursive(suFullFileURL);
						else
							osl::File::remove(suFullFileURL);
					}
				}
				aDir.close();
			}
		}
		osl::Directory::remove(_suDirURL);
	}

	void remove_all(const fs::path &in)
	{
		removeRecursive(in.data);
	}
}

/* vim: set noet sw=4 ts=4: */
