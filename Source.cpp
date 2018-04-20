#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker, "/STACK:10034217728")
#include <iostream>
#include <cstdio>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cassert>
#include <utility>
#include <ctime>
#include <string>
#include <sstream>
#include <queue>
#include <cstring>
#include <cmath>
#include <random>
#include <unordered_map>
#include <list>
#include <numeric>

using namespace std;
typedef long long ll;

// declarations
class Environment;
class Point;
class Command;
class CommandsSequence;
class Passanger;
class Taxi;

Environment* env; // the only global object

// gets next int from the interactor
int askInt() {
	int res;
	cin >> res;
	return res;
}


// classes definitions
// all functions definitions are out of the class

// our program state
class Environment {
public:
	void ask();

	Environment();

	// getters
	int time() const;
	int width() const;
	int height() const;
	const list<Passanger>& freePassangers();

	void update(Passanger passanger);
	
	int takeNextPsngId();
protected:
	int _time;
	int _width;
	int _height;
	int _max_psng_id;
	vector<Taxi> _taxis;
	list<Passanger> _passangers;
};

// position on plane
class Point {
public:
	void ask();

	Point(int x = 0, int y = 0);

	int getX();
	int getY();
	void setX(int x);
	void setY(int y);

protected:
	int x;
	int y;
};

// one command for taxi, gonna to be extended later
class Command : public Point {};

// sequence of commands for taxi
class CommandsSequence {
protected:
	vector<Command> v; // reversed order
};

class Passanger {
public:
	void ask();

	Passanger();

	int id();
	int time();
	Point from();
	Point to();

	bool isStart()  const;
	bool isFinish() const;

protected:
	int _id;
	int _time;     // time when this passanger appeared
	Point _p_from;
	Point _p_to;
};

class Taxi {
public:
	void ask();
	
	void update(int prevTime, int curTime);

protected:
	Point pos;
	CommandsSequence commands;
	list<Passanger> passangers;
};

enum UpdateState {
	ST_START,
	ST_NORMAL,
	ST_FINISH
};

// main solution function
void updateCommands(UpdateState state) {
	// TODO: write logic and claim function
}

void solve() {
	env = new Environment();
	env->ask();
	updateCommands(ST_START);
	Passanger passanger;
	passanger.ask();
	while (!passanger.isFinish()) {
		env->update(passanger);
		passanger.ask();
		updateCommands(ST_NORMAL);
	}
	updateCommands(ST_FINISH);
}

int main() {

	return 0;
}







// Method definitions
// Environment ==================================================================================
void Environment::ask() {
	_width = askInt();
	_height = askInt();
	int taxiCnt = askInt();
	_taxis.resize(taxiCnt);
	for (auto el : _taxis) el.ask();
}

Environment::Environment() {
	_time = 0;
	_max_psng_id = 0;
}

int Environment::time() const {
	return _time;
}

int Environment::width() const {
	return _width;
}

int Environment::height() const {
	return _height;
}

const list<Passanger>& Environment::freePassangers() {
	return _passangers;
}

void Environment::update(Passanger passanger) {
	_passangers.push_back(passanger);
	int prevTime = _time;
	_time = passanger.time();
	for (auto el : _taxis) {
		el.update(prevTime, time());
	}
}

int Environment::takeNextPsngId() {
	return ++_max_psng_id;
}

// Point ==================================================================================
void Point::ask() {
	x = askInt();
	y = askInt();
}

Point::Point(int x, int y) {
	setX(x);
	setY(y);
}

int Point::getX() {
	return x;
}

int Point::getY() {
	return y;
}

void Point::setX(int x) {
	assert(x >= 0 && x < env->width());
	this->x = x;
}

void Point::setY(int y) {
	assert(y >= 0 && y < env->height());
	this->y = y;
}

// Passanger ==================================================================================
void Passanger::ask() {
	_id = env->takeNextPsngId();
	_p_from.ask();
	_p_to.ask();
}

Passanger::Passanger() {
	_time = 0;
}

int Passanger::id() {
	return _id;
}

int Passanger::time() {
	return _time;
}

Point Passanger::from() {
	return _p_from;
}

Point Passanger::to() {
	return _p_to;
}

bool Passanger::isStart() const {
	return _time == 0;
}

bool Passanger::isFinish() const {
	return _time == -1;
}

// Taxi ==================================================================================
void Taxi::ask() {
	pos.ask();
}

void Taxi::update(int prevTime, int curTime) {
	// TODO: update commands and passangers
}
