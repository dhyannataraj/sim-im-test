/*
 * XML Parser/Generator
 * Simple XML parser/generator sufficient to
 * send+receive the XML in ICQ SMS messages
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef XML_H
#define XML_H

#include <string>
#include <list>
#include <memory>
#include <ctype.h>

using namespace std;

class XmlNode {
private:
    static string parseTag(string::iterator& curr, string::iterator end);
    static void skipWS(string::iterator& curr, string::iterator end);

protected:
    string tag;

    XmlNode(const string& t);

public:
    virtual ~XmlNode();

    virtual bool isBranch() = 0;
    bool isLeaf();

    string getTag();

    static XmlNode *parse(string::iterator& start, string::iterator end);

    static string quote(const string& s);
    static string unquote(const string& s);
    static string replace_all(const string& s, const string& r1, const string& r2);

    virtual string toString(int n) = 0;
};

class XmlLeaf;

class XmlBranch : public XmlNode {
private:
    list<XmlNode*> children;

public:
    XmlBranch(const string& t);
    ~XmlBranch();

    bool isBranch();
    bool exists(const string& tag);
    XmlNode *getNode(const string& tag);
    XmlBranch *getBranch(const string& tag);
    XmlLeaf *getLeaf(const string& tag);

    void pushnode(XmlNode *c);

    string toString(int n);

};

class XmlLeaf : public XmlNode {
private:
    string value;
public:
    XmlLeaf(const string& t, const string& v);
    ~XmlLeaf();

    bool isBranch();
    string getValue();

    string toString(int n);

};

#endif

