#define _CRT_SECURE_NO_WARNINGS
#pragma comment(linker, "/STACK:10034217728")
#include <iostream>
#include <fstream>
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
const int MAX_ID = 1000;

const int MAX_SECONDS_CNT = 11;


// defines
#define LOG cout
//#define LOG_ALL_STATES

// 0 - no score logging
// 1 - log only total score in the end
// 2 - log all passengers' score in the end
#define _SCORE_LOG 1
#define _TAXI_LOG 1

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

	const IdToPassMap& getFreePassengers() const { return _freePassengers; }
	const vector<Taxi>& getTaxis() const { return _taxis; }
	const Passenger getLastPassenger() const;
	//const IdToPassMap getAllPassengers() const { return _allPassengers; } // no need in this?

	void update(const Passenger &passenger);
	void finishUpdate(); // updates state after passenger finished (with inf time)

	Passenger getFreePassengerById(int id);
	Passenger getWayPassengerById(int id);
	Passenger& getAllPassengerById(int id);
	Passenger& getArrPassengerById(int id);
	void setPassengerInWayById(int id);
	void delWayPassengerById(int id);

	int takeNextPassengerId();
	int takeNextTaxiId();

	void commit(map<int, CommandsSequence>& c);

	void taxisLog();
	void finishLog();

	bool isRecentPassenger(const Passenger &p) const;
	bool isRecentPassenger(int id) const;

	bool taxisHaveAnyCommands() const;

	void logStateToDraw(const string &filename, int logTime) const;

	bool checkToDoOptimizations() const;

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

	Passenger* _passArray;

	long long startWorkingTime;
	mutable bool flagDoOptimizations;
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

bool operator <(const Point& a, const Point& b) {
	if (a.getX() != b.getX()) return a.getX() < b.getX();
	return a.getY() < b.getY();
}

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

	string toStringDraw() const;

protected:
	int a;
};

// sequence of commands for taxi
using CommandsList = list<Command>;
int hasCorrect[MAX_ID];
class CommandsSequence {
public:
	int size() const { return v.size(); }
	const CommandsList& getCommands() const { return v; }
	Command getFirst() const { return v.front(); }

	void popFirst() { v.pop_front(); }
	void addCommand(const Command &comm) { v.push_back(comm); }
	Command back() { return *--v.end(); }

	Command takeLast() {
		Command res = v.back();
		v.pop_back();
		return res;
	}

	int ordersCount() const; // number of different passengers we want to take
	bool isCorrectS() const;
	bool isCorrectL() const;
	bool isCorrect() const; // check order and that taxi capacity is enough to perform this sequence
	bool isEmpty() { return v.empty(); }

	void clearZeroCommands();

	void nextPermutation() { next_permutation(v.begin(), v.end()); }

	// estimate score we will get for performing these commands
	// if we assign it to certain taxi
	double estimateScore(const Taxi& taxi);

	void insert(CommandsSequence cs, int ind);
	void insert(Command c, int ind);
	void delPrev(int ind);

	void delPassenger(int id);
	void pickUpPassenger(const Passenger &p);

	CommandsList::iterator begin() { return v.begin(); }
	CommandsList::iterator end() { return v.end(); }

	CommandsList::reverse_iterator rbegin() { return v.rbegin(); }
	CommandsList::reverse_iterator rend() { return v.rend(); }

	CommandsList::const_iterator begin() const { return v.begin(); }
	CommandsList::const_iterator end() const { return v.end(); }
protected:
	CommandsList v; // reversed order
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

	string toStringToDraw() const;

	void distributeToTaxi(int idTaxi) { _idTaxi = idTaxi; }
	void deleteDistributionToTaxi() { _idTaxi = -1; }

protected:
	int _id;
	int _time;     // time when this passenger appeared
	Point _pFrom;
	Point _pTo;

	int _waitingTime;   // d1 for score calculation
	int _totalDuration; // used for d2 calculation

	int _idTaxi;
};

class Taxi {
public:
	void ask();

	Taxi();

	int id() const { return _id; }
	Point pos() const { return _pos; }
	const CommandsSequence& commands() const { return _commands; }
	const PassengersSet& passengers() const { return _passengers; }
	list<int> passengersIds() const;
	int ordersCount() const { return _commands.ordersCount(); } // number of passengers distributed to this taxi

	void update(int prevTime, int curTime);
	void updateCommands(const CommandsSequence &commands) { _commands = commands; }

	bool isGoodToPickUp(const Passenger &p) const;
	void pickUpPassenger(const Passenger &p);

	void addPassenger(const Passenger &p);
	void delPassenger(const Passenger &p);
	bool havePassenger(int id) const { return (bool)_passengersIds.count(id); }

	int freeSeats() { return 4 - (int)_passengers.size(); }

	bool isAtBorder() const { return pos().getX() == env->width() - 1 || pos().getY() == env->height() - 1; }

	string toStringToDraw() const;

protected:
	int _id;
	Point _pos;
	CommandsSequence _commands;
	PassengersSet _passengers;
	set<int> _passengersIds;
};

bool operator <(const Taxi& a, const Taxi& b) {
	return a.id() < b.id();
}

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
	const IdToPassMap& getWaitingPassengers() const { return _waitingPassengers; }
	void setWaitingPassangers(IdToPassMap waitingPassengers) { _waitingPassengers = waitingPassengers; }
	vector<Passenger> getVectorWaitingPassengers() const;
	set<Passenger> getSetWaitingPassengers() const;

	pair<double, CommandsSequence> getBestPermutation(CommandsSequence cs, const Taxi& taxi);
	void generateValidPermutations(CommandsSequence& cur, int mask, CommandsSequence& initial, const Taxi& taxi);

	bool isWaitingPassenger(const Passenger &p) const { return (bool)_waitingPassengers.count(p.id()); }
	void distributePassenger(const Passenger &p) { _waitingPassengers.erase(p.id()); }
	void distributePassenger(int id) { _waitingPassengers.erase(id); }
	void addPassenger(const Passenger &p) { _waitingPassengers[p.id()] = p; }

	// reorder commands to get maximum score from it
	void optimizeCommandsOrder(CommandsSequence& commands, const Taxi& taxi) const;

	set<Point> generatePointsForTaxisByPassengers(const vector<Passenger> &psngrs, int n) const;
	set<Point> generatePointsForTaxisByTaxis(const vector<Taxi> &taxis, int num) const;
	void distributeFreeTaxis(const vector<Passenger> &psngrs, map<int, CommandsSequence>& c);
private:
	IdToPassMap _waitingPassengers; // list of undistributed to taxis passengers

	const int FULL_REORDER_LIMIT = 9;

	// fields used by generateValidPermutations functions
	int _pcnt;
	CommandsSequence _pbestSequence;
	double _pbestScore;
};

void clearTaxiCommands(vector<Taxi> &taxis, map<int, CommandsSequence> &mTaxiCommands) {
	if (!env->checkToDoOptimizations()) {
		return;
	}
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

void clearTaxiCommandsRecentPassenger(vector<Taxi> &taxis, map<int, CommandsSequence> &mTaxiCommands) {
	if (!env->checkToDoOptimizations()) {
		return;
	}
	sol->setWaitingPassangers(env->getFreePassengers());
	for (auto& taxi : taxis) {
		CommandsSequence newCommands;
		auto& lastCommands = taxi.commands();
		auto psIntoTaxi = taxi.passengers();
		for (const auto& command : lastCommands) {
			int id = abs(command.getA());
			if (taxi.havePassenger(id) || !env->isRecentPassenger(id)) {
				sol->distributePassenger(id);
				newCommands.addCommand(command);
			}
		}
		taxi.updateCommands(newCommands);
		mTaxiCommands[taxi.id()] = newCommands;
	}
}

vector<Passenger> sortPassengerByBestTaxi(const vector<Passenger> &psngrs, const vector<Taxi> &taxis) {
	vector<pair<double, Passenger>> newPassengers;
	for (const auto& p : psngrs) {
		double bestAddition = -DOUBLE_INF;
		/*for (const auto& taxi : taxis) {
			auto takePas = Command(p.from(), p.id());
			auto dropPas = Command(p.to(), -p.id());
			auto commands = taxi.commands();
			double wasScore = commands.estimateScore(taxi);
			commands.addCommand(takePas);
			commands.addCommand(dropPas);
			commands.clearZeroCommands();
			sol->optimizeCommandsOrder(commands, taxi);
			double becomeScore = commands.estimateScore(taxi);
			double addition = becomeScore - wasScore;
			if (addition > bestAddition) {
				bestAddition = addition;
			}
		}
		newPassengers.emplace_back(bestAddition, p);*/
		double u = 100 + p.getIdealDuration();
		double t = env->time() - p.time();
		double d = t * u;
		newPassengers.emplace_back(d, p);
	}
	sort(newPassengers.rbegin(), newPassengers.rend());

	vector<Passenger> res;
	for (auto el : newPassengers) {
		res.push_back(el.second);
	}

	return res;
}

void updateTaxiInfo(map<int, CommandsSequence> &mTaxiCommands, const vector<Taxi> &taxis,
	int taxiId, Command takePas, Command dropPas)
{
	// updating taxi info
	for (auto& taxi : taxis) {
		if (taxi.id() != taxiId) continue;
		if (!mTaxiCommands.count(taxi.id())) {
			mTaxiCommands[taxi.id()] = taxi.commands();
		}
		mTaxiCommands[taxi.id()].addCommand(takePas);
		mTaxiCommands[taxi.id()].addCommand(dropPas);
		mTaxiCommands[taxi.id()].clearZeroCommands();
		sol->optimizeCommandsOrder(mTaxiCommands[taxi.id()], taxi);
	}
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
			commands.clearZeroCommands();
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
		updateTaxiInfo(mTaxiCommands, taxis, bestTaxiId, takePas, dropPas);
	}
}

void updateMapCommandsBrutforcePassengersPermutation(const vector<Passenger> &sourcePsngrs,
	const vector<Taxi> &taxis,
	map<int, CommandsSequence> &mTaxiCommands)
{
	vector<Passenger> psngrs = sourcePsngrs;
	vector<Passenger> bestPsngrsSeq;
	vector<int> bestTaxiIds;
	double bestTotalScore = -DOUBLE_INF;

	sort(psngrs.begin(), psngrs.end());
	while (next_permutation(psngrs.begin(), psngrs.end())) {
		vector<int> bti;
		double currentScore = 0;
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
				commands.clearZeroCommands();
				sol->optimizeCommandsOrder(commands, taxi);
				double becomeScore = commands.estimateScore(taxi);
				double addition = becomeScore - wasScore;
				if (addition > bestAddition) {
					bestAddition = addition;
					bestTaxiId = taxi.id();
				}
			}
			assert(bestAddition + EPS >= 0);
			bti.push_back(bestTaxiId);
			currentScore += bestAddition;
		}

		if (currentScore > bestTotalScore) {
			bestTotalScore = currentScore;
			bestPsngrsSeq = psngrs;
			bestTaxiIds = bti;
		}
	}

	for (int i = 0; i < bestPsngrsSeq.size(); ++i) {
		Passenger p = bestPsngrsSeq[i];
		Command takePas = Command(p.from(), p.id());
		Command dropPas = Command(p.to(), -p.id());
		int taxiId = bestTaxiIds[i];
		sol->distributePassenger(p);
		updateTaxiInfo(mTaxiCommands, taxis, taxiId, takePas, dropPas);
	}
}

// main solution function
map<int, CommandsSequence> calcCommands(UpdateState state, bool flagClearCommands) {
	if (state == ST_NORMAL) {
		sol->addPassenger(env->getLastPassenger());
	}

	vector<Taxi> taxis = env->getTaxis(); // all taxis
	map<int, CommandsSequence> mTaxiCommands;

	if (flagClearCommands && env->checkToDoOptimizations()) {
		clearTaxiCommands(taxis, mTaxiCommands);
	}
	if (false && env->checkToDoOptimizations()) {
		clearTaxiCommandsRecentPassenger(taxis, mTaxiCommands);
	}

	auto psngrs = sortPassengerByBestTaxi(sol->getVectorWaitingPassengers(), taxis);

	if (/*psngrs.size() < 5*/false && env->checkToDoOptimizations()) {
		updateMapCommandsBrutforcePassengersPermutation(psngrs, taxis, mTaxiCommands);
	}
	else {
		updateMapCommands(psngrs, taxis, mTaxiCommands);
	}
	sol->distributeFreeTaxis(psngrs, mTaxiCommands);
	return mTaxiCommands;
}

void updateAndCommit(UpdateState state) {
	static int c = 0;
	auto m = calcCommands(state, state == ST_FINISH/* || ++c % 20 == 0*/);
	// auto m = calcCommands(state, rand() % 20 == 0);
	env->commit(m);
	interactor->commit(m);
}

void solve() {
	env = new Environment();
	sol = new SolutionEnvironment();
	memset(hasCorrect, 0, sizeof(hasCorrect));
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
#ifdef _LOCAL_TEST
	cout << "\nTIME ELAPSED: " << 1. * clock() / CLOCKS_PER_SEC << " sec\n";
#endif
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

	_passArray = new Passenger[MAX_ID];

	startWorkingTime = clock();
	flagDoOptimizations = true;
}

const Passenger Environment::getLastPassenger() const {
	int last_id = _maxPsngId;
	return _freePassengers.at(last_id);
}

void Environment::update(const Passenger &passenger) {
	int prevTime = _time;
	_time = passenger.time();
	//cerr << _time << endl;

#ifdef LOG_ALL_STATES
	for (int curTime = prevTime + 1; curTime < _time; ++curTime) {
		for (auto& el : _taxis) {
			el.update(curTime - 1, curTime);
		}
		logStateToDraw("C:\\Users\\amalykh\\source\\repos\\WildcardVisualizer\\WildcardVisualizer\\bin\\Release\\log", curTime);
	}
#endif

	_freePassengers[passenger.id()] = passenger;
	_allPassengers[passenger.id()] = passenger;
	_passArray[passenger.id()] = passenger;

#ifdef LOG_ALL_STATES
	for (auto& el : _taxis) {
		el.update(_time - 1, _time);
	}
#else
	for (auto& el : _taxis) {
		el.update(prevTime, time());
	}
#endif


#ifdef LOG_ALL_STATES
	logStateToDraw("log", _time);
#endif
}

bool Environment::taxisHaveAnyCommands() const {
	for (const auto& taxi : _taxis) {
		if (taxi.commands().size()) {
			return true;
		}
	}
	return false;
}

void Environment::finishUpdate() {
#ifdef LOG_ALL_STATES
	int prevTime = _time;
	_time = INF;
	int curTime = prevTime + 1;
	while (taxisHaveAnyCommands()) {
		for (auto& el : _taxis) {
			el.update(curTime - 1, curTime);
		}
		logStateToDraw("log", curTime);
		curTime++;
	}
#else
	int prevTime = _time;
	_time = INF;
	for (auto& el : _taxis) {
		el.update(prevTime, time());
	}
#endif
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

void Environment::taxisLog() {
	vector<string> t(height());
	string s = "";
	for (int i = 0; i < width(); i++) s += ".";
	for (int i = 0; i < height(); i++) t[i] = s;
	for (auto& taxi : _taxis) {
		Point p = taxi.pos();
		char c = '0' + taxi.commands().size();
		if (_TAXI_LOG == 1) c = '#';
		t[p.getY()][p.getX()] = c;
	}
	for (auto& el : _freePassengers) {
		Passenger p = el.second;
		int add = rand() % 26;
		char ctake = 'A' + add;
		char cdrop = 'a' + add;
		Point from = p.from();
		Point to = p.to();
		if (_TAXI_LOG >= 2) {
			t[from.getY()][from.getX()] = ctake;
			t[to.getY()][to.getX()] = cdrop;
		}
	}
	for (int i = 0; i < height(); i++) {
		LOG << t[i] << "\n";
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

Passenger& Environment::getAllPassengerById(int id) {
	return _allPassengers[id];
}

Passenger & Environment::getArrPassengerById(int id) {
	return _passArray[id];
}

void Environment::delWayPassengerById(int id) {
	assert(_wayPassengers.count(id) != 0);
	_wayPassengers.erase(id);
}

void Environment::logStateToDraw(const string &filename, int logTime) const {
	string myfilename = "C:\\Users\\amalykh\\source\\repos\\WildcardVisualizer\\WildcardVisualizer\\bin\\Release\\log";
	ofstream out(myfilename + std::to_string(logTime));

	//cerr << logTime << endl;

	out << logTime << endl;

	out << _width << ' ' << _height << endl;

	out << _freePassengers.size() << endl;
	for (const auto& p : _freePassengers) {
		out << p.second.toStringToDraw() << endl;
	}

	out << _wayPassengers.size() << endl;
	for (const auto &p : _wayPassengers) {
		out << p.second.toStringToDraw() << endl;
	}

	out << _taxis.size() << endl;
	for (const auto& taxi : _taxis) {
		out << taxi.toStringToDraw() << endl;
	}
	out.close();
}

bool Environment::isRecentPassenger(const Passenger &p) const {
	return _maxPsngId - p.id() < 4;
}

bool Environment::isRecentPassenger(int id) const {
	return _maxPsngId - id < 4;
}

bool Environment::checkToDoOptimizations() const {
	if (!flagDoOptimizations) {
		return false;
	}
	double currTime = (double)(clock() - startWorkingTime) / CLOCKS_PER_SEC;
	if (currTime > MAX_SECONDS_CNT) {
		flagDoOptimizations = false;
	}
	return flagDoOptimizations;
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

string Passenger::toStringToDraw() const {
	stringstream ss;
	ss << _id << ' ' << _time << ' ' <<
		_pFrom.getX() << ' ' << _pFrom.getY() << ' ' <<
		_pTo.getX() << ' ' << _pTo.getY() << endl;

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

bool Taxi::isGoodToPickUp(const Passenger &p) const {
	auto firstCommand = *commands().begin();
	double oldDist = getDistance(_pos, firstCommand.getPoint());
	double newDist =
			getDistance(_pos, p.from()) +
			getDistance(p.from(), p.to()) +
			getDistance(p.to(), firstCommand.getPoint());
	return newDist < oldDist * 1.5;
}

void Taxi::addPassenger(const Passenger &p) {
	assert(static_cast<int>(_passengers.size()) < 4);
	_passengers.insert(p);
	_passengersIds.insert(p.id());
}

void Taxi::delPassenger(const Passenger &p) {
	assert(_passengers.count(p));
	_passengers.erase(p);
	_passengersIds.erase(p.id());
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
				env->getArrPassengerById(idPassenger).setWaitingTime(nowTime - p.time());
			}
			else if (idPassenger < 0) {
				idPassenger = -idPassenger;
				Passenger p = env->getWayPassengerById(idPassenger);
				assert(p.to() == _pos);
				delPassenger(p);
				env->delWayPassengerById(idPassenger);
				env->getAllPassengerById(idPassenger).setTotalDuration(nowTime - p.time());
				env->getArrPassengerById(idPassenger).setTotalDuration(nowTime - p.time());
			}

			restTime -= needTime;
		}
		else {
			_pos = firstComm.performPart(_pos, restTime);
			break;
		}
	}
}

string Taxi::toStringToDraw() const {
	stringstream ss;
	ss << _id << ' ' << _pos.getX() << ' ' << _pos.getY() << ' ' << _passengers.size() << ' ';

	int kostyl = 0;
	for (const auto& p : _passengers) {
		if (kostyl != 0) {
			ss << ' ';
		}
		ss << p.id();
		kostyl++;
	}

	int num = 0;
	ss << ' ';
	ss << commands().size();
	for (const auto& command : _commands) {
		ss << ' ';
		ss << command.toStringDraw();
	}

	string res;
	getline(ss, res);
	return res;
}

list<int> Taxi::passengersIds() const {
	list<int> res;
	for (auto id : _passengersIds) {
		res.push_back(id);
	}
	return res;
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
	}
	else {
		resX += dx * haveTime;
	}

	return Point(resX, resY);
}

string Command::toStringDraw() const {
	stringstream ss;
	ss << x << " " << y << " " << a << "\n";
	string res;
	getline(ss, res);
	return res;
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

bool CommandsSequence::isCorrectS() const {
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
		}
		else {
			openedPassengers.erase(id);
		}
	}

	assert(openedPassengers.empty());
	return true;
}

bool CommandsSequence::isCorrectL() const {
	int cnt = 0;
	bool res = true;
	for (auto it = v.rbegin(); it != v.rend(); ++it) {
		Command command = *it;
		int a = command.getA();
		if (a == 0) continue;
		if (a < 0) {
			hasCorrect[-a] = 1;
			cnt++;
		}
		else {
			if (!hasCorrect[a]) {
				res = false;
			}
			hasCorrect[a] = 0;
			cnt--;
		}
	}

	// checking capacity
	for (auto& command : v) {
		int a = command.getA();
		if (a == 0) continue;
		int id = abs(a);
		if (a > 0) {
			hasCorrect[a] = 1;
			cnt++;
			if (cnt > 4) {
				res = false;
			}
		}
		else {
			if (hasCorrect[id]) cnt--;
			hasCorrect[id] = 0;
		}
	}
	for (auto& command : v) {
		int a = command.getA();
		if (a == 0) continue;
		int id = abs(a);
		hasCorrect[id] = 0;
	}
	assert(cnt == 0);
	return res;
}

bool CommandsSequence::isCorrect() const {
	//assert(isCorrectL() == isCorrectS()); {
	//isCorrectL();
	//}
	return isCorrectL();
}

void CommandsSequence::clearZeroCommands() {
	bool found = true;
	while (found) {
		found = false;
		for (auto it = v.begin(); it != v.end(); ++it) {
			Command command = *it;
			if (command.getA() == 0) {
				v.erase(it);
				found = true;
				break;
			}
		}
	}
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
			passengers[id] = env->getArrPassengerById(id);
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

void CommandsSequence::insert(CommandsSequence cs, int ind) {
	auto it = v.begin();
	for (int i = 0; i < ind; ++i, ++it);
	v.insert(it, cs.begin(), cs.end());
}

void CommandsSequence::insert(Command c, int ind) {
	auto it = v.begin();
	for (int i = 0; i < ind; ++i, ++it);
	v.insert(it, c);
}

void CommandsSequence::delPrev(int ind) {
	auto it = v.begin();
	for (int i = 0; i < ind; ++i, ++it);
	v.erase(--it);
}

void CommandsSequence::delPassenger(int id) {
	CommandsList::iterator it_land;
	for (it_land = v.begin(); it_land != v.end(); it_land++) {
		if (it_land->getA() == id) {
			break;
		}
	}
	assert(it_land != v.end());
	v.erase(it_land);

	CommandsList::iterator it_debarkation;
	for (it_debarkation = v.begin(); it_debarkation != v.end(); it_debarkation++) {
		if (it_debarkation->getA() == -id) {
			break;
		}
	}
	assert(it_debarkation != v.end());
	v.erase(it_debarkation);
}

void CommandsSequence::pickUpPassenger(const Passenger &p) {
	Command pLand(p.from(), p.id());
	Command pDebark(p.from(), p.id());
	v.push_front(pDebark);
	v.push_front(pLand);
}

// SolutionEnvironment ==================================================================================

pair<double, CommandsSequence> getBestSequenceBruteforce(CommandsSequence commands, const Taxi& taxi) {
	//return sol->getBestPermutation(commands, taxi);


	int fact = 1;
	for (int i = 2; i < commands.size(); i++) {
		fact *= i;
	}
	CommandsSequence bestSequence = commands;
	double bestScore = bestSequence.estimateScore(taxi);
	for (int i = 0; i < fact; i++) {
		commands.nextPermutation();
		if (commands.back().getA() > 0) continue;
		if (!commands.isCorrect()) continue;
		double curScore = commands.estimateScore(taxi);
		if (curScore > bestScore) {
			bestScore = curScore;
			bestSequence = commands;
		}
	}
	return make_pair(bestScore, bestSequence);
};

pair<double, CommandsSequence> getBestSequenceInsertLast(CommandsSequence commands, const Taxi& taxi) {
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
	Command drop = betterSequence.takeLast();
	Command take = betterSequence.takeLast();
	CommandsSequence adding;
	adding.addCommand(take);
	adding.addCommand(drop);
	CommandsSequence resSequence;
	double mxScore = -1e9;
	for (int i = 0; i <= betterSequence.size(); i++) {
		CommandsSequence curSequqnce = betterSequence;
		curSequqnce.insert(take, i);
		for (int j = i + 1; j <= curSequqnce.size(); ++j) {
			curSequqnce.insert(drop, j);
			if (curSequqnce.isCorrect()) {
				double curScore = curSequqnce.estimateScore(taxi);
				if (curScore > mxScore) {
					mxScore = curScore;
					resSequence = curSequqnce;
				}
			}
			curSequqnce.delPrev(j + 1);
		}
	}
	if (mxScore < -100) {
		int i = betterSequence.size();
		CommandsSequence curSequqnce = betterSequence;
		curSequqnce.insert(adding, i);
		if (curSequqnce.isCorrect()) {
			double curScore = curSequqnce.estimateScore(taxi);
			if (curScore > mxScore) {
				mxScore = curScore;
				resSequence = curSequqnce;
			}
		}
	}
	assert(mxScore > -100);
	return make_pair(mxScore, resSequence);
};

bool isAvailableCommand(const list<int> &takedPs, const list<int> &deliveredPs, const Command &command) {
	int aPass = command.getA();
	int idPass = abs(aPass);
	if (aPass < 0) {
		return find(takedPs.begin(), takedPs.end(), idPass) != takedPs.end();
	}
	if (find(takedPs.begin(), takedPs.end(), idPass) != takedPs.end()) {
		return false;
	}
	return find(deliveredPs.begin(), deliveredPs.end(), idPass) == takedPs.end();
}

void addCommandToList(list<int> &takedPs, list<int> &deliveredPs, CommandsSequence &commands, const Command &command) {
	int aPass = command.getA();
	int idPass = abs(aPass);
	if (aPass < 0) {
		takedPs.remove(idPass);
		deliveredPs.push_back(idPass);
	} else {
		takedPs.push_back(idPass);
		assert(find(deliveredPs.begin(), deliveredPs.end(), idPass) == deliveredPs.end());
	}
	commands.addCommand(command);
}

pair<double, CommandsSequence> getBestSequenceMinDist(CommandsSequence commands, const Taxi& taxi) {
	CommandsSequence resSequence;
	double mxScore = -1e9;
	for (Command firstCommand : commands) {
		list<int> takedPs = taxi.passengersIds();
		list<int> deliveredPs;
		int aFirst = firstCommand.getA();
		if (aFirst < 0 && find(takedPs.begin(), takedPs.end(), -aFirst) == takedPs.end()) {
			continue;
		}
		if (aFirst > 0) {
			takedPs.push_back(aFirst);
		} else {
			takedPs.remove(aFirst);
			deliveredPs.push_back(aFirst);
		}
		CommandsSequence curCommands;
		curCommands.addCommand(firstCommand);
		while (curCommands.size() < commands.size()) {
			Command addBestCommand;
			double distBestCommand = DOUBLE_INF;
			for (auto command : commands) {
				if (isAvailableCommand(takedPs, deliveredPs, command)) {
					double comDist = getDistance(curCommands.back(), command.getPoint());
					if (comDist < distBestCommand) {
						distBestCommand = comDist;
						addBestCommand = command;
					}
				}
			}
			addCommandToList(takedPs, deliveredPs, curCommands, addBestCommand);
		}
		double curScore = curCommands.estimateScore(taxi);
		if (curScore > mxScore) {
			mxScore = curScore;
			resSequence = curCommands;
		}
	}
	assert(mxScore > -100);
	return make_pair(mxScore, resSequence);
};

void SolutionEnvironment::optimizeCommandsOrder(CommandsSequence& commands, const Taxi& taxi) const {
	if (!env->checkToDoOptimizations()) {
		return;
	}
	if (commands.size() < FULL_REORDER_LIMIT) {
		auto best = getBestSequenceBruteforce(commands, taxi);
		commands = best.second;
	} else {
//		auto best = getBestSequenceInsertLast(commands, taxi);
//		commands = best.second;
		//auto best1 = getBestSequenceMinDist(commands, taxi);
		auto best2 = getBestSequenceInsertLast(commands, taxi);
		//if (best1.first > best2.first) {
			//commands = best1.second;
		//} else {
			commands = best2.second;
	//	}
	}
}

set<Point> SolutionEnvironment::generatePointsForTaxisByPassengers(const vector<Passenger> &psngrs, int n) const {
	set<Point> points;
	for (auto p : psngrs) {
		points.insert(p.from());
	}
	while (points.size() < n) {
		points.insert(Point(rand() % env->width(), rand() % env->height()));
	}
	return points;
}

set<Point> SolutionEnvironment::generatePointsForTaxisByTaxis(const vector<Taxi> &taxis, int num) const {
	set<Point> result;

	vector<Point> candidats;
	int bw = env->width() * 0.1;
	int bh = env->height() * 0.1;
	for (int i = 0; i < 500; ++i) {
		candidats.emplace_back(rand() % (env->width() - 2*bw) + bw, rand() % (env->height() - 2*bh) + bh);
	}

	double taxiW = 0.5;
	double pointW = 1.0;
	while (result.size() < num) {
		double bestTotalDist = 0;
		int bestPointId = 0;
		for (int i = 0; i < candidats.size(); ++i) {
			auto curPoint = candidats[i];
			double totalDist = 0;
			
			int nearestTaxi = 0;
			double nearestTaxiDst = 1e18;
			for (const auto& taxi : taxis) {
				double d = getDistance(taxi.pos(), curPoint);
				//totalDist += taxiW / (d * d);
				if (d < nearestTaxiDst) {
					nearestTaxi = taxi.id();
					nearestTaxiDst = d;
				}
			}
			totalDist = max(totalDist, nearestTaxiDst * taxiW);
			Point nearestPoint;
			double nearestPointDist = 1e18;
			for (const auto& p : result) {
				double d = getDistance(p, curPoint);
				//totalDist += pointW / (d * d);
				if (d < nearestPointDist) {
					nearestPointDist = d;
					nearestPoint = p;
				}
			}
			totalDist = max(totalDist, nearestPointDist * pointW);
			if (totalDist > bestTotalDist) {
				bestTotalDist = totalDist;
				bestPointId = i;
			}
		}
		result.insert(candidats[bestPointId]);
		candidats[bestPointId] = candidats.back();
		candidats.pop_back();
	}

	return result;
}

void SolutionEnvironment::distributeFreeTaxis(const vector<Passenger> &psngrs, map<int, CommandsSequence>& c) {
	set<Taxi> freeTaxis;
	for (auto& taxi : env->getTaxis()) {
		CommandsSequence commands = taxi.commands();
		commands.clearZeroCommands();
		if (commands.size()) continue;
		//if (!taxi.isAtBorder()) continue;
		freeTaxis.insert(taxi);
	}
	//set<Point> points = generatePointsForTaxisByPassengers(psngrs, (int)freeTaxis.size());
	set<Point> points = generatePointsForTaxisByTaxis(env->getTaxis(), (int)freeTaxis.size());
	while (!freeTaxis.empty()) {
		Point p;
		Taxi t;
		ll bestDist = DOUBLE_INF;
		for (auto taxi : freeTaxis) {
			for (auto point : points) {
				ll d = getDistance(taxi.pos(), point);
				if (d < bestDist) {
					bestDist = d;
					t = taxi;
					p = point;
				}
			}
		}
		c[t.id()].addCommand(Command(p, 0));
		points.erase(p);
		freeTaxis.erase(t);
	}
	//assert(points.empty());
}

vector<Passenger> SolutionEnvironment::getVectorWaitingPassengers() const {
	vector<Passenger> res;
	for (auto el : _waitingPassengers) {
		res.push_back(el.second);
	}
	return res;
}

pair<double, CommandsSequence> SolutionEnvironment::getBestPermutation(CommandsSequence cs, const Taxi& taxi) {
	_pbestScore = -1e9;
	_pcnt = 0;
	for (auto it = cs.rbegin(); it != cs.rend(); ++it) {
		Command command = *it;
		int a = command.getA();
		assert(a != 0);
		if (a < 0) {
			hasCorrect[-a] = 1;
			_pcnt++;
		}
		else {
			hasCorrect[a] = 0;
			_pcnt--;
		}
	}
	CommandsSequence cur;
	generateValidPermutations(cur, 0, cs, taxi);

	for (auto it = cs.rbegin(); it != cs.rend(); ++it) {
		Command command = *it;
		int a = command.getA();
		hasCorrect[abs(a)] = 0;
	}

	return { _pbestScore, _pbestSequence };
}

void SolutionEnvironment::generateValidPermutations(CommandsSequence& cur, int mask, CommandsSequence& initial, const Taxi& taxi) {
	if (mask == (1 << initial.size()) - 1) {
		double curScore = cur.estimateScore(taxi);
		if (curScore > _pbestScore) {
			_pbestScore = curScore;
			_pbestSequence = cur;
		}
		return;
	}
	int i = 0;
	for (auto it = initial.begin(); it != initial.end(); ++it, ++i) {
		if (mask >> i & 1) continue;
		Command command = *it;
		int a = command.getA();
		int nxtmask = (1 << i) | mask;
		cur.addCommand(command);
		assert(a != 0);
		int id = abs(a);
		if (a > 0) {
			if (_pcnt < 4) {
				_pcnt++;
				hasCorrect[id] = 1;
				generateValidPermutations(cur, nxtmask, initial, taxi);
				_pcnt--;
				hasCorrect[id] = 0;
			}
		}
		else {
			if (hasCorrect[id]) {
				_pcnt--;
				hasCorrect[id] = 0;
				generateValidPermutations(cur, nxtmask, initial, taxi);
				_pcnt++;
				hasCorrect[id] = 1;
			}
		}
		cur.takeLast();
	}
}
