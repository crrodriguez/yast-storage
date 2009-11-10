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

/*
  Textdomain    "storage"
*/

#include <iostream>
#include <sstream>

#include "y2storage/DmraidCo.h"
#include "y2storage/Dmraid.h"
#include "y2storage/MdPartCo.h"
#include "y2storage/SystemCmd.h"
#include "y2storage/AppUtil.h"
#include "y2storage/Storage.h"
#include "y2storage/StorageDefines.h"

using namespace std;
using namespace storage;


DmraidCo::DmraidCo(Storage * const s, const string& Name, ProcPart& ppart)
    : DmPartCo(s, "/dev/mapper/"+Name, staticType(), ppart)
{
    DmPartCo::init(ppart);
    getRaidData(Name);
    y2deb("constructing dmraid co " << Name);
}


DmraidCo::~DmraidCo()
{
    y2deb("destructed raid co " << dev);
}


void DmraidCo::getRaidData( const string& name )
    {
    y2milestone( "name:%s", name.c_str() );
    SystemCmd c(DMRAIDBIN " -s -c -c -c " + quote(name));
    list<string>::const_iterator ci;
    list<string> sl;
    if( c.numLines()>0 )
	sl = splitString( *c.getLine(0), ":" );
    Pv *pve = new Pv;
    if( sl.size()>=4 )
	{
	ci = sl.begin();
	++ci; ++ci; ++ci;
	raidtype = *ci;
	}
    unsigned num = 1;
    while( num<c.numLines() )
	{
	sl = splitString( *c.getLine(num), ":" );
	y2mil( "sl:" << sl );
	if( sl.size()>=3 )
	    {
	    ci = sl.begin();
	    ++ci; ++ci;
	    if( *ci == name )
		{
		--ci;
		if( controller.empty() && !ci->empty() )
		    controller = *ci;
		--ci;
		if( ci->find( "/dev/" )==0 )
		    {
		    pve->device = *ci;
		    addPv( pve );
		    }
		}
	    }
	++num;
	}
    delete( pve );
    }


void
DmraidCo::setUdevData( const list<string>& id )
{
    y2mil("disk:" << nm << " id:" << id);
    udev_id = id;
    udev_id.erase(remove_if(udev_id.begin(), udev_id.end(), find_begin("dm-")), udev_id.end());
    udev_id.sort();
    y2mil("id:" << udev_id);

    DmPartCo::setUdevData(udev_id);

    DmraidPair pp = dmraidPair();
    for( DmraidIter p=pp.begin(); p!=pp.end(); ++p )
	{
	p->addUdevData();
	}
}


void
DmraidCo::newP( DmPart*& dm, unsigned num, Partition* p )
    {
    y2mil( "num:" << num );
    dm = new Dmraid( *this, num, p );
    }

void DmraidCo::addPv( Pv*& p )
    {
    PeContainer::addPv( p );
    if( !deleted() )
	getStorage()->setUsedBy( p->device, UB_DMRAID, name() );
    p = new Pv;
    }

void DmraidCo::activate( bool val )
    {
    y2milestone( "old active:%d val:%d", active, val );
    if( active != val )
	{
	SystemCmd c;
	if( val )
	    {
	    Dm::activate(true);
	    c.execute(DMRAIDBIN " -ay -p");
	    }
	else
	    {
	    c.execute(DMRAIDBIN " -an");
	    }
	active = val;
	}
    }


    bool
    DmraidCo::isActivated(const string& name)
    {
	SystemCmd c(DMSETUPBIN " table " + quote(name));
	return c.retcode() == 0 && c.numLines() >= 1 && isdigit(c.stdout()[0]);
    }


    list<string>
    DmraidCo::getRaids()
    {
	list<string> l;

	SystemCmd c(DMRAIDBIN " -s -c -c -c");
	for( unsigned i=0; i<c.numLines(); ++i )
	{
	    list<string> sl = splitString( *c.getLine(i), ":" );
	    if( sl.size()>=3 )
	    {
		list<string>::const_iterator ci = sl.begin();
		if( !ci->empty()
		    && ci->find( "/dev/" )==string::npos
		    && find( l.begin(), l.end(), *ci )==l.end())
		{
		    if (isActivated(*ci))
		    {
			l.push_back( *ci );
		    }
		    else
		    {
			y2mil("ignoring inactive dmraid " << *ci);
		    }
		}
	    }
	}

	y2mil("detected dmraids " << l);
	return l;
    }


string DmraidCo::removeText( bool doing ) const
    {
    string txt;
    if( doing )
        {
        // displayed text during action, %1$s is replaced by a name (e.g. pdc_igeeeadj),
        txt = sformat( _("Removing raid %1$s"), name().c_str() );
        }
    else
        {
        // displayed text before action, %1$s is replaced by a name (e.g. pdc_igeeeadj),
        txt = sformat( _("Remove raid %1$s"), name().c_str() );
        }
    return( txt );
    }


string DmraidCo::setDiskLabelText( bool doing ) const
    {
    string txt;
    string d = nm;
    if( doing )
        {
        // displayed text during action, %1$s is replaced by raid name (e.g. pdc_igeeeadj),
	// %2$s is replaced by label name (e.g. msdos)
        txt = sformat( _("Setting disk label of raid %1$s to %2$s"),
		       d.c_str(), labelName().c_str() );
        }
    else
        {
        // displayed text before action, %1$s is replaced by raid name (e.g. pdc_igeeeadj),
	// %2$s is replaced by label name (e.g. msdos)
        txt = sformat( _("Set disk label of raid %1$s to %2$s"),
		      d.c_str(), labelName().c_str() );
        }
    return( txt );
    }

int
DmraidCo::doRemove()
    {
    y2milestone( "Raid:%s", name().c_str() );
    int ret = 0;
    if( deleted() )
	{
	if( active )
	    {
	    activate_part(false);
	    activate(false);
	    }
	if( !silent )
	    {
	    getStorage()->showInfoCb( removeText(true) );
	    }
	string cmd = "cd /var/log/YaST2 && echo y | " DMRAIDBIN " -E -r";
	SystemCmd c;
	for( list<Pv>::const_iterator i=pv.begin(); i!=pv.end(); ++i )
	    {
	    c.execute(cmd + " " + quote(i->device));
	    }
	if( c.retcode()!=0 )
	    {
	    ret = DMRAID_REMOVE_FAILED;
	    setExtError( c );
	    }
	if( ret==0 )
	    {
	    setDeleted( false );
	    }
	}
    y2mil("ret:" << ret);
    return( ret );
    }

void DmraidCo::getInfo( DmraidCoInfo& tinfo ) const
    {
    DmPartCo::getInfo( info );
    tinfo.p = info;
    }

namespace storage
{

std::ostream& operator<< (std::ostream& s, const DmraidCo& d )
    {
    s << *((DmPartCo*)&d);
    s << " Cont:" << d.controller
      << " RType:" << d.raidtype;
    return( s );
    }

}

string DmraidCo::getDiffString( const Container& d ) const
    {
    string log = DmPartCo::getDiffString( d );
    const DmraidCo * p = dynamic_cast<const DmraidCo*>(&d);
    if( p )
	{
	if( controller!=p->controller )
	    log += " controller:" + controller + "-->" + p->controller;
	if( raidtype!=p->raidtype )
	    log += " raidtype:" + raidtype + "-->" + p->raidtype;
	}
    return( log );
    }

bool DmraidCo::equalContent( const Container& rhs ) const
    {
    bool ret = Container::equalContent(rhs);
    if( ret )
	{
	const DmraidCo *p = dynamic_cast<const DmraidCo*>(&rhs);
	ret = p && DmPartCo::equalContent( *p ) &&
              controller==p->controller && raidtype==p->raidtype;
	}
    return( ret );
    }

DmraidCo::DmraidCo( const DmraidCo& rhs ) : DmPartCo(rhs)
    {
    raidtype = rhs.raidtype;
    controller = rhs.controller;
    }

void DmraidCo::logData( const string& Dir ) {;}

bool DmraidCo::active = false;
