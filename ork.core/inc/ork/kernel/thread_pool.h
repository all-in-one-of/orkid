///////////////////////////////////////////////////////////////////////////////
// orkid thread pools
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#define _USE_TBB

#include <ork/kernel/mutex.h>
#include <queue>
#include <vector>
#include <ork/kernel/any.h>
#include <ork/kernel/concurrent_queue.h>
#include <boost/thread.hpp>

#if defined(ORK_OSX)
#include <libkern/OSAtomic.h>
#elif defined(_USE_TBB)
#include <tbb/atomic.h>
//#include <boost/thread.hpp>
#endif

namespace ork { namespace threadpool {
///////////////////////////////////////////////////////////////////////////////

class task;
class thread_pool;

#if defined(_USE_TBB)
template <typename T> class MyAtomicNum
{
public:
	void Store( int inew )
	{
		mData.fetch_and_store(inew);
	}
	T FetchAndStore( int inew )
	{
		return mData.fetch_and_store(inew);		
	}
	T FetchAndIncrement(int ival=1)
	{
		return mData.fetch_and_add(ival);		
	}
	T FetchAndDecrement()
	{
		return mData.fetch_and_decrement();		
	}
	T Fetch() const
	{
		return mData;		
	}

private:
	tbb::atomic<T>	mData;
};
#else
template <typename T> class MyAtomicNum
{
public:
	void Store( int inew )
	{
		mData.LockForWrite() = inew;
		mData.UnLock();
	}
	T FetchAndStore( int inew )
	{
		T& ref = mData.LockForWrite();
		T iold = ref;
		ref = inew;
		mData.UnLock();
		return iold;		
	}
	T FetchAndIncrement(int ival=1)
	{
		T& ref = mData.LockForWrite();
		T iold = ref;
		ref+=ival;
		mData.UnLock();
		return iold;		
	}
	T FetchAndDecrement()
	{
		T& ref = mData.LockForWrite();
		T iold = ref;
		ref--;
		mData.UnLock();
		return iold;		
	}
	T Fetch() const
	{
		T iold = mData.LockForRead();
		mData.UnLock();
		return iold;		
	}

private:
	LockedResource<T>	mData;
};
#endif
///////////////////////////////////////////////////////////////////////////////

class sub_task
{
public:
	sub_task( task* tsk ) : mpTask(tsk) {}
	task* GetTask() const { return mpTask; }
	template <typename T> const T& GetData() const
	{
		return mSubTaskData.Get<T>();
	}
	template <typename T> void SetData(const T&data)
	{
		mSubTaskData.Set<T>(data);
	}
private:
	task* 				mpTask;
	any256	            mSubTaskData;

};

///////////////////////////////////////////////////////////////////////////////
class thread_pool_worker;

class task
{
public:
	static const int kTaskIdle = -1;
	static const int kTaskFinished = -2;
	task();
	~task();
	void divide(thread_pool* tpool);
	void started();
	void finished();
	void subtask_finished( const sub_task* pst );
	void process( const sub_task* tsk, const thread_pool_worker* ptpw );
	void wait();
	bool HasFinished() const
	{
		int iret = mNumSubTasks.Fetch();
		return (iret==kTaskFinished);
	}
	void IncNumTasks(int ival=1)
	{
		int irval = mNumSubTasks.FetchAndIncrement(ival);
		//OrkAssert(irval<900);
	}
	MyAtomicNum<int>& RefNumSubTasks() { return mNumSubTasks; }

private:

	virtual void 		do_subtask_finished( const sub_task* pst ) = 0;
	virtual void 		do_divide(thread_pool* tpool) = 0;
	virtual void 		do_onstarted() = 0;
	virtual void 		do_onfinished() = 0;
	virtual void 		do_process( const sub_task* tsk, const thread_pool_worker* ptpw ) = 0;

protected:
	MyAtomicNum<int>	mNumSubTasks;
	
};

///////////////////////////////////////////////////////////////////////////////

class thread_pool_worker
{
public:
	thread_pool_worker( thread_pool* ppool );
	void Process();
	void Kill();
	int GetIndex() const { return mTPWIndex; }
private:
	thread_pool*	mpThreadPool;
	bool			mbExitSignal;
	bool			mbExited;
	int				mTPWIndex;
};

///////////////////////////////////////////////////////////////////////////////

struct thread
{
	thread_pool_worker*	mpWorker;
	boost::thread*		mpThread;

	thread();
	~thread();
};


class thread_pool
{
public:
	thread_pool();
	~thread_pool();
	void init( int inumthreads );
	const sub_task* GetSubTask();
	void AddTask( task* ptask );
	void AddSubTask( const sub_task* subtask );
	void AddSubTasks( const orkset<const sub_task*>& subtasks );

private:

	void Lock();
	void UnLock();

	orkvector<thread*>								mThreads;
	ork::ConcurrentQueue<task*>						mTasks;
	ork::ConcurrentQueue<const sub_task*>			mSubTasks;
	ork::LockedResource<int>						mLock;
};

///////////////////////////////////////////////////////////////////////////////

}} // namespace ork