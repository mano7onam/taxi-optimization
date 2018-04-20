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
	int time() const { return _time; }
	int width() const { return _width; }
	int height() const { return _height; }
	const set<Passanger>& getFreePassangers() { return _passangers; }

	void update(const Passanger &passanger);
	
	int takeNextPsngId();

protected:
	int _time;
	int _width;
	int _height;
	int _max_psng_id;
	vector<Taxi> _taxis;
	set<Passanger> _passangers;
};

// position on plane
class Point {
public:
	void ask();

	Point(int x = 0, int y = 0) : x(x), y(y) {}

	int getX() const { return x; }
	int getY() const { return y; }
	
	void setX(int x);
	void setY(int y);

protected:
	int x;
	int y;
};

// one command for taxi, gonna to be extended later
class Command : public Point {
public:
	Command(int x = 0, int y = 0, int a = 0) : Point(x, y), a(a) {}

	int getA() const { return a; }
	Point getPoint() const { return Point(x, y); }

	// can perform full command
	int getTimeToPerform(const Point &from) const; 

	// perform part of command - get new position of taxi in the path
	Point performPart(const Point &from);

protected:	
	int a;
};

// sequence of commands for taxi
class CommandsSequence {
protected:
	vector<Command> v; // reversed order
};

class Passanger {
public:
	void ask();

	Passanger();

	// getters
	int id() const { return _id; }
	int time() const { return _time; }
	Point from() const { return _p_from; }
	Point to() const { return _p_to; }

	bool isStart()  const;
	bool isFinish() const;

	bool operator < (const Passanger &p) const { return _id < p._id; }

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

	void addPassanger(const Passanger &p);

	void delPassanger(const Passanger &p);

protected:
	Point _pos;
	CommandsSequence _commands;
	set<Passanger> _passangers;
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

void Environment::update(const Passanger &passanger) {
	_passangers.insert(passanger);
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

void Point::setX(int x) {
	assert(x >= 0 && x < env->width());
	this->x = x;
}

void Point::setY(int y) {
	assert(y >= 0 && y < env->height());
	this->y = y;
}

static int getDistance(const Point &a, const Point &b) {
	return abs(a.getX() - b.getX()) + abs(a.getY() - b.getY());
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

bool Passanger::isStart() const {
	return _time == 0;
}

bool Passanger::isFinish() const {
	return _time == -1;
}

// Taxi ==================================================================================
void Taxi::ask() {
	_pos.ask();
}

void Taxi::addPassanger(const Passanger &p) {
	assert(_passangers.size() < 4);
	_passangers.insert(p);
}

void Taxi::delPassanger(const Passanger &p) {
	assert(_passangers.count(p));
	_passangers.erase(p);
}

void Taxi::update(int prevTime, int curTime) {
	// TODO: update commands and passangers
	int restTime = curTime - prevTime;
	
}

// Command ==================================================================================
int Command::getTimeToPerform(const Point &from) const {
	return getDistance(from, getPoint());
}

Point Command::performPart(const Point &from) {
	// TODO: calculate new position of the taxi on the path
	return Point(0, 0);
}
