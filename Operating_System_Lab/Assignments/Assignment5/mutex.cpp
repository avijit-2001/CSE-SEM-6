#include <bits/stdc++.h>
#include <thread>
#include <mutex>

std:: mutex m;
using namespace std;

int myAmount = 0;
void addMoney(){
	m.lock();
	++myAmount;
	// m.unlock();
}
void subMoney(){
	m.lock();
	--myAmount;
	m.unlock();
}
int main(){
	std:: thread t1(addMoney);
	std:: thread t2(subMoney);
	t1.join();
	t2.join();
	cout << myAmount << "\n";
}