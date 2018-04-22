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
#include <tuple>
// #include "Source.h"

using namespace std;
typedef long long ll;

const int INF = 1e9 + 7;
const double DOUBLE_INF = 1e9;
const double EPS = 1e-12;


// defines
#define LOG cout

// 0 - no score logging
// 1 - log only total score in the end
// 2 - log all passengers' score in the end
#define _SCORE_LOG 2

// declarations
class Interactor;
class Environment;

class Point;
class Command;
class CommandsSequence;
class Passenger;
class Taxi;
class SolutionEnvironment;
using PassengersSet = set<Passenger>;
using IdToPassMap = map<int, Passenger>;

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

	const map<int, Passenger>& getFreePassengers() const { return _freePassengers; }
	const vector<Taxi>& getTaxis() const { return _taxis; }
	const Passenger getLastPassenger() const;
	//const IdToPassMap getAllPassengers() const { return _allPassengers; } // no need in this?

	void update(const Passenger &passenger);
	void finishUpdate(); // updates state after passenger finished (with inf time)

	Passenger getFreePassengerById(int id);
	Passenger getWayPassengerById(int id);
	Passenger& getAllPassengerById(int id);
	void setPassengerInWayById(int id);
	void delWayPassengerById(int id);
	
	int takeNextPassengerId();
	int takeNextTaxiId();

	void commit(map<int, CommandsSequence>& c);

	void finishLog();

protected:
	int _time;
	int _width;
	int _height;
	int _maxPsngId;
	int _maxTaxiId;
	vector<Taxi> _taxis;
	IdToPassMap _wayPassengers;
	IdToPassMap _freePassengers;
	IdToPassMap _allPassengers; // used to check score in the end
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

	string toString() const;

protected:
	int x;
	int y;
};

static int getDistance(const Point &a, const Point &b);

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

	bool operator <(const Command c) {
		if (a != c.a) return a < c.a;
		if (x != c.getX()) return x < c.getX();
		return y < c.getY();
	}

protected:	
	int a;
};

// sequence of commands for taxi
class CommandsSequence {
public:
	int size() const { return v.size(); }
	const deque<Command>& getCommands() const { return v; }
	Command getFirst() const { return v.front(); }

	void popFirst() { v.pop_front(); }
	void addCommand(const Command &comm) { v.push_back(comm); }

	int ordersCount() const; // number of different passengers we want to take
	bool isCorrect() const; // check order and that taxi capacity is enough to perform this sequence
	bool isEmpty() { return v.empty(); }

	void nextPermutation() { next_permutation(v.begin(), v.end()); }

	// estimate score we will get for performing these commands
	// if we assign it to certain taxi
	double estimateScore(const Taxi& taxi);

	deque<Command>::iterator begin() { return v.begin(); }
	deque<Command>::iterator end() { return v.end(); }

protected:
	deque<Command> v; // reversed order
};

class Passenger {
public:
	void ask();

	Passenger();

	// getters
	int id() const { return _id; }
	int time() const { return _time; }
	Point from() const { return _pFrom; }
	Point to() const { return _pTo; }

	bool isStart()  const;
	bool isFinish() const;

	bool operator < (const Passenger &p) const { return _id < p._id; }
	
	int getPathLength();
	int getIdealDuration() const { return getDistance(_pFrom, _pTo); } // w0 from statements

	void setWaitingTime(int waitingTime) { _waitingTime = waitingTime; }
	void setTotalDuration(int duration) { _totalDuration = duration; }
	double getScore() const;

	string toString() const;

protected:
	int _id;
	int _time;     // time when this passenger appeared
	Point _pFrom;
	Point _pTo;

	int _waitingTime;   // d1 for score calculation
	int _totalDuration; // used for d2 calculation
};

class Taxi {
public:
	void ask();

	Taxi();
	
	int id() const { return _id; }
	Point pos() const { return _pos; }
	const CommandsSequence& commands() const { return _commands; }
	const PassengersSet& passengers() const { return _passengers; }
	int ordersCount() const { return _commands.ordersCount(); } // number of passengers distributed to this taxi

	void update(int prevTime, int curTime);
	void updateCommands(CommandsSequence commands) { _commands = commands; }

	void addPassenger(const Passenger &p);
	void delPassenger(const Passenger &p);

	int freeSeats() const { return 4 - (int)_passengers.size(); }

protected:
	int _id;
	Point _pos;
	CommandsSequence _commands;
	PassengersSet _passengers;
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
	const map<int, Passenger>& getWaitingPassengers() const { return _waitingPassengers; }
	void setWaitingPassangers(map<int, Passenger> waitingPassengers) { _waitingPassengers = waitingPassengers; }
	vector<Passenger> getVectorWaitingPassengers() const;
	set<Passenger> getSetWaitingPassengers() const;

	bool isWaitingPassenger(const Passenger &p) const {return (bool)_waitingPassengers.count(p.id()); }
	void distributePassenger(const Passenger &p) { _waitingPassengers.erase(p.id()); }
	void addPassenger(const Passenger &p) { _waitingPassengers[p.id()] = p; }

	// reorder commands to get maximum score from it
	void optimizeCommandsOrder(CommandsSequence& commands, const Taxi& taxi) const;
private:
	map<int, Passenger> _waitingPassengers; // list of undistributed to taxis passengers

	const int FULL_REORDER_LIMIT = 8;
};

void clearTaxiCommands(vector<Taxi> &taxis, map<int, CommandsSequence> &mTaxiCommands) {
	sol->setWaitingPassangers(env->getFreePassengers());
	for (auto& taxi : taxis) {
		CommandsSequence commands;
		auto psIntoTaxi = taxi.passengers();
		for (const auto& p : psIntoTaxi) {
			commands.addCommand(Command(p.to(), -p.id()));
			sol->distributePassenger(p);
		}
		taxi.updateCommands(commands);
		mTaxiCommands[taxi.id()] = commands;
	}
}

vector<Passenger> sortPassengerByBestTaxi(const vector<Passenger> &psngrs, const vector<Taxi> &taxis) {
	vector<pair<double, Passenger>> newPassengers;
	for (const auto& p : psngrs) {
		double bestAddition = -DOUBLE_INF;
		for (const auto& taxi : taxis) {
			auto takePas = Command(p.from(), p.id());
			auto dropPas = Command(p.to(), -p.id());
			auto commands = taxi.commands();
			double wasScore = commands.estimateScore(taxi);
			commands.addCommand(takePas);
			commands.addCommand(dropPas);
			sol->optimizeCommandsOrder(commands, taxi);
			double becomeScore = commands.estimateScore(taxi);
			double addition = becomeScore - wasScore;
			if (addition > bestAddition) {
				bestAddition = addition;
			}
		}
		newPassengers.emplace_back(bestAddition, p);
	}
	sort(newPassengers.rbegin(), newPassengers.rend());

	vector<Passenger> res;
	for (auto el : newPassengers) {
		res.push_back(el.second);
	}

	return res;
}

void updateMapCommands(const vector<Passenger> &psngrs, const vector<Taxi> &taxis,
                       map<int, CommandsSequence> &mTaxiCommands)
{
	for (const auto& p : psngrs) {
		int bestTaxiId = -1;
		double bestAddition = -DOUBLE_INF;

		Command takePas = Command(p.from(), p.id());
		Command dropPas = Command(p.to(), -p.id());

		for (auto& taxi : taxis) {
			CommandsSequence commands = mTaxiCommands.count(taxi.id()) ? mTaxiCommands[taxi.id()] : taxi.commands();
			double wasScore = commands.estimateScore(taxi);
			commands.addCommand(takePas);
			commands.addCommand(dropPas);
			sol->optimizeCommandsOrder(commands, taxi);
			double becomeScore = commands.estimateScore(taxi);
			double addition = becomeScore - wasScore;
			if (addition > bestAddition) {
				bestAddition = addition;
				bestTaxiId = taxi.id();
			}
		}
		assert(bestAddition + EPS >= 0);

		sol->distributePassenger(p);

		// updating taxi info
		for (auto& taxi : taxis) {
			if (taxi.id() != bestTaxiId) continue;
			if (!mTaxiCommands.count(taxi.id())) {
				mTaxiCommands[taxi.id()] = taxi.commands();
			}
			mTaxiCommands[taxi.id()].addCommand(takePas);
			mTaxiCommands[taxi.id()].addCommand(dropPas);
			sol->optimizeCommandsOrder(mTaxiCommands[taxi.id()], taxi);
		}
	}
}

// main solution function
map<int, CommandsSequence> calcCommands(UpdateState state, bool flagClearCommands) {
	if (state == ST_NORMAL) {
		sol->addPassenger(env->getLastPassenger());
	}

	vector<Taxi> taxis = env->getTaxis(); // all taxis
	map<int, CommandsSequence> mTaxiCommands;

	if (flagClearCommands) {
		clearTaxiCommands(taxis, mTaxiCommands);
	}

	auto psngrs = sortPassengerByBestTaxi(sol->getVectorWaitingPassengers(), taxis);

	updateMapCommands(psngrs, taxis, mTaxiCommands);
	return mTaxiCommands;
}

void updateAndCommit(UpdateState state) {
	auto m = calcCommands(state, state == ST_FINISH);
	// auto m = calcCommands(state, rand() % 20 == 0);
	env->commit(m);
	interactor->commit(m);
}

void solve() {
	env = new Environment();
	sol = new SolutionEnvironment();
	env->ask();
	updateAndCommit(ST_START);
	Passenger passenger;
	passenger.ask();
	while (!passenger.isFinish()) {
		env->update(passenger);
		updateAndCommit(ST_NORMAL);
		passenger.ask();
	}
	updateAndCommit(ST_FINISH);
	env->finishUpdate();
	env->finishLog();
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
	_maxPsngId = 0;
	_maxTaxiId = 0;
}

const Passenger Environment::getLastPassenger() const {
	int last_id = _maxPsngId;
	return _freePassengers.at(last_id);
}

void Environment::update(const Passenger &passenger) {
	_freePassengers[passenger.id()] = passenger;
	_allPassengers[passenger.id()] = passenger;
	int prevTime = _time;
	_time = passenger.time();
	for (auto& el : _taxis) {
		el.update(prevTime, time());
	}
}

void Environment::finishUpdate() {
	int prevTime = _time;
	_time = INF;
	for (auto& el : _taxis) {
		el.update(prevTime, time());
	}
}

int Environment::takeNextPassengerId() {
	return ++_maxPsngId;
}

int Environment::takeNextTaxiId() {
	return ++_maxTaxiId;
}

void Environment::commit(map<int, CommandsSequence>& c) {
	for (auto& el : _taxis) {
		if (!c.count(el.id())) continue;
		el.updateCommands(c[el.id()]);
	}
}

void Environment::finishLog() {
#ifndef _LOCAL_TEST
	return;
#endif // _LOCAL_TEST
	if (_SCORE_LOG >= 2) {
		LOG << "\n\nSCORE BY PASSENGERS =====================\n";
		for (auto& el : _allPassengers) {
			Passenger& p = el.second;
			LOG << "pass " << p.toString() << " score = " << p.getScore() << "\n";
		}
	}
	if (_SCORE_LOG >= 1) {
		double totalScore = 0;
		for (auto& el : _allPassengers) {
			Passenger& p = el.second;
			totalScore += p.getScore();
		}
		totalScore /= (double)_allPassengers.size();
		LOG << "\nTotal score = " << (ll)(totalScore + 0.5) << "\n";
	}
}

Passenger Environment::getFreePassengerById(int id) {
	assert(_freePassengers.count(id) != 0);
	return _freePassengers[id];
}

void Environment::setPassengerInWayById(int id) {
	assert(_freePassengers.count(id) != 0);
	Passenger p = _freePassengers[id];
	_freePassengers.erase(id);
	_wayPassengers[id] = p;
}

Passenger Environment::getWayPassengerById(int id) {
  assert(_wayPassengers.count(id) != 0);
  return _wayPassengers[id];
}

Passenger & Environment::getAllPassengerById(int id) {
	return _allPassengers[id];
}

void Environment::delWayPassengerById(int id) {
	assert(_wayPassengers.count(id) != 0);
	_wayPassengers.erase(id);
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

string Point::toString() const {
	stringstream ss;
	ss << "(" << x << ", " << y << ")";

	string str;
	getline(ss, str);
	return str;
}

// Passenger ==================================================================================
void Passenger::ask() {
	_id = env->takeNextPassengerId();
	_time = interactor->askInt();
	_pFrom.ask();
	_pTo.ask();
}

Passenger::Passenger() {
	_time = 0;
	_id = -1; // uninitialized
	_waitingTime = -1;   // uninitialized
	_totalDuration = -1; // uninitialized
}

bool Passenger::isStart() const {
	return _time == 0;
}

bool Passenger::isFinish() const {
	return _time == -1;
}

int Passenger::getPathLength() {
  return getDistance(_pFrom, _pTo);
}

double Passenger::getScore() const {
	assert(_waitingTime != -1 && _totalDuration != -1);
	double t = 1e7;
	double d1 = _waitingTime;
	int w0 = getIdealDuration();
	double d2 = _totalDuration - _waitingTime - w0;
	double alpha = (t - min(d1 * d1 + d2 * d2, t)) / t;
	return (ll)(alpha * (100 + w0) + 0.5);
}

string Passenger::toString() const {
	stringstream ss;
	ss << "P( #" << _id << " " << " time: " << _time << " ";
	ss << "from: " << _pFrom.toString() << " to: " << _pTo.toString() << ")";

	string str;
	getline(ss, str);
	return str;
}

// Taxi ==================================================================================
void Taxi::ask() {
	_pos.ask();
	_id = env->takeNextTaxiId();
}

Taxi::Taxi() {
}

void Taxi::addPassenger(const Passenger &p) {
	assert(static_cast<int>(_passengers.size()) < 4);
	_passengers.insert(p);
}

void Taxi::delPassenger(const Passenger &p) {
	assert(_passengers.count(p));
	_passengers.erase(p);
}

void Taxi::update(int prevTime, int curTime) {
	const int timeDiff = curTime - prevTime;
	int restTime = timeDiff;
	while (!_commands.isEmpty()) {
		auto firstComm = _commands.getFirst();
		int needTime = firstComm.getTimeToPerform(_pos);
		if (needTime <= restTime) {
			int nowTime = prevTime + (timeDiff - (restTime - needTime)); // env time when this command will be done
			_commands.popFirst();
			_pos = firstComm.getPoint();
			
			int idPassenger = firstComm.getA();
			if (idPassenger > 0) {
				Passenger p = env->getFreePassengerById(idPassenger);
				assert(p.from() == _pos);
				addPassenger(p);
				env->setPassengerInWayById(idPassenger);
				env->getAllPassengerById(idPassenger).setWaitingTime(nowTime - p.time());
			} else if (idPassenger < 0) {
				idPassenger = -idPassenger;
				Passenger p = env->getWayPassengerById(idPassenger);
				assert(p.to() == _pos);
				delPassenger(p);
				env->delWayPassengerById(idPassenger);
				env->getAllPassengerById(idPassenger).setTotalDuration(nowTime - p.time());
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

// CommandsSequence ==================================================================================
int CommandsSequence::ordersCount() const {
	// TODO: check that passengers are different
	int res = 0;
	for (auto el : v) {
		if (el.getA() > 0) {
			res++;
		}
	}
	return res;
}

bool CommandsSequence::isCorrect() const {
	set<int> openedPassengers;

	// there are possible commands which are partially done
	// when passenger is already in the taxi. So, we mark all
	// these passengers as already opened
	for (auto it = v.rbegin(); it != v.rend(); ++it) {
		Command command = *it;
		int a = command.getA();
		if (0 == a) {
			continue;
		}
		int id = abs(a);
		if (a < 0) {
			openedPassengers.insert(id);
		}
		else {
			if (!openedPassengers.count(id)) {
				return false;
			}
			openedPassengers.erase(id);
		}
	}

	// checking taxi capacity (4 passengers)
	for (auto& command : v) {
		int a = command.getA();
		if (0 == a) {
			continue;
		}
		int id = abs(a);
		if (a > 0) {
			openedPassengers.insert(id);
			if (openedPassengers.size() > 4) {
				return false;
			}
		} else {
			openedPassengers.erase(id);
		}
	}

	assert(openedPassengers.empty());
	return true;
}

double CommandsSequence::estimateScore(const Taxi & taxi) {
	int nowTime = env->time();
	Point p = taxi.pos();
	IdToPassMap passengers; // list of passenger which will be delivered by this command
	for (auto& command : v) {
		int a = command.getA();
		if (a == 0) continue;
		int id = abs(a);
		Point nxtP = command.getPoint();
		nowTime += getDistance(p, nxtP);
		p = nxtP;
		if (!passengers.count(id)) {
			passengers[id] = env->getAllPassengerById(id);
		}
		if (a > 0) {
			passengers[id].setWaitingTime(nowTime - passengers[id].time());
		}
		else {
			passengers[id].setTotalDuration(nowTime - passengers[id].time());
		}
	}
	double res = 0.0;
	for (auto& el : passengers) {
		res += el.second.getScore();
	}
	return res;
}

// SolutionEnvironment ==================================================================================

void SolutionEnvironment::optimizeCommandsOrder(CommandsSequence& commands, const Taxi& taxi) const {
//	cerr << "optimizeCommandsOrder: " << endl;
//	cerr << commands.size() << endl;
	if (commands.size() < FULL_REORDER_LIMIT) {
		int fact = 1;
		for (int i = 2; i < commands.size(); i++) {
			fact *= i;
		}
		CommandsSequence bestSequence = commands;
		double bestScore = bestSequence.estimateScore(taxi);
		for (int i = 0; i < fact; i++) {
			commands.nextPermutation();
			if (!commands.isCorrect()) continue;
			double curScore = commands.estimateScore(taxi);
			if (curScore > bestScore) {
				bestScore = curScore;
				bestSequence = commands;
			}
		}
		commands = bestSequence;
	} else {
		// TODO: when a is zero for a command, it must be at the end
		// TODO: reorder if there are more commands, actually not sure if there are a lot of such cases
		CommandsSequence betterSequence;
		vector<Command> zeroCommands;
		for (auto command : commands) {
			if (0 == command.getA()) {
				zeroCommands.push_back(command);
			} else {
				betterSequence.addCommand(command);
			}
		}
		for (auto command : zeroCommands) {
			betterSequence.addCommand(command);
		}
		commands = betterSequence;
	}
}

vector<Passenger> SolutionEnvironment::getVectorWaitingPassengers() const {
	vector<Passenger> res;
	for (auto el : _waitingPassengers) {
		res.push_back(el.second);
	}
	return res;
}

set<Passenger> SolutionEnvironment::getSetWaitingPassengers() const {
	set<Passenger> res;
	for (auto el : _waitingPassengers) {
		res.insert(el.second);
	}
	return res;
}
