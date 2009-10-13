/*
 * Copyright (c) [2004-2009] Novell, Inc.
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


#ifndef DM_PART_H
#define DM_PART_H

#include "y2storage/Dm.h"
#include "y2storage/Partition.h"

namespace storage
{

class DmPartCo;
class ProcPart;

class DmPart : public Dm
    {
    public:
	DmPart( const DmPartCo& d, unsigned nr, Partition* p=NULL );
	DmPart( const DmPartCo& d, const DmPart& rd );
	DmPart& operator=( const DmPart& );

	virtual ~DmPart();
	friend std::ostream& operator<< (std::ostream& s, const DmPart &p );
	virtual void print( std::ostream& s ) const { s << *this; }
	void getInfo( storage::DmPartInfo& info ) const;
	bool equalContent( const DmPart& rhs ) const;
	void logDifference( const DmPart& d ) const;
	void setPtr( Partition* pa ) { p=pa; };
	Partition* getPtr() const { return p; };
	unsigned id() const { return p?p->id():0; }
	void updateName();
	void updateMinor();
	void updateSize( ProcPart& pp );
	void updateSize();
	void getCommitActions( std::list<storage::commitAction*>& l ) const;
	void addUdevData();
	virtual const std::list<string> udevId() const;
	virtual string setTypeText( bool doing=true ) const;
	static bool notDeleted( const DmPart& l ) { return( !l.deleted() ); }

    protected:
	void init( const string& name );
	void dataFromPart( const Partition* p );
	virtual const string shortPrintedName() const { return( "DmPart" ); }
	const DmPartCo* co() const; 
	void addAltUdevId( unsigned num );
	Partition* p;

	mutable storage::DmPartInfo info;
    };

}

#endif