#include <iostream>
#include <thread>
#include <queue>
#include <numeric>
#include <mutex>
#include <vector>
#include <chrono>
#include <random>

using namespace std;

std::string notificationMsg;
std::queue<int>shared_queue;
std::mutex mtx;

int average(std::vector<int> const& v){
    if(v.empty()) return 0;
    return std::reduce(v.begin(), v.end()) / v.size();
}

template <class T>
class MyTemplate {
    T element;

    public:
        MyTemplate (T arg) {element=arg;}
        void updateQueue () {
            std::lock_guard<std::mutex> guard(mtx);
            shared_queue.push(element);
        }
};

template <>
class MyTemplate <vector<int>> {
    vector<int> vec;

    public:
        MyTemplate (vector<int> arg) {vec=arg;}
    void updateQueue() {
        std::lock_guard<std::mutex> guard(mtx);
        while (!shared_queue.empty()) shared_queue.pop();
        cout << "Vector generated: ";
        for (auto i: vec) {
            cout << i << " ";
            shared_queue.push(i);
        }
        cout << endl;
    }
};

void dataGenerator() {
    double nseconds;
    chrono::steady_clock::time_point clock_now;
    chrono::steady_clock::duration time_span;
    chrono::steady_clock::time_point clock_begin = chrono::steady_clock::now();

    // Instance of random device engine (c++11)
    random_device rnd_device;
    // Specify the engine and distribution.
    mt19937 mersenne_engine {rnd_device()};
    uniform_int_distribution<int> vector_dist {1, 100}; // let's take ints in 1..100 interval
    uniform_int_distribution<int> time_dist {1, 10}; // random time max 10 seconds
    
    auto gen = [&vector_dist, &mersenne_engine](){
                   return vector_dist(mersenne_engine);
               };

    double secs = time_dist(mersenne_engine);
    cout << "Random execution time: " << secs << " seconds" << endl;
    auto counter=0;
    while (true) {
        // some random time 1..10 seconds
        clock_now = chrono::steady_clock::now();
        time_span = clock_now - clock_begin;
        nseconds = double(time_span.count()) * chrono::steady_clock::period::num / chrono::steady_clock::period::den;
        if (nseconds > secs) {
            MyTemplate que(-1);
            que.updateQueue();
            notificationMsg = "no more work (time limit reached)";
            break;
        }
 
        // generating random size vectors of ints 
        vector<int> vec(vector_dist(mersenne_engine));
        generate(begin(vec), end(vec), gen);
        counter++;

        // updating queue for data exchange between threads
        MyTemplate que(vec);
        que.updateQueue();

        // some little sleep between each yield
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    cout << "vectors generated: " << counter << endl;
}

void dataProcessor() {
    vector<int> procvec, tmpvec;
    while (true) {
        if (!shared_queue.empty()) {
            if (shared_queue.front() == -1) {
                while (!shared_queue.empty()) shared_queue.pop();
                shared_queue.push(-2);
                for (auto i: procvec) {
                    shared_queue.push(i);
                }
                break;
            }
            while (!shared_queue.empty()) {
                tmpvec.push_back(shared_queue.front());
                shared_queue.pop();
            }
            procvec.push_back(average(tmpvec));
            tmpvec.erase(tmpvec.begin(), tmpvec.end());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    cout << notificationMsg << endl << "averages: ";
    for (auto i: procvec) {
        cout << i << " ";
    }
    cout << endl;
}

void dataAggregator() {
    vector<int> aggvec;
    while (true) {
        if (!shared_queue.empty()) {
            if (shared_queue.front() == -2) {
                shared_queue.pop();
                while (!shared_queue.empty()) {
                    aggvec.push_back(shared_queue.front());
                    shared_queue.pop();
                }
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    cout << "average of averages: " << average(aggvec) << endl;
}

int main()
{
	std::thread t1(dataGenerator);
	std::thread t2(dataProcessor);
	std::thread t3(dataAggregator);

	t1.join();
	t2.join();
	t3.join();


	return 0;
}
