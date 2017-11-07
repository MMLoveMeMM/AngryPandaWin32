#include "stdafx.h"
#include "safequeue.h"

template <typename T>
safequeue<T>::safequeue()
{
	pthread_mutex_init(&mutexsum, NULL);
}

template <typename T>
safequeue<T>::~safequeue()
{
}

template <typename T>
T safequeue<T>::Pop()
{
	pthread_mutex_lock(&mutexsum);
	T=mQueue.pop();
	pthread_mutex_unlock(&mutexsum);
	return T;
}

template <typename T>
void safequeue<T>::Push(T& t)
{
	pthread_mutex_lock(&mutexsum);
	mQueue.push(t);
	pthread_mutex_unlock(&mutexsum);
}

template <typename T>
queue<T> safequeue<T>::popAll()
{
}

template <typename T>
void safequeue<T>::clrAll()
{
	pthread_mutex_destroy(&mutexsum);
}