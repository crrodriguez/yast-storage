
#include <iostream>
#include <iterator>

#include <y2storage/StorageInterface.h>

using namespace std;
using namespace storage;


StorageInterface* s = 0;


void
print_partitions (const string& disk)
{
    list<PartitionInfo> partitioninfos;
    s->getPartitions (disk, partitioninfos);
    for (list<PartitionInfo>::iterator i = partitioninfos.begin ();
	 i != partitioninfos.end(); i++)
    {
	cout << i->name << ' ';
	switch (i->partitionType)
	{
	    case PRIMARY: cout << "PRIMARY "; break;
	    case EXTENDED: cout << "EXTENDED "; break;
	    case LOGICAL: cout << "LOGICAL "; break;
	    default: cout << "UNKNOWN ";
	}
	cout << i->cylStart << ' ' << i->cylSize << '\n';
    }

    cout << '\n';
}


void
msdos (const string& disk, int n)
{
    printf ("msdos %s %d\n", disk.c_str(), n);

    s->destroyPartitionTable (disk, "msdos");

    long int S = 100000;
    string name;

    cout << s->createPartitionKb (disk, PRIMARY, 0*S, S, name) << '\n';
    cout << s->createPartitionKb (disk, PRIMARY, 1*S, S, name) << '\n';
    cout << s->createPartitionKb (disk, PRIMARY, 2*S, S, name) << '\n';

    cout << s->createPartitionKb (disk, EXTENDED, 3*S, (n+1)*S, name) << '\n';

    for (int i = 0; i < n; i++)
	cout << s->createPartitionKb (disk, LOGICAL, (3+i)*S, S, name) << '\n';

    cout << s->createPartitionKb (disk, LOGICAL, (3+n)*S, S, name) << '\n'; // FAILS

    print_partitions (disk);
}


void
gpt (const string& disk, int n)
{
    printf ("gpt %s %d\n", disk.c_str(), n);

    s->destroyPartitionTable (disk, "gpt");

    long int S = 100000;
    string name;

    for (int i = 0; i < n; i++)
	cout << s->createPartitionKb (disk, PRIMARY, i*S, S, name) << '\n';

    cout << s->createPartitionKb (disk, PRIMARY, n*S, S, name) << '\n'; // FAILS

    print_partitions (disk);
}


int
main ()
{
    setenv ("YAST2_STORAGE_TDIR", ".", 1);

    s = createStorageInterface (true, true, true);

    /*
     * Check that we can create 3 primary, 1 extended and 59 logical partitions
     * on a ide disk with msdos partition table.
     */
    msdos ("/dev/hda", 59);

    /*
     * Check that we can create 3 primary, 1 extended and 11 logical partitions
     * on a scsi disk with msdos partition table.
     */
    msdos ("/dev/sda", 11);

    /*
     * Check that we can create 63 primary partitions on a ide disk with gpt
     * partition table.
     */
    gpt ("/dev/hda", 63);

    /*
     * Check that we can create 15 primary partitions on a scsi disk with gpt
     * partition table.
     */
    gpt ("/dev/sda", 15);

    delete s;
}
