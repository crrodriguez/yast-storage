#ifndef DISK_H
#define DISK_H

#include <list>

#include "y2storage/Container.h"
#include "y2storage/Partition.h"

namespace storage
{

class Storage;
class SystemCmd;
class ProcPart;
class Region;

class Disk : public Container
    {
    friend class Storage;
    friend class Dasd;

    struct label_info
	{
	string name;
	bool extended;
	unsigned primary;
	unsigned logical;
	unsigned long long max_size_k;
	};

    public:
	Disk( Storage * const s, const string& Name, unsigned long long Size );
	Disk( Storage * const s, const string& Name, unsigned num, 
	      unsigned long long Size, ProcPart& ppart );
	Disk( const Disk& rhs );
	virtual ~Disk();

	unsigned long cylinders() const { return cyl; }
	unsigned heads() const { return head; }
	unsigned sectors() const { return sector; }
	unsigned long numMinor() const { return range; }
	unsigned long cylSizeB() const { return byte_cyl; }
	unsigned maxPrimary() const { return max_primary; }
	unsigned maxLogical() const { return max_logical; }
	const string& labelName() const { return label; }
	const string& udevPath() const { return udev_path; }
	const std::list<string>& udevId() const { return udev_id; }
	void addMpAlias( const string& dev ); 
	void clearMpAlias() { mp_alias.clear(); }
	const std::list<string>& mpAlias() const { return mp_alias; }
	const string& sysfsDir() const { return sysfs_dir; }
	unsigned numPartitions() const;
	bool isDasd() const { return( nm.find("dasd")==0 ); }
	bool isLogical( unsigned nr ) const;
	static storage::CType staticType() { return storage::DISK; }
	friend std::ostream& operator<< (std::ostream&, const Disk& );
	void triggerUdevUpdate();

	static bool needP( const string& dev );
	void setUdevData( const string& path, const string& id );
	virtual int createPartition( storage::PartitionType type, long unsigned start,
				     long unsigned len, string& device,
				     bool checkRelaxed=false );
	int createPartition( long unsigned len, string& device,
			     bool checkRelaxed=false );
	int createPartition( storage::PartitionType type, string& device );
	virtual int removePartition( unsigned nr );
	virtual int changePartitionId( unsigned nr, unsigned id );
	virtual int initializeDisk( bool ) { return storage::DISK_INIT_NOT_POSSIBLE;}
	bool initializeDisk() const { return init_disk; }
	void resetInitDisk() { init_disk=false; }
	int forgetChangePartitionId( unsigned nr );
	int changePartitionArea( unsigned nr, unsigned long start, 
	                         unsigned long size, bool checkRelaxed=false );
	int nextFreePartition( storage::PartitionType type, unsigned& nr,
	                       string& device );
	int destroyPartitionTable( const string& new_label );
	unsigned availablePartNumber( storage::PartitionType type=storage::PRIMARY );
	virtual void getCommitActions( std::list<storage::commitAction*>& l ) const;
	virtual int getToCommit( storage::CommitStage stage, 
	                         std::list<Container*>& col,
				 std::list<Volume*>& vol );
	virtual int commitChanges( storage::CommitStage stage );
	int commitChanges( storage::CommitStage stage, Volume* vol );
	virtual int resizePartition( Partition* p, unsigned long newCyl );
	int resizeVolume( Volume* v, unsigned long long newSize );
	int removeVolume( Volume* v );
	void getUnusedSpace( std::list<Region>& free, bool all=true, 
	                     bool logical=false );
	bool hasExtended() const;
	string setDiskLabelText( bool doing=true ) const;
	unsigned long long cylinderToKb( unsigned long ) const;
	unsigned long kbToCylinder( unsigned long long ) const;
	string getPartName( unsigned nr ) const;
	void getInfo( storage::DiskInfo& info ) const;
	bool equalContent( const Container& rhs ) const;
	void logDifference( const Container& d ) const;
	Disk& operator= ( const Disk& rhs );
	bool FakeDisk() const { return(range==1); }

	static string getPartName( const string& disk, unsigned nr );
	static string getPartName( const string& disk, const string& nr );
	static std::pair<string,long> getDiskPartition( const string& dev );
	static unsigned long long maxSizeLabelK( const string& label );

    protected:

	// iterators over partitions
        // protected typedefs for iterators over partitions
        typedef CastIterator<VIter, Partition *> PartInter;
        typedef CastIterator<CVIter, const Partition *> PartCInter;
        template< class Pred >
            struct PartitionPI { typedef ContainerIter<Pred, PartInter> type; };
        template< class Pred >
            struct PartitionCPI { typedef ContainerIter<Pred, PartCInter> type; };
        typedef CheckFnc<const Partition> CheckFncPartition;
        typedef CheckerIterator< CheckFncPartition, PartitionPI<CheckFncPartition>::type,
                                 PartInter, Partition > PartPIterator;
        typedef CheckerIterator< CheckFncPartition, PartitionCPI<CheckFncPartition>::type,
                                 PartCInter, const Partition > PartCPIterator;
	typedef DerefIterator<PartPIterator,Partition> PartIter;
	typedef DerefIterator<PartCPIterator,const Partition> ConstPartIter;
        typedef IterPair<PartIter> PartPair;
        typedef IterPair<ConstPartIter> ConstPartPair;

        PartPair partPair( bool (* CheckPart)( const Partition& )=NULL)
            {
            return( PartPair( partBegin( CheckPart ), partEnd( CheckPart ) ));
            }
        PartIter partBegin( bool (* CheckPart)( const Partition& )=NULL)
            {
	    IterPair<PartInter> p( (PartInter(begin())), (PartInter(end())) );
            return( PartIter( PartPIterator( p, CheckPart )) );
	    }
        PartIter partEnd( bool (* CheckPart)( const Partition& )=NULL)
            {
	    IterPair<PartInter> p( (PartInter(begin())), (PartInter(end())) );
            return( PartIter( PartPIterator( p, CheckPart, true )) );
	    }

        ConstPartPair partPair( bool (* CheckPart)( const Partition& )=NULL) const
            {
            return( ConstPartPair( partBegin( CheckPart ), partEnd( CheckPart ) ));
            }
        ConstPartIter partBegin( bool (* CheckPart)( const Partition& )=NULL) const
            {
	    IterPair<PartCInter> p( (PartCInter(begin())), (PartCInter(end())) );
            return( ConstPartIter( PartCPIterator( p, CheckPart )) );
	    }
        ConstPartIter partEnd( bool (* CheckPart)( const Partition& )=NULL) const
            {
	    IterPair<PartCInter> p( (PartCInter(begin())), (PartCInter(end())) );
            return( ConstPartIter( PartCPIterator( p, CheckPart, true )) );
	    }

	Disk( Storage * const s, const string& File );
	unsigned long long capacityInKb() const { return size_k; }
	virtual bool detectGeometry();
	virtual bool detectPartitions( ProcPart& ppart );
	bool getSysfsInfo( const string& SysFsDir );
	int checkSystemError( const string& cmd_line, const SystemCmd& cmd );
	int execCheckFailed( const string& cmd_line );
	int execCheckFailed( SystemCmd& cmd, const string& cmd_line );
	bool checkPartedOutput( const SystemCmd& cmd, ProcPart& ppart );
	bool scanPartedLine( const string& Line, unsigned& nr,
	                     unsigned long& start, unsigned long& csize,
			     storage::PartitionType& type, 
			     unsigned& id, bool& boot );
	bool checkPartedValid( const ProcPart& pp, const string& diskname,
	                       std::list<Partition*>& pl, unsigned long& rng );
	bool getPartedValues( Partition *p );
	bool getPartedSectors( const Partition *p, unsigned long long& start,
	                       unsigned long long& end );
	const Partition * getPartitionAfter( const Partition * p );
	virtual void print( std::ostream& s ) const { s << *this; }
	virtual Container* getCopy() const { return( new Disk( *this ) ); }
	void getGeometry( const string& line, unsigned long& c, 
			  unsigned& h, unsigned& s );
	virtual void redetectGeometry();
	void changeNumbers( const PartIter& b, const PartIter& e, 
	                    unsigned start, int incr );
	int createChecks( storage::PartitionType& type, unsigned long start,
	                  unsigned long len, bool checkRelaxed );
	void removePresentPartitions();
	void removeFromMemory();

	static bool notDeleted( const Partition&d ) { return( !d.deleted() ); }
	static bool bootSpecial( const Partition&d ) { return( !d.deleted()&&(d.id()==0x41||d.id()==0x06) ); }

	virtual int doCreate( Volume* v );
	virtual int doRemove( Volume* v );
	virtual int doResize( Volume* v );
	virtual int doSetType( Volume* v );
	virtual int doCreateLabel();

	//std::list<Region> getUnusedRegions();
	void logData( const string& Dir );
	bool haveBsdPart( const std::list<Partition*>& pl) const;
	void setLabelData( const string& );

	static string defaultLabel( unsigned long long size_k );
	static label_info labels[];
	static string p_disks[];

	unsigned long cyl;
	unsigned head;
	unsigned sector;
	unsigned long new_cyl;
	unsigned new_head;
	unsigned new_sector;
	string label;
	string udev_path;
	std::list<string> udev_id;
	std::list<string> mp_alias;
	string detected_label;
	string system_stderr;
	string logfile_name;
	string sysfs_dir;
	unsigned max_primary;
	bool ext_possible;
	bool init_disk;
	bool iscsi;
	unsigned max_logical;
	unsigned long byte_cyl;
	unsigned long range;
	mutable storage::DiskInfo info;
    };

}

#endif
