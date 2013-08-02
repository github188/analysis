//  宏哥的消息队列，据说工业标准
/*
 * this file provide obj queue in share memory realization
 * composed by antony
 * 2003-3-17
 * copyright reserved
 */

#ifndef __SHMBOJQUEUE_H_
#define __SHMBOJQUEUE_H_
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
//#include "mylog.h"

template <class st> class shmobjqueue
{

    private:
        char *m_pShm;
        int m_iSemid;
        int m_iObjSize;
        int m_ipckey;

        //记录偏移量地址
        st *shmStart;
        st *shmEnd;
        st *first;
        st *last;

    typedef struct
    {
        unsigned int maxPoolSize;
        unsigned int actPoolSize;
        unsigned int exFirst;
        unsigned int exLast;
    }shmHead;

    shmHead *m_pShmHead;
    union semun {
            int val;
            struct semid_ds *buf;
            ushort *array;
    } arg;

    class guard
    {
        private:
            shmobjqueue<st> *p;
        public:
            guard (shmobjqueue<st> *);
            ~guard();
    };

        char *shmcreate(int ipckey, unsigned long size);
        void asynInit(int ipckey);
        void asynStart();
        void asynEnd();

    public:
        //create memory to contain num st obj
        shmobjqueue(int ipckey, int num = 5000 );
        ~shmobjqueue();
        //get a obj from pool
        bool get(st *);
        //put a obj to pool
        bool put(st *);
        //clear up all objs in pool
        void clearup();
        //detach from shm
        void detach();
        //return true if empty
        bool isEmpty();
        //return pool size
        int getSize();
        int getIpcKey();
        //return is full
        bool isFull();
}
;

template <class st > shmobjqueue<st>::shmobjqueue(int ipckey,int num)
{
    m_ipckey = ipckey;
    this->m_iObjSize = sizeof(st);
    //mylog("init queue shmobjqueue key=%x,size=%d", ipckey,this->m_iObjSize);
    //m_pShm = this->shmcreate(shmfilename, proj_id, num*this->m_iObjSize +sizeof(int)*4);
    m_pShm = this->shmcreate(ipckey, num);
    if(m_pShm == NULL)
    {
        assert(0);
    }
    else
    {
        //mylog("shm=%d",(int)m_pShm);
    }
    this->m_pShmHead = (shmHead *)this->m_pShm;
    if((int)m_pShmHead->maxPoolSize != num)
    {
        //mylog("max num %d,req count %d",m_pShmHead->maxPoolSize, num);
        assert(0);
    }
    this->asynInit(ipckey);
}

template <class st > shmobjqueue<st>::~shmobjqueue()
{
    //mylog("destruct[%x]",m_ipckey);
    this->detach();
}

template <class st > char* shmobjqueue<st>::shmcreate(int ipckey, unsigned long num)
{
    int shmid = 0;
    int size;
    key_t shmkey;

    if(num != 0)
        size = num*this->m_iObjSize +sizeof(shmHead);
    else
        size = 0;
    shmkey = ipckey;

    shmid = shmget( shmkey, size, 0640 );
    if( shmid < 0 )
    {
        shmid = shmget( shmkey, size, IPC_CREAT | 0640);
        if( shmid < 0 )
            return NULL;
        m_pShm = (char *) shmat( shmid, NULL, 0 );
        //m_pShm = (char *) shmat( shmid, NULL, SHM_RND );
        if( m_pShm == (char *)-1 )
            return NULL;
        memset( m_pShm, 0, size );
        m_pShmHead = (shmHead *)m_pShm;
        m_pShmHead->maxPoolSize = num;
        m_pShmHead->actPoolSize = 0;
        m_pShmHead->exFirst = 0;
        m_pShmHead->exLast = 0;

        //记录在类成员变量中,这些都是偏移地址
        this->shmStart = (st *)(m_pShm+sizeof(shmHead));
        this->shmEnd = (st *)(m_pShm+sizeof(shmHead)+sizeof(st)*num);
        this->first = this->shmStart ;
        this->last = this->shmStart ;
    }
    else
    {
        m_pShm = (char *) shmat( shmid, NULL, 0 );
        //mylog("attaching key=%d,shmid:%d,addr:%d",ipckey, shmid, int(m_pShm));
        m_pShmHead = (shmHead *)m_pShm;
            if( m_pShm == (void *)-1 )
        {
            //mylog("error:%s",strerror(errno));
                return NULL;
        }
        this->shmStart = (st *)(m_pShm+sizeof(shmHead));
        this->shmEnd = (st *)(m_pShm+sizeof(shmHead)+sizeof(st)*num);

        this->first = this->shmStart + m_pShmHead->exFirst;
        this->last = this->shmStart + m_pShmHead->exLast;
    }
    //mylog("last=%d", (int)this->last);
    return m_pShm;
}

template <class st > bool shmobjqueue<st>::get(st *p_st)
{
    bool ret;
    guard tmpGuard(this);
    //this->asynStart();
    if(m_pShmHead->actPoolSize >= m_pShmHead->maxPoolSize/2)
    {
        //mylog("danger pool key=%x , size %d", m_ipckey, m_pShmHead->actPoolSize);
    }
    if( m_pShmHead->actPoolSize <0)
    {
        ret =  false;
        //mylog("m_piPoolSize error:%d", m_pShmHead->actPoolSize);
        //force to clearup;
        this->clearup();
    }
    else if(m_pShmHead->actPoolSize == 0)
    {
        ret = false;
        //mylog("pool is empty");
        if(this->first != this->last )
        {
            //mylog("consistent error,%d,%d",(int)this->first, (int)this->last);
        }
    }
    else
    {
        ret = true;
        (m_pShmHead->actPoolSize)--;
        memcpy(p_st, this->first, sizeof(st));
        this->first ++;
        m_pShmHead->exFirst ++;
        if(this->first == this->shmEnd)
        {
            this->first = this->shmStart;
            m_pShmHead->exFirst = 0;
        }
        if(this->first == this->last)
        {
            m_pShmHead->actPoolSize = 0;
        }
    }
    //this->asynEnd();
    return ret;
}

template <class st > bool shmobjqueue<st>::put(st *p_st)
{
    bool ret;
    guard tmpGuard(this);
    //this->asynStart();
    if(m_pShmHead->actPoolSize >= m_pShmHead->maxPoolSize/2)
    {
        //mylog("danger pool key=%x , size %d", m_ipckey, m_pShmHead->actPoolSize);
    }
    if(m_pShmHead->actPoolSize >m_pShmHead->maxPoolSize)
    {

        //mylog("pool size error,max size:%d,actual size:%d",
            //m_pShmHead->maxPoolSize,m_pShmHead->actPoolSize);
        ret =  false;
    }
    else if(m_pShmHead->actPoolSize ==m_pShmHead->maxPoolSize)
    {
        ret =  false;
        //mylog("pool is full,max size:%d",m_pShmHead->maxPoolSize);
        if(this->first != this->last )
        {
            //mylog("consistent error");
        }
    }
    else
    {
        ret = true;
        memcpy(this->last,p_st, sizeof(st));
        this->last++;
        m_pShmHead->exLast++;
        if(this->last == this->shmEnd)
        {
            this->last = this->shmStart;
            m_pShmHead->exLast = 0;
        }
        m_pShmHead->actPoolSize++;

    }
    //this->asynEnd();
    return  ret;
}

template <class st > void shmobjqueue<st>::clearup()
{
    guard tmpGuard(this);
    //this->asynStart();
    this->first = this->shmStart;
    this->last = this->shmStart;

    m_pShmHead->actPoolSize = 0;
    m_pShmHead->exFirst = 0;
    m_pShmHead->exLast = 0;
    //this->asynEnd();
}

template <class st > void shmobjqueue<st>::asynInit(int ipckey)
{
    key_t semkey;
    int value;
    semkey = ipckey;

    m_iSemid = semget(semkey, 0, 0640);
    if(m_iSemid <0)
    {

        m_iSemid = semget(semkey, 1, IPC_CREAT|0640);
        if(m_iSemid<0)
        {
            //mylog("error:%s",strerror( errno ));
        }
        arg.val = 1;
        value = semctl(m_iSemid,0, SETVAL, arg);
        //mylog("create sem value=%d",value);
        return;
    }
    else
    {
        value = semctl(m_iSemid,0, GETVAL, arg);
        //mylog("sem exist,value=%d",value);
        return;
    }
}

template <class st > void shmobjqueue<st>::asynStart()
{
    struct sembuf sops;
    int value;

    sops.sem_num=0;
    sops.sem_op=-1;
    sops.sem_flg=0;

    value = semop(m_iSemid,&sops,1);

    this->first = this->shmStart + m_pShmHead->exFirst;
    this->last = this->shmStart + m_pShmHead->exLast;
    //mylog("asyn start");
    return;
}

template <class st > void shmobjqueue<st>::asynEnd()
{
    struct sembuf sops;
    int value;

    sops.sem_num=0;
    sops.sem_op=1;
    sops.sem_flg=0;

    value = semop(m_iSemid,&sops,1);
    //mylog("asyn end");
    return;
}
template <class st > void shmobjqueue<st>::detach()
{
    if(m_pShm > (void *)0)
        shmdt(m_pShm);
    //mylog("ipckey=%x", m_ipckey);
}

template <class st > bool shmobjqueue<st>::isEmpty()
{
    if(m_pShmHead->actPoolSize == 0)
        return true;
    else
        return false;
}


template <class st > int shmobjqueue<st>::getSize()
{
    int l_size;
    guard tmpGuard(this);
    //asynStart();
    l_size = m_pShmHead->actPoolSize;
    //asynEnd();
    return l_size;
}

template <class st > int shmobjqueue<st>::getIpcKey()
{
    return m_ipckey;
}

template <class st > bool shmobjqueue<st>::isFull()
{
    bool l_bRet;
    guard tmpGuard(this);

    //asynStart();
    l_bRet = (m_pShmHead->actPoolSize == m_pShmHead->maxPoolSize);
    //asynEnd();
    return l_bRet;
}

template <class st > shmobjqueue<st>::guard::guard(shmobjqueue<st> *stp)
{
    this->p = stp;
    p->asynStart();
}

template <class st > shmobjqueue<st>::guard::~guard()
{
    p->asynEnd();
}
#endif
