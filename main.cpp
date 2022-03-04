//#define DEBUGPRINT
#ifdef DEBUGPRINT
#include <stdio.h>
#endif

#include <cstdint>
#include "cmvec.h"
#include <list>
#include <cstdlib>
#include <ctime>
#include <SFML/Graphics.hpp>

const uint8_t S = 8;
const ivec2 dirs[4] = {{0,  -1},
		{0,  1},
		{1,  0},
		{-1, 0}};
const ivec2 boardSize = {S, S};
struct action {
	ivec2 p;
	uint8_t bef, aft;
#ifdef DEBUGPRINT
	void print() const {
        printf("(%d,%d): %x->%x\n", p.x, p.y, bef, aft);
    }
#endif
};

struct step {
	std::list<action> acts;

	step() {
		acts = {};
	}

	void add(ivec2 p, uint8_t bef, uint8_t aft) {
		acts.push_back({p, bef, aft});
	}

#ifdef DEBUGPRINT
	void print() {
        for (auto a : acts) {
            a.print();
        }
        printf("===============\n");
    }
#endif
};

class board {
	uint8_t *map;
	bool begin_continue = true;
	//std::set<ivec2> alive;
	//0b12345678 (12: w/b figure)(34: default/queen)(56: end of field)(78: w/b cell)
public:
	std::list<step> story = {};
	bool white_black = true;   //turn into game flags
	ivec2 activeFigure = {-1, -1};

	void set(int16_t x, int16_t y, uint8_t t) {
		if (inZone(ivec2(x, y), boardSize)) map[S * y + x] = t;
	}

	void set(ivec2 p, uint8_t t) {
		if (inZone(p, boardSize)) map[S * p.y + p.x] = t;
	}

	inline uint8_t get(uint8_t x, uint8_t y) const {
		if (inZone(ivec2(x, y), boardSize)) return map[S * y + x];
		return 0;
	}

	inline uint8_t get(ivec2 p) const {
		if (inZone(p, boardSize)) return map[S * p.y + p.x];
		return 0;
	}

	board() {
		map = new uint8_t[S * S];
		for (uint8_t y = 0; y < S; y++) {
			uint8_t t = 0;
			t |= (y > 0 && y < 3) ? 0b10100000 : (y < S - 1 && y > S - 4) ? 0b01100000 : 0;
			t |= (y == 0) ? 0b00000100 : (y == S - 1) ? 0b00001000 : 0;
			for (uint16_t x = 0; x < S; x++) {
				set(x, y, t | ((x + y) % 2 ? 0b00000001 : 0b00000010));
			}
		}
	}
#ifdef DEBUGPRINT
	void print() const {
        printf(" ");
        for (int x = 0; x < boardSize.x; x++) {
            printf(" %d", x);
        }
        printf("\n #");
        for (int x = 1; x < boardSize.x; x++) {
            printf("--");
        }
        printf("-\n");
        ivec2 p;
        for (p.y = boardSize.y - 1; p.y >= 0; p.y--) {
            printf("%d|", p.y);
            for (p.x = 0; p.x < boardSize.x; p.x++) {
                uint8_t t = get(p);
                bool s_b = t & 0b00100000;
                char c = (t & 0b10000000) ? (s_b ? 'w' : 'W') : (t & 0b01000000) ? (s_b ? 'b' : 'B') : ((t & 0b00000010) ? '.' : ',');
                printf("%c ", c);
            }
            putchar('\n');
        }
        putchar('\n');
    }
#endif
	~board() {
		delete[] map;
	}
	bool isMain(uint8_t f) const {
		return f & (white_black ? 0b10000000 : 0b01000000);
	}
	bool isPossibleDir(uint8_t d, uint8_t f) const {
		return d != (white_black ? 0 : 1) || (f & 0b00010000);
	}
	bool isEnemy(uint8_t f) const {
		return f & (white_black ? 0b01000000 : 0b10000000);
	}
	bool isFree(uint8_t f) const {
		return !(f & 0b11000000) && f;
	}
	uint8_t freeRad(ivec2 p0, ivec2 dir, uint8_t R) const {
		uint8_t Rf = 1;
		for (; Rf <= R; Rf++) {
			if (!isFree(get(p0 + dir * Rf))) {
				break;
			}
		}
		return Rf - 1;
	}
	uint8_t radius(uint8_t f) const {
		return (f & 0b00100000) ? 1 : S;
	}

	uint8_t protection(uint8_t f1, uint8_t f2) const {
		return (f1 ? f1 : 0b11000000) & (f2 ? f2 : 0b11000000);
	}
	float value() const {
		float sqw = 0., sqb = 0.;
		float s = 0., ms = 0.;
		ivec2 p;
		for (p.y = 0; p.y < S; p.y++) {
			for (p.x = 0; p.x < S; p.x++) {
				uint8_t f = get(p);
				if (!(f & 0b11000000)) continue;
				float m = (f & 0b10000000) ? 1.f : -1.f;
				float yf = (f & 0b00010000) ? 0. : (f & 0b10000000) ? p.y : (S - 1 - p.y);
				float c = (f & 0b00100000) ? 1. : S / 2.;
				s += m * c * (1. + yf / (S * 5.));
				//ms += 1.;
			}
		}
		for (p.y = 0; p.y <= S; p.y++) {
			for (p.x = 0; p.x <= S; p.x++) {
				uint8_t t = get(p);
				uint8_t tx = protection(get(p.x - 1, p.y), t);
				uint8_t ty = get(p.x, p.y - 1) && t;
				if (tx & 0b10000000)sqw += 1.;
				if (ty & 0b10000000)sqw += 1.;
				if (tx & 0b01000000)sqb += 1.;
				if (ty & 0b01000000)sqb += 1.;
			}
		}
		return s + (sqw - sqb) * .1;
	}
	void moveAct(ivec2 p0, ivec2 p1, step &s) const {
		uint8_t f00 = get(p0), f10 = get(p1);
		uint8_t f01 = f00 & 0b00001111, f11 = (f10 & 0b00001111) | (f00 & 0b11110000);
		s.add(p0, f00, f01);
		s.add(p1, f10, f11);
		const uint8_t wf = 0b10101000, bf = 0b01100100;
		if (white_black ? ((f11 & wf) == wf) : ((f11 & bf) == bf)) {
			uint8_t f12 = (f11 & 0b11001111) | 0b00010000;
			s.add(p1, f11, f12);
		}
	}
	void killAct(ivec2 p, step &s) const {
		uint8_t f0 = get(p), f1 = f0 & 0b00001111;
		s.add(p, f0, f1);
	}
	void moveKillAct(ivec2 p0, ivec2 pe, ivec2 p1, step &s) const {
		moveAct(p0, p1, s);
		killAct(pe, s);
	}
	void playStep(const step &s, bool swap = true) {
		for (auto a : s.acts) {
			set(a.p, a.aft);
		}
		story.push_back(s);
		if (swap) white_black = !white_black;
	}
	void undo(bool swap = true) {
		auto s = --story.end();
		for (auto a = s->acts.rbegin(); a != s->acts.rend(); ++a) {
			set(a->p, a->bef);
		}
		story.pop_back();
		if (swap) white_black = !white_black;
	}
	bool userStep_(ivec2 p0, ivec2 p1, step &s, bool &kill) {
		uint8_t f = get(p0);
		kill = false;
		if (!isMain(f)) {
			return false;
		}
		uint8_t R = radius(f);
		ivec2 dp = p1 - p0;
		for (uint8_t d = 0; d < 4; d++) {
			int16_t Rp = proj(dp, dirs[d]);
			if (Rp <= 0) continue;
			uint8_t Rf = freeRad(p0, dirs[d], R);
			if (Rp <= R && Rp <= Rf && !kill && isPossibleDir(d, f)) {
				moveAct(p0, p1, s);
				return true;
			} else if (Rf == Rp - 2 && Rp <= R + 1) {
				ivec2 pe = p1 - dirs[d];
				if (isEnemy(get(pe)) && isFree(get(p1))) {
					moveKillAct(p0, pe, p1, s);
					return kill = true;
				}
				break;
			}
		}
		return false;
	}
	void userStep(ivec2 p) {
		if (p == activeFigure) {
			activeFigure = {-1, -1};
			if (!begin_continue) {
				begin_continue = true;
				white_black = !white_black;
			}
			return;
		}
		bool kill = !begin_continue;
		step s_;
		step &s = begin_continue ? s_ : *(--story.end());
		if (userStep_(activeFigure, p, s, kill)) {
			if (begin_continue) {
				story.push_back(s);
				playStep(s, !kill);
				activeFigure = kill ? p : ivec2(-1, -1);
				begin_continue = !kill;
			} else {
				if (kill) {
					playStep(s, false);
					activeFigure = p;
				}
			}
		} else {
			if (begin_continue) {
				activeFigure = isMain(get(p)) ? p : ivec2(-1, -1);
			}
		}
	}

	void getStepList(std::list<step> &sl, step buf = {}) {
		if (begin_continue) {
			ivec2 p0;
			for (p0.y = 0; p0.y < S; p0.y++) {
				for (p0.x = 0; p0.x < S; p0.x++) {
					uint8_t f = get(p0);
					if (!isMain(f)) continue;
					uint8_t R = radius(f);
					for (uint8_t d = 0; d < 4; d++) {
						uint8_t Rf = freeRad(p0, dirs[d], R);
						if (Rf < R) {
							ivec2 p_ = p0 + dirs[d] * (Rf + 2), pe = p0 + dirs[d] * (Rf + 1);
							if (isEnemy(get(pe)) && isFree(get(p_))) {
								step s;
								moveKillAct(p0, pe, p_, s);
								sl.push_back(s);
								begin_continue = false;
								activeFigure = p_;
								playStep(s, false);
								getStepList(sl, s);
								undo(false);
								begin_continue = true;
								activeFigure = {-1, -1};
							}
						}
						if (isPossibleDir(d, f)) {
							for (uint8_t r = 1; r <= Rf && r <= R; r++) {
								step s;
								moveAct(p0, p0 + dirs[d] * r, s);
								sl.push_back(s);
							}
						}
					}
				}
			}
		} else {
			ivec2 p0 = activeFigure;
			uint8_t R = radius(get(p0));
			for (uint8_t d = 0; d < 4; d++) {
				uint8_t Rf = freeRad(p0, dirs[d], R);
				if (Rf < R) {
					ivec2 p_ = p0 + dirs[d] * (Rf + 2), pe = p0 + dirs[d] * (Rf + 1);
					uint8_t pef = get(pe), p_f = get(p_);
					if (isEnemy(pef) && isFree(p_f)) {
						step s, buf_ = buf;
						moveKillAct(p0, pe, p_, s);
						moveKillAct(p0, pe, p_, buf_);
						activeFigure = p_;
						sl.push_back(buf_);
						playStep(s, false);
						getStepList(sl, buf_);
						undo(false);
					}
				}
			}
		}
	}

	bool isTerminal() const {
		if (begin_continue) {
			ivec2 p0;
			for (p0.y = 0; p0.y < S; p0.y++) {
				for (p0.x = 0; p0.x < S; p0.x++) {
					uint8_t f = get(p0);
					if (!isMain(f)) continue;
					for (uint8_t d = 0; d < 4; d++) {
						if (isPossibleDir(d, f) && isFree(get(p0 + dirs[d]))) return false;
					}
					for (uint8_t d = 0; d < 4; d++) {
						if (isEnemy(get(p0 + dirs[d])) && isFree(get(p0 + dirs[d] * 2))) {
							return false;
						}
					}
				}
			}
		} else {
			return false;
		}
		return true;
	}
};

float minimax(board &state, uint8_t depth, float alpha, float beta, bool maximizingPlayer) {
	if (depth == 0 || state.isTerminal()) {
		return state.value();
	}
	std::list<step> sl;
	state.getStepList(sl);
	if (maximizingPlayer) {
		float value = -100000.;
		for (const auto &s : sl) {
			state.playStep(s);
			value = std::max(value, minimax(state, depth - 1, alpha, beta, false));
			state.undo();
			alpha = std::max(alpha, value);
			if (value >= beta) break;
		}
		return value;
	} else {
		float value = 100000.;
		for (const auto &s : sl) {
			state.playStep(s);
			value = std::min(value, minimax(state, depth - 1, alpha, beta, true));
			state.undo();
			beta = std::min(beta, value);
			if (value <= alpha) break;
		}
		return value;
	}
}

step optimalstep(board &state, uint8_t depth, bool maximizingPlayer) {
	std::list<step> sl;
	state.getStepList(sl);
	step os = {};
	float m = maximizingPlayer ? -1 : 1;
	float val = -100000.;
	for (const auto &s : sl) {
		state.playStep(s);
		float val_ = -m * minimax(state, depth, -100000., 100000., !maximizingPlayer);
		if (val_ > val) {
			val = val_;
			os = s;
		} else if (val_ == val && rand() % 2) {
			os = s;
		}
		state.undo();
	}
	return os;
}

class boardUI {
	sf::IntRect zone;
	sf::RenderWindow *win;
public:
	board *b;

	boardUI(sf::RenderWindow &win_, sf::IntRect zone_) {
		b = new board();
		zone = zone_;
		win = &win_;
	}

	boardUI() {
		delete b;
	}

	void click() {
		sf::Vector2i mpos = sf::Mouse::getPosition(*win);
		ivec2 p((S * (mpos.x - zone.left)) / zone.width, S - 1 - (S * (mpos.y - zone.top)) / zone.height);
		if (inZone(p, boardSize)) {
			b->userStep(p);
		}
	}

	void printPart(ivec2 p0, const std::string &path) {
		p0.y = 7 - p0.y;
		sf::Texture tex;
		tex.loadFromFile(path);
		sf::Sprite spr;
		spr.setTexture(tex);
		sf::Vector2f bs((float) zone.width / S, (float) zone.height / S);
		sf::Vector2u ts = tex.getSize();
		spr.setScale(bs.x / ts.x, bs.y / ts.y);
		spr.setPosition(zone.left + p0.x * bs.x, zone.top + p0.y * bs.y);
		win->draw(spr);
	}

	void printFull() {
		ivec2 p;
		for (p.y = 0; p.y < 8; p.y++) {
			for (p.x = 0; p.x < 8; p.x++) {
				uint8_t f = b->get(p);
				if (f & 0b00000010) printPart(p, "cellWhite.png");
				if (f & 0b00000001) printPart(p, "cellBlack.png");
				if (f & 0b00000100) printPart(p, "endRed.png");
				if (f & 0b00001000) printPart(p, "endBlue.png");
				if (f & 0b10000000) printPart(p, "baseRed.png");
				if (f & 0b01000000) printPart(p, "baseBlue.png");
				if (f & 0b00010000) printPart(p, "crown.png");
				if (p == b->activeFigure) printPart(p, "select.png");
			}
		}
	}
};

step stepById(std::list<step> &sl, uint8_t id) {
	auto s = sl.begin();
	for (uint8_t i = 0; i < id; i++) {
		++s;
	}
	return *s;
}

void frap(sf::RenderWindow &mainWindow, boardUI &mainBoard) {
	mainWindow.clear();
	mainBoard.printFull();
	mainWindow.display();
}

int main() {
	uint8_t seed = (std::time(0)) % 255;
	srand(seed);
	const uint16_t winW = 800, winH = 800;

	sf::RenderWindow mainWindow(sf::VideoMode(winW, winH), "The Cyan #1");

	boardUI mainBoard(mainWindow, sf::IntRect(0, 0, 800, 800));
	board &b_ = *(mainBoard.b);
	frap(mainWindow, mainBoard);
	while (mainWindow.isOpen()) {
		sf::Event event;
		while (mainWindow.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				mainWindow.close();
#ifdef DEBUGPRINT
				printf("game length: %llu", b_.story.size());
#endif
			} else if (b_.white_black && event.type == sf::Event::MouseButtonPressed) {
				mainBoard.click();
				frap(mainWindow, mainBoard);
			}
		}
		if (!(b_.white_black) && !(b_.isTerminal())) {
			step s = optimalstep(b_, 5, false);
			b_.playStep(s);
			frap(mainWindow, mainBoard);
			sf::sleep(sf::milliseconds(400));
			b_.undo();
			frap(mainWindow, mainBoard);
			sf::sleep(sf::milliseconds(400));
			b_.playStep(s);
			frap(mainWindow, mainBoard);
		}
	}
	return 0;
}