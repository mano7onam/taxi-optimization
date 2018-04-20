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
// #include "Source.h"

using namespace std;
typedef long long ll;

// declarations
class Interactor;
class Environment;

class Point;
class Command;
class CommandsSequence;
class Passanger;
class Taxi;

// globals
Environment* env;
Interactor* interactor;

// class definitions

// our program state
class Environment {
public:
	void ask();

	Environment();

	// getters
	int time() const { return _time; }
	int width() const { return _width; }
	int height() const { return _height; }

	// uncomment when necessary (but really necessary ???) 
	// const map<int, Passanger>& getFreePassangers() { return _freePassangers; }

	void update(const Passanger &passanger);

	Passanger getFreePassangerById(int id);
	void setPassangerInWayById(int id);
	void delWayPassangerById(int id);
	
	int takeNextPsngId();

protected:
	int _time;
	int _width;
	int _height;
	int _max_psng_id;
	vector<Taxi> _taxis;
	map<int, Passanger> _wayPassangers;
	map<int, Passanger> _freePassangers;
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
	Point performPart(const Point &from, int haveTime);

protected:	
	int a;
};

// sequence of commands for taxi
class CommandsSequence {
public:
	int size() { return v.size(); }
	const deque<Command> getCommands() { return v; }

	Command getFirst() const { return v.front(); }

	void popFirst() { v.pop_front(); }

	void addCommand(const Command &comm) { v.push_back(comm); }

	bool isEmpty() { return v.empty(); }

protected:
	deque<Command> v; // reversed order
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



class Interactor {
public:
	int askInt() {
		int res;
		cin >> res;
		return res;
	}

	// id identified by index in vector in env
	void setTaxiCommands(int taxiId, CommandsSequence new_commands) {
		buffer[taxiId] = new_commands;
	}

	void commit() {
		cout << buffer.size() << "\n";
		for (auto el : buffer) {
			cout << el.first + 1 << " " << el.second.size() << "\n";
			auto& v = el.second.getCommands();
			for (auto com_it = v.rbegin(); com_it != v.rend(); ++com_it) {
				Command command = *com_it;
				cout << command.getPoint().getX() + 1 << " " << command.getPoint().getY() + 1 << " " << command.getA() << " ";
			}
			cout << "\n";
		}
		cout.flush();
		buffer.clear();
	}

private:
	map<int, CommandsSequence> buffer;
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
	interactor = new Interactor();
	return 0;
}



// Method definitions
// Environment ==================================================================================
void Environment::ask() {
	_width = interactor->askInt();
	_height = interactor->askInt();
	int taxiCnt = interactor->askInt();
	_taxis.resize(taxiCnt);
	for (auto el : _taxis) el.ask();
}

Environment::Environment() {
	_time = 0;
	_max_psng_id = 0;
}

void Environment::update(const Passanger &passanger) {
	_freePassangers[passanger.id()] = passanger;
	int prevTime = _time;
	_time = passanger.time();
	for (auto el : _taxis) {
		el.update(prevTime, time());
	}
}

int Environment::takeNextPsngId() {
	return ++_max_psng_id;
}

Passanger Environment::getFreePassangerById(int id) {
	assert(_freePassangers.count(id) != 0);
	return _freePassangers[id];
}

void Environment::setPassangerInWayById(int id) {
	assert(_freePassangers.count(id) != 0);
	Passanger p = _freePassangers[id];
	_freePassangers.erase(id);
	_wayPassangers[id] = p;
}

void Environment::delWayPassangerById(int id) {
	assert(_wayPassangers.count(id) != 0);
	_wayPassangers.erase(id);
}

// Point ==================================================================================
void Point::ask() {
	x = interactor->askInt();
	y = interactor->askInt();
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
	assert(static_cast<int>(_passangers.size()) < 4);
	_passangers.insert(p);
}

void Taxi::delPassanger(const Passanger &p) {
	assert(_passangers.count(p));
	_passangers.erase(p);
}

void Taxi::update(int prevTime, int curTime) {
	// TODO: update commands and passangers
	int restTime = curTime - prevTime;
	while (!_commands.isEmpty()) {
		auto firstComm = _commands.getFirst();
		int needTime = firstComm.getTimeToPerform(_pos);
		if (needTime >= restTime) {
			_commands.popFirst();
			_pos = firstComm.getPoint();
			
			int idPassanger = firstComm.getA();
			if (idPassanger > 0) {
				Passanger p = env->getFreePassangerById(idPassanger);
				addPassanger(p);
				env->setPassangerInWayById(idPassanger);
			} else if (idPassanger < 0) {
				idPassanger = -idPassanger;
				Passanger p = env->getFreePassangerById(idPassanger);
				delPassanger(p);
				env->delWayPassangerById(idPassanger);
			}
			
			restTime -= needTime;
		} else {
			_pos = firstComm.performPart(_pos, restTime);
			break;
		}
	}
}

// Command ==================================================================================
int Command::getTimeToPerform(const Point &from) const {
	return getDistance(from, getPoint());
}

Point Command::performPart(const Point &from, int haveTime) {
	// TODO: calculate new position of the taxi on the path
	int dx = (from.getX() < getPoint().getX()) ? 1 : -1;
	int dy = (from.getY() < getPoint().getY()) ? 1 : -1;
	
	int diffX = abs(from.getX() - getPoint().getX());
	int diffY = abs(from.getY() - getPoint().getY());
	assert(diffX + diffY > haveTime);

	int resX = from.getX();
	int resY = from.getY();

	if (haveTime >= diffX) {
		resX = getPoint().getX();
		resY += dy * (haveTime - diffX);
	} else {
		resX += dx * haveTime;
	}

	return Point(resX, resY);
}
