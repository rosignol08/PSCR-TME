#pragma once

#include "Queue.h"
#include "Job.h"
#include <vector>
#include <thread>
#include <atomic>

namespace pr {

class Pool {
	Queue<Job> queue;
	std::vector<std::thread> threads;
	std::atomic<bool> tourne;
public:
	// create a pool with a queue of given size
	Pool(int qsize) : queue(qsize),tourne(false) {}
	// start the pool with nbthread workers
	void start (int nbthread) {
		tourne = true;
		for (int i = 0; i < nbthread; i++) {
			// syntaxe pour passer une methode membre au thread
			threads.emplace_back(&Pool::worker, this);
		}
	}

	// submit a job to be executed by the pool
	void submit (Job * job) { 
		queue.push(job);
	}
	
	// initiate shutdown, wait for threads to finish
	void stop() {
		tourne = false;
		for (size_t i = 0 ; i < threads.size(); ++i){
			queue.push(nullptr);//pour dire que c'est la fin à chaque mec
		}
		for(auto& taffeur : threads){
			if(taffeur.joinable()){
				taffeur.join();
			}
		}
		threads.clear();
	}
	

private:
	// worker thread function
	void worker() {
		while (true) {
			Job* job = queue.pop();
			//check la fin de service
			if (job == nullptr) {
				break;
			}
			job->run();
			//free le job
			delete job;
		}
	}
};

}
