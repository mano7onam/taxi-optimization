#define _CRT_SECURE_NO_WARNINGS
//#pragma comment(linker, "/STACK:10034217728")
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
class SolutionEnvironment;
using PassangersSet = set<Passanger>;

// globals
Environment* env;
Interactor* interactor;
SolutionEnvironment* sol;

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

	const map<int, Passanger>& getFreePassangers() const { return _freePassangers; }
	const vector<Taxi>& getTaxis() const { return _taxis; }
	const Passanger getLastPassanger() const;

	void update(const Passanger &passanger);

	Passanger getFreePassangerById(int id);
	void setPassangerInWayById(int id);
	void delWayPassangerById(int id);
	
	int takeNextPassangerId();
	int takeNextTaxiId();

	void commit(map<int, CommandsSequence>& c);

protected:
	int _time;
	int _width;
	int _height;
	int _max_psng_id;
	int _max_taxi_id;
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

	bool operator ==(const Point& b) { return x == b.getX() && y == b.getY(); }

protected:
	int x;
	int y;
};

// one command for taxi, gonna to be extended later
class Command : public Point {
public:
	Command(Point p, int a = 0) : Point(p), a(a) {}
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

	Taxi();
	
	int id() const { return _id; }
	Point pos() const { return _pos; }
	const CommandsSequence& commands() const { return _commands; }
	const PassangersSet& passangers() const { return _passangers; }

	void update(int prevTime, int curTime);
	void updateCommands(CommandsSequence commands) { _commands = commands; }

	void addPassanger(const Passanger &p);
	void delPassanger(const Passanger &p);

	int freeSeats() { return 4 - _passangers.size(); }

protected:
	int _id;
	Point _pos;
	CommandsSequence _commands;
	PassangersSet _passangers;
};



class Interactor {
public:
	Interactor() {
#ifdef _LOCAL_TEST
		freopen("input.txt", "r", stdin);
		freopen("output.txt", "w", stdout);
#endif
	}

	int askInt() {
		int res;
		cin >> res;
		return res;
	}

	void commit(map<int, CommandsSequence>& buffer) {
		cout << buffer.size() << " ";
		for (auto el : buffer) {
			cout << el.first << " " << el.second.size() << " ";
			auto& v = el.second.getCommands();
			for (auto com_it = v.begin(); com_it != v.end(); ++com_it) {
				Command command = *com_it;
				cout << command.getPoint().getX() + 1 << " " << command.getPoint().getY() + 1 << " " << command.getA() << " ";
			}
		}
		cout << endl;
		cout.flush();
	}
};

enum UpdateState {
	ST_START,
	ST_NORMAL,
	ST_FINISH
};

class SolutionEnvironment {
public:
	const map<int, Passanger>& getWaitingPassangers() const { return _waitingPassangers; }
	void distributePassanger(const Passanger& p) { _waitingPassangers.erase(p.id()); }
	void addPassanger(const Passanger& p) { _waitingPassangers[p.id()] = p; }
private:
	map<int, Passanger> _waitingPassangers; // list of undistributed to taxis passangers
};

// main solution function
map<int, CommandsSequence> calcCommands(UpdateState state) {
	vector<Taxi> taxis = env->getTaxis();
	if (state == ST_NORMAL) {
		sol->addPassanger(env->getLastPassanger());
	}
	auto psngrs = sol->getWaitingPassangers(); // unredistributed to taxis passangers

	map<int, CommandsSequence> c;
	while (!psngrs.empty()) {
		random_shuffle(taxis.begin(), taxis.end());
		auto taxi = *taxis.begin();
		if (!taxi.freeSeats()) continue;
		Passanger p = psngrs.begin()->second;
		psngrs.erase(psngrs.begin());
		sol->distributePassanger(p);
		if (!c.count(taxi.id())) {
			c[taxi.id()] = taxi.commands();
		}
		Command take_pas = Command(p.from(), p.id());
		Command drop_pas = Command(p.to(), -p.id());
		c[taxi.id()].addCommand(take_pas);
		c[taxi.id()].addCommand(drop_pas);
	}
	return c;
}

void updateAndCommit(UpdateState state) {
	auto m = calcCommands(state);
	env->commit(m);
	interactor->commit(m);
}

void solve() {
	env = new Environment();
	sol = new SolutionEnvironment();
	env->ask();
	updateAndCommit(ST_START);
	Passanger passanger;
	passanger.ask();
	while (!passanger.isFinish()) {
		env->update(passanger);
		updateAndCommit(ST_NORMAL);
		passanger.ask();
	}
	updateAndCommit(ST_FINISH);
}

int main() {
	interactor = new Interactor();
	solve();
	return 0;
}



// Method definitions
// Environment ==================================================================================
void Environment::ask() {
	_width = interactor->askInt();
	_height = interactor->askInt();
	int taxiCnt = interactor->askInt();
	_taxis.resize(taxiCnt);
	for (auto& el : _taxis) el.ask();
}

Environment::Environment() {
	_time = 0;
	_max_psng_id = 0;
	_max_taxi_id = 0;
}

const Passanger Environment::getLastPassanger() const {
	int last_id = _max_psng_id;
	return _freePassangers.at(last_id);
}

void Environment::update(const Passanger &passanger) {
	_freePassangers[passanger.id()] = passanger;
	int prevTime = _time;
	_time = passanger.time();
	for (auto& el : _taxis) {
		el.update(prevTime, time());
	}
}

int Environment::takeNextPassangerId() {
	return ++_max_psng_id;
}

int Environment::takeNextTaxiId() {
	return ++_max_taxi_id;
}

void Environment::commit(map<int, CommandsSequence>& c) {
	for (auto& el : _taxis) {
		if (!c.count(el.id())) continue;
		el.updateCommands(c[el.id()]);
	}
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
	x = interactor->askInt() - 1;
	y = interactor->askInt() - 1;
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
	_id = env->takeNextPassangerId();
	_time = interactor->askInt();
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
	_id = env->takeNextTaxiId();
}

Taxi::Taxi() {
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
	int restTime = curTime - prevTime;
	while (!_commands.isEmpty()) {
		auto firstComm = _commands.getFirst();
		int needTime = firstComm.getTimeToPerform(_pos);
		if (needTime <= restTime) {
			_commands.popFirst();
			_pos = firstComm.getPoint();
			
			int idPassanger = firstComm.getA();
			if (idPassanger > 0) {
				Passanger p = env->getFreePassangerById(idPassanger);
				assert(p.from() == _pos);
				addPassanger(p);
				env->setPassangerInWayById(idPassanger);
			} else if (idPassanger < 0) {
				idPassanger = -idPassanger;
				Passanger p = env->getFreePassangerById(idPassanger);
				assert(p.to() == _pos);
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
