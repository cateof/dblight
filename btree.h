/* 
 * 
 * HOC 
 * Sep 14 2009. Initial creation of btree.h
 *
 * **
 * This header file defines the I/F that the sqlite B-Tree file
 * subsystem. See comments in the source code for a detailed
 * description of what each I/F routine does
 */ 

#ifndef _BTREE_H
#define _BTREE_H

/* TODO: This definition is just included so other modules compile.
 *       It needs to be revisited
 */
#define SQLITE_N_BTREE_META 10

#ifndef SQLITE_DEFAULT_AUTOVACUUM
#define SQLITE_DEFAULT_AUTOVACUUM 0
#endif

#define BTREE_AUTOVACUUM_NONE 0		/* do not do auto-vacuum */
#define BTREE_AUTOVACUUM_FULL 1		/* do full auto-vacuum	 */
#define BTREE_AUTOVACUUM_INCR 2		/* incremental vacuum	 */

/* forward declarations of structure */
typedef struct Btree Btree;
typedef struct BtCursor BtCursor;
typedef struct BtShared BtShared;
typedef struct BtreeMutexArray BtreeMutexArray;

/*
 * This structure records all of the Btrees that need to hold
 * a mutex before we enter sqlite3VdbeExec(). The Btrees are
 * placed in aBtree[] in order of aBtree[]->pBt. That way,
 * we can always lock and unlock them all quickly
 */
struct BtreeMutexArray {
	int nMutex;
	Btree *aBtree[SQLITE_MAX_ATTACHED+1];
};

int sqlite3BtreeOpen(
	 const char *zFilename,			/* Name of the database file to open */
	 sqlite *db,					/* Associated database connection */
	 Btree **ppBtree,				/* Return open Btree* here */
	 int flags,						/* Flags */
	 int vfsFlags					/* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the 
 * following values.
 *
 * NOTE: These values must match the corresponding PAGER_ values in pager.h
 */
#define BTREE_OMIT_JOURNAL	1	/* do not use journal, no arguments */
#define BTREE_NO_READLOCK	2	/* omit readlocks on readonly files */
#define BTREE_MEMORY		4	/* in-memory db, no argument */
#define BTREE_READONLY		8	/* open the database in read-only mode */
#define BTREE_READWRITE		16	/* open for both reading and writting */
#define BTREE_CREATE		32	/* create the database if it does not exist */

int sqlite3BtreeClose(Btree*);
int sqlite3BtreeSetCacheSize(Btree*,int);
int sqlite3BtreeSetSafetyLevel(Btree*,int,int);
int sqlite3BtreeSyncDisabled(Btree*);
int sqlite3BtreeSetPageSize(Btree *p, int nPagesize, int nReserve, int eFix);
int sqlite3BtreeGetPageSize(Btree*);
int sqlite3BtreeMaxPageCount(Btree*,int);
int sqlite3BtreeGetReserve(Btree*);
int sqlite3BtreeSetAutoVacuum(Btree *, int);
int sqlite3BtreeGetAutoVacuum(Btree *);
int sqlite3BtreeBeginTrans(Btree*,int);
int sqlite3BtreeCommitPhaseOne(Btree*, const char *zMaster);
int sqlite3BtreeCommitPhaseTwo(Btree*);
int sqlite3BtreeCommit(Btree*);
int sqlite3BtreeRollback(Btree*);
int sqlite3BtreeBeginStmt(Btree*,int);
int sqlite3BtreeCreateTable(Btree*, int*, int flags);
int sqlite3BtreeIsInTrans(Btree*);
int sqlite3BtreeIsInReadTrans(Btree*);
int sqlite3BtreeIsInBackup(Btree*);
void *sqlite3BtreeSchema(Btree *, int, void(*)(void *));
int sqlite3BtreeSchemaLocked(Btree *pBtree);
int sqlite3BtreeLockTable(Btree *pBtree, int iTab, u8 isWriteLock);
int sqlite3BtreeSavepoint(Btree *, int, int);

const char *sqlite3BtreeGetFilename(Btree *);
const char *sqlite3BtreeGetJournalname(Btree *);
int sqlite3BtreeCopyFile(Btree *, Btree *);

int sqlite3BtreeIncrVacuum(Btree *);

/* the flags parameter to sqlite3BtreeCreateTable can be the bitwise OR of 
 * the following flags
 */
#define BTREE_INTKEY	1	/* table has only 64-bit signed integer keys */
#define BTREE_ZERODATA	2	/* table has keys only - no data */
#define BTREE_LEAFDATA	4	/* data stored in leaves only. implies INTKEY */

int sqlite3BtreeDropTable(Btree*, int, int*);
int sqlite3BtreeClearTable(Btree*, int, int*);
void sqlite3BtreeTripAllCursors(Btree*, int);

void sqlite3BtreeGetMeta(Btree *pBtree, int idx, u32 *pValue);
int sqlite3BtreeUpdateMeta(Btree*, int idx, u32 value);

/*
** The second parameter to sqlite3BtreeGetMeta or sqlite3BtreeUpdateMeta
** should be one of the following values. The integer values are assigned 
** to constants so that the offset of the corresponding field in an
** SQLite database header may be found using the following formula:
**
**   offset = 36 + (idx * 4)
**
** For example, the free-page-count field is located at byte offset 36 of
** the database file header. The incr-vacuum-flag field is located at
** byte offset 64 (== 36+4*7).
*/
#define BTREE_FREE_PAGE_COUNT     0
#define BTREE_SCHEMA_VERSION      1
#define BTREE_FILE_FORMAT         2
#define BTREE_DEFAULT_CACHE_SIZE  3
#define BTREE_LARGEST_ROOT_PAGE   4
#define BTREE_TEXT_ENCODING       5
#define BTREE_USER_VERSION        6
#define BTREE_INCR_VACUUM         7

int sqlite3BtreeCursor (
	Btree*,							/* Btree containing table to open */
	int iTable,						/* Index of root page */
	int wrFlag,						/* 1 for writing. 0 for read only */
	struct KeyInfo*,				/* First argument to compare function */
	BtCursor *pCursor				/* Space to wrote cursor structure	*/
);

int sqlite3BtreeCursorSize(void);

int sqlite3BtreeCloseCursor(BtCursor*);
int sqlite3BtreeMovetoUnpacked(
  BtCursor*,
  UnpackedRecord *pUnKey,
  i64 intKey,
  int bias,
  int *pRes
);
int sqlite3BtreeCursorHasMoved(BtCursor*, int*);
int sqlite3BtreeDelete(BtCursor*);
int sqlite3BtreeInsert(BtCursor*, const void *pKey, i64 nKey,
                                  const void *pData, int nData,
                                  int nZero, int bias, int seekResult);
int sqlite3BtreeFirst(BtCursor*, int *pRes);
int sqlite3BtreeLast(BtCursor*, int *pRes);
int sqlite3BtreeNext(BtCursor*, int *pRes);
int sqlite3BtreeEof(BtCursor*);
int sqlite3BtreePrevious(BtCursor*, int *pRes);
int sqlite3BtreeKeySize(BtCursor*, i64 *pSize);
int sqlite3BtreeKey(BtCursor*, u32 offset, u32 amt, void*);
const void *sqlite3BtreeKeyFetch(BtCursor*, int *pAmt);
const void *sqlite3BtreeDataFetch(BtCursor*, int *pAmt);
int sqlite3BtreeDataSize(BtCursor*, u32 *pSize);
int sqlite3BtreeData(BtCursor*, u32 offset, u32 amt, void*);
void sqlite3BtreeSetCachedRowid(BtCursor*, sqlite3_int64);
sqlite3_int64 sqlite3BtreeGetCachedRowid(BtCursor*);

char *sqlite3BtreeIntegrityCheck(Btree*, int *aRoot, int nRoot, int, int*);
struct Pager *sqlite3BtreePager(Btree*);

int sqlite3BtreePutData(BtCursor*, u32 offset, u32 amt, void*);
void sqlite3BtreeCacheOverflow(BtCursor *);
void sqlite3BtreeClearCursor(BtCursor *);

#ifndef NDEBUG
int sqlite3BtreeCursorIsValid(BtCursor*);
#endif

#ifndef SQLITE_OMIT_BTREECOUNT
int sqlite3BtreeCount(BtCursor *, i64 *);
#endif

#ifdef SQLITE_TEST
int sqlite3BtreeCursorInfo(BtCursor*, int*, int);
void sqlite3BtreeCursorList(Btree*);
#endif

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#ifndef SQLITE_OMIT_SHARED_CACHE
  void sqlite3BtreeEnter(Btree*);
  void sqlite3BtreeEnterAll(sqlite3*);
#else
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
#endif

#if !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  void sqlite3BtreeLeave(Btree*);
  void sqlite3BtreeEnterCursor(BtCursor*);
  void sqlite3BtreeLeaveCursor(BtCursor*);
  void sqlite3BtreeLeaveAll(sqlite3*);
  void sqlite3BtreeMutexArrayEnter(BtreeMutexArray*);
  void sqlite3BtreeMutexArrayLeave(BtreeMutexArray*);
  void sqlite3BtreeMutexArrayInsert(BtreeMutexArray*, Btree*);
#ifndef NDEBUG
  /* These routines are used inside assert() statements only. */
  int sqlite3BtreeHoldsMutex(Btree*);
  int sqlite3BtreeHoldsAllMutexes(sqlite3*);
#endif
#else

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeEnterCursor(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)
# define sqlite3BtreeMutexArrayEnter(X)
# define sqlite3BtreeMutexArrayLeave(X)
# define sqlite3BtreeMutexArrayInsert(X,Y)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
#endif


#endif