/*
 * main.cc
 *
 *  Created on: Dec 16, 2018
 *      Author: xiagai
 */

#include "EventLoop.h"
#include "CurrentThread.h"
#include "Thread.h"
#include "Channel.h"

#include <sys/timerfd.h>
#include <string.h>
#include <iostream>

//void threadFunc1() {
//	printf("threadFunc(): pid = %d, tid = %d\n", getpid(), miniws::CurrentThread::tid());
//	miniws::EventLoop loop;
//	loop.loop();
//}
//void test1() {
//	printf("main(): pid = %d, tid = %d\n", getpid(), miniws::CurrentThread::tid());
//	miniws::EventLoop loop;
//
//	miniws::Thread thread(threadFunc1);
//	thread.start();
//
//	loop.loop();
//	pthread_exit(nullptr);
//}
//
//miniws::EventLoop *g_loop;
//void threadFunc2() {
//	printf("threadFunc(): pid = %d, tid = %d\n", getpid(), miniws::CurrentThread::tid());
//	g_loop->loop();
//	printf("tf2 end\n");
//}
//void test2() {
//	printf("main(): pid = %d, tid = %d\n", getpid(), miniws::CurrentThread::tid());
//	miniws::EventLoop loop;
//	g_loop = &loop;
//	miniws::Thread t(threadFunc2);
//	t.start();
//	printf("main2 end\n");
//	t.join();
//
//}

miniws::EventLoop *g_loop;
void timeout() {
	printf("Timeout!\n");
	g_loop->quit();
}

void test3() {
	miniws::EventLoop loop;
	g_loop = &loop;

	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK |TFD_CLOEXEC);
	miniws::Channel channel(&loop, timerfd);
	channel.setReadCallback(timeout);
	channel.enableReading();

	itimerspec howlong;
	bzero(&howlong, sizeof howlong);
	howlong.it_value.tv_sec = 5;
	::timerfd_settime(timerfd, 0, &howlong, NULL);

	loop.loop();
	::close(timerfd);
}

int main() {
	test3();
}


