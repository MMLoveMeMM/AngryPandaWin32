#pragma once
#include<queue>
#include<iostream>
using namespace std;
#include "pthread.h"
/*
* һ�����ڲ������ݺ���������֮���һ���������ݶ���,��ȫpop��push
*/
template <typename T>
class safequeue
{
public:
	safequeue();
	~safequeue();
public:
	T Pop();
	void Push(T&);
	queue<T> popAll();
	void clrAll();
private:
	std::queue<T> mQueue;
	pthread_mutex_t mutexsum;

};

