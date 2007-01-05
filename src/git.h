/*
	Author: Marco Costalba (C) 2005-2006

	Copyright: See COPYING file that comes with this distribution

*/
#ifndef GIT_H
#define GIT_H

#include <QAbstractItemModel>
#include <QPair>
#include "exceptionmanager.h"
#include "common.h"

class QRegExp;
class QTextCodec;
class Annotate;
class Cache;
class DataLoader;
class Domain;
class Git;
class Lanes;
class MyProcess;

class FileHistory : public QAbstractItemModel {
Q_OBJECT
public:
	explicit FileHistory(Git* git);
	~FileHistory();
	void clear(SCRef name);
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int s, Qt::Orientation o, int role = Qt::DisplayRole) const;
	QModelIndex index(int r, int c, const QModelIndex& p = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	int row(SCRef sha);
	int rowCount() const { return _rowCnt; }
	int rowCount(const QModelIndex&) const { return _rowCnt; }
	int columnCount(const QModelIndex&) const { return 5; }

	QString fileName;
	StrVect revOrder;

private slots:
	void on_newRevsAdded(const FileHistory*, const QVector<QString>&);

private:
	friend class DataLoader;
	friend class Git;

	const QString timeDiff(unsigned long secs) const;

	Git* git;

	RevMap revs;
	Lanes* lns;
	uint firstFreeLane;
	QList<QByteArray*> rowData;
	QList<QVariant> _headerInfo;
	int _rowCnt;
	unsigned long _secs;
};

class Git : public QObject {
Q_OBJECT
public:
	explicit Git(QWidget* parent);

	enum BoolOption { // used as self-documenting boolean parameters
		optFalse,
		optSaveCache,
		optGoDown,
		optOnlyLoaded,
		optDragDrop,
		optFold,
		optOnlyInIndex
	};

	enum RefType {
		TAG        = 1,
		BRANCH     = 2,
		CUR_BRANCH = 4,
		REF        = 8,
		APPLIED    = 16,
		UN_APPLIED = 32,
		ANY_REF    = 63
	};

	void checkEnvironment();
	const QString getBaseDir(bool* c, SCRef wd, bool* ok = NULL, QString* gd = NULL);
	bool init(SCRef workDir, bool askForRange, QStringList* filterList, bool* quit);
	bool stop(bool saveCache);
	void setThrowOnStop(bool b);
	bool isThrowOnStopRaised(int excpId, SCRef curContext);
	void setLane(SCRef sha, FileHistory* fh);
	Annotate* startAnnotate(FileHistory* fh, QObject* guiObj);
	const FileAnnotation* lookupAnnotation(Annotate* ann, SCRef fileName, SCRef sha);
	void cancelAnnotate(Annotate* ann);
	void startFileHistory(FileHistory* fh);
	void cancelDataLoading(const FileHistory* fh);
	void cancelProcess(MyProcess* p);
	bool isCommittingMerge() const { return isMergeHead; }
	bool isStGITStack() const { return isStGIT; }
	bool isPatchName(SCRef nm);
	bool isSameFiles(SCRef tree1Sha, SCRef tree2Sha);
	bool isNothingToCommit();
	bool isUnknownFiles() const { return (_wd.otherFiles.count() > 0); }
	bool isTextHighlighter() const { return isTextHighlighterFound; }
	bool isMainHistory(const FileHistory* fh) { return (fh == revData); }
	MyProcess* getDiff(SCRef sha, QObject* receiver, SCRef diffToSha, bool combined);
	MyProcess* getFile(SCRef file, SCRef revSha, QObject* receiver, QString* runOutput);
	MyProcess* getHighlightedFile(SCRef file, SCRef revSha, QObject* rcv, QString* ro);
	bool saveFile(SCRef fileName, SCRef sha, SCRef path);
	void getFileFilter(SCRef path, QMap<QString, bool>& shaMap);
	bool getPatchFilter(SCRef exp, bool isRegExp, QMap<QString, bool>& shaMap);
	const RevFile* getFiles(SCRef sha, SCRef sha2 = "", bool all = false, SCRef path = "");
	bool getTree(SCRef ts, SList nm, SList sha, SList type, bool wd, SCRef treePath);
	const QString getLocalDate(SCRef gitDate);
	const QString getDesc(SCRef sha, QRegExp& shortLogRE, QRegExp& longLogRE);
	const QString getDefCommitMsg();
	const QString getLaneParent(SCRef fromSHA, int laneNum);
	const QStringList getChilds(SCRef parent);
	const QStringList getNearTags(bool goDown, SCRef sha);
	const QStringList getDescendantBranches(SCRef sha);
	const QString getShortLog(SCRef sha);
	const QString getTagMsg(SCRef sha);
	const Rev* revLookup(SCRef sha, const FileHistory* fh = NULL) const;
	uint checkRef(SCRef sha, uint mask = ANY_REF) const;
	const QString getRevInfo(SCRef sha);
	const QString getRefSha(SCRef refName, RefType type = ANY_REF, bool askGit = true);
	const QStringList getRefName(SCRef sha, RefType type, QString* curBranch = NULL) const;
	const QStringList getAllRefNames(uint mask, bool onlyLoaded);
	const QStringList getAllRefSha(uint mask);
	void getWorkDirFiles(SList files, SList dirs, const QChar& status = QChar());
	QTextCodec* getTextCodec(bool* isGitArchive);
	bool formatPatch(SCList shaList, SCRef dirPath, SCRef remoteDir = "");
	bool updateIndex(SCList selFiles);
	bool commitFiles(SCList files, SCRef msg);
	bool makeTag(SCRef sha, SCRef tag, SCRef msg);
	bool deleteTag(SCRef sha);
	bool applyPatchFile(SCRef patchPath, bool commit, bool fold, bool sign);
	bool resetCommits(int parentDepth);
	bool stgCommit(SCList selFiles, SCRef msg, SCRef patchName, bool fold);
	bool stgPush(SCRef sha);
	bool stgPop(SCRef sha);
	void setTextCodec(QTextCodec* tc);
	void addExtraFileInfo(QString* rowName, SCRef sha, SCRef diffToSha, bool allMergeFiles);
	void removeExtraFileInfo(QString* rowName);
	void formatPatchFileHeader(QString* rowName, SCRef sha, SCRef dts, bool cmb, bool all);
	int findFileIndex(const RevFile& rf, SCRef name);
	const QString filePath(const RevFile& rf, uint i) const {

		return dirNamesVec[rf.dirs[i]] + fileNamesVec[rf.names[i]];
	}
	void setCurContext(Domain* d) { curDomain = d; }
	Domain* curContext() const { return curDomain; }

signals:
	void newRevsAdded(const FileHistory*, const QVector<QString>&);
	void loadCompleted(const FileHistory*, const QString&);
	void cancelLoading(const FileHistory*);
	void cancelAllProcesses();
	void annotateReady(Annotate*, const QString&, bool, const QString&);

public slots:
	void procReadyRead(const QString&);
	void procFinished() { filesLoadingPending = filesLoadingCurSha = ""; }

private slots:
	void loadFileNames();
	void on_runAsScript_eof();
	void on_getHighlightedFile_eof();
	void on_newDataReady(const FileHistory*);
	void on_loaded(const FileHistory*, ulong,int,bool,const QString&,const QString&);
private:
	friend class Annotate;
	friend class MainImpl;
	friend class DataLoader;
	friend class ConsoleImpl;
	friend class RevsView;

	struct Reference { // stores tag information associated to a revision
		Reference() : type(0) {}
		uint type;
		QStringList branches;
		QString     currentBranch;
		QStringList tags;
		QStringList refs;
		QString     tagObj; // TODO support more then one obj
		QString     tagMsg;
		QString     stgitPatch;
	};
	typedef QMap<QString, Reference> RefMap;
	RefMap refsShaMap;

	struct WorkingDirInfo {
		void clear() { diffIndex = diffIndexCached = ""; otherFiles.clear(); }
		QString diffIndex;
		QString diffIndexCached;
		QStringList otherFiles;
	};
	WorkingDirInfo _wd;

	bool run(SCRef cmd, QString* out = NULL, QObject* rcv = NULL, SCRef buf = "");
	MyProcess* runAsync(SCRef cmd, QObject* rcv, SCRef buf = "");
	MyProcess* runAsScript(SCRef cmd, QObject* rcv = NULL, SCRef buf = "");
	bool allProcessDeleted();
	const QString getArgs(bool askForRange, bool* quit);
	bool getRefs();
	bool getStGITPatches();
	bool addStGitPatch(SCRef pn, SCList files, SCList filesSHA, bool applied);
	void dirWalker(SCRef dirPath, SList files, SList filesSHA, SCRef nameFilter = "");
	void clearRevs();
	void clearFileNames();
	bool startRevList(SCRef args, FileHistory* fh);
	bool startUnappliedList();
	bool startParseProc(SCRef initCmd, FileHistory* fh);
	int addChunk(FileHistory* fh, const QByteArray& ba, int ofs);
	void parseDiffFormat(RevFile& rf, SCRef buf);
	void parseDiffFormatLine(RevFile& rf, SCRef line, int parNum);
	void getDiffIndex();
	const QString getFileSha(SCRef file, SCRef revSha);
	const Rev* fakeWorkDirRev(SCRef parent, SCRef log, SCRef longLog, int idx, FileHistory* fh);
	const RevFile* fakeWorkDirRevFile(const WorkingDirInfo& wd);
	void copyDiffIndex(FileHistory* fh, SCRef parent);
	const RevFile* insertNewFiles(SCRef sha, SCRef data);
	const RevFile* getAllMergeFiles(const Rev* r);
	bool isParentOf(SCRef par, SCRef child);
	bool isTreeModified(SCRef sha);
	void indexTree();
	void annotateExited(Annotate* ann);
	void updateDescMap(const Rev* r, uint i, QMap<QPair<uint, uint>,bool>& dm,
	                   QMap<uint, QVector<int> >& dv);
	void mergeNearTags(bool down, Rev* p, const Rev* r, const QMap<QPair<uint, uint>, bool>&dm);
	void mergeBranches(Rev* p, const Rev* r);
	void updateLanes(Rev& c, Lanes& lns, SCRef sha);
	void removeFiles(SCList selFiles, SCRef workDir, SCRef ext);
	void restoreFiles(SCList selFiles, SCRef workDir, SCRef ext);
	bool mkPatchFromIndex(SCRef msg, SCRef patchFile);
	const QStringList getOthersFiles();
	const QStringList getOtherFiles(SCList selFiles, bool onlyInIndex);
	const QString colorMatch(SCRef txt, QRegExp& regExp);
	void appendFileName(RevFile& rf, SCRef name);
	void populateFileNamesMap();
	const QString quote(SCRef nm);
	const QString quote(SCList sl);
	const QStringList noSpaceSepHack(SCRef cmd);
	void removeDeleted(SCList selFiles);
	void setStatus(RevFile& rf, SCRef stInfo, int parNum);
	void appendNamesWithId(QStringList& names, SCRef sha, SCList data, bool onlyLoaded);
	Reference* lookupReference(SCRef sha, bool create = false);

	EM_DECLARE(exGitStopped);

	Cache* cache;
	Domain* curDomain;
	QString workDir; // workDir is always without trailing '/'
	QString gitDir;
	QString filesLoadingPending;
	QString filesLoadingCurSha;
	QString curRange;
	bool cacheNeedsUpdate;
	bool errorReportingEnabled;
	bool isMergeHead;
	bool isStGIT;
	bool isGIT;
	bool isTextHighlighterFound;
	bool loadingUnAppliedPatches;
	bool fileCacheAccessed;
	int patchesStillToFind;
	QString firstNonStGitPatch;
	RevFileMap revsFiles;
	StrVect fileNamesVec;
	StrVect dirNamesVec;
	QMap<QString, int> fileNamesMap; // quick lookup file name
	QMap<QString, int> dirNamesMap;  // quick lookup directory name
	FileHistory* revData;
};

#endif
