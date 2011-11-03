/*
 * Copyright (c) 2010 Novell, Inc.
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact information at www.novell.com.
 */


#ifndef LSSCSI_H
#define LSSCSI_H

#include <string>
#include <map>
#include <vector>

#include "storage/StorageInterface.h"


namespace storage
{
    using std::map;
    using std::vector;


    class Lsscsi
    {
    public:

	Lsscsi();

	struct Entry
	{
	    Entry() : transport(TUNKNOWN) {}

	    Transport transport;
	};

	friend std::ostream& operator<<(std::ostream& s, const Entry& entry);

	bool getEntry(const string& device, Entry& entry) const;

    protected:

	void parse(const vector<string>& lines);

	typedef map<string, Entry>::const_iterator const_iterator;

	map<string, Entry> data;

    };

}


#endif