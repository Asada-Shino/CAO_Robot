/*
 * @Author       : KnightZJ
 * @LastEditTime : 2022-02-10 12:01:51
 * @LastEditors  : KnightZJ
 * @Description  : poker source file
 */
#include "poker.hpp"
#include <iostream>

const bits64 FOUR_MASK  = 0x1fff000000000000ULL;
const bits64 THREE_MASK = 0x00001fff00000000ULL;
const bits64 TWO_MASK   = 0x000000001fff0000ULL;
const bits64 ONE_MASK   = 0x0000000000007fffULL;

const cards RedJoker    = 0x4000ULL;
const cards BlackJoker  = 0x2000ULL; 
const cards FullCards   = 0x1fff000000006000ULL;


const char* strCardGroupType[15] = {
    "非法牌型",
    "单张",
    "对子",
    "三张",
    "炸弹",
    "王炸",
    "三带一",
    "三带一对",
    "四带二",
    "四带两对",
    "顺子",
    "连对",
    "飞机",
    "飞机带单",
    "飞机带对"
};




cards ones(cards cs) { return cs & ONE_MASK; }
cards twos(cards cs) { return (cs & TWO_MASK) >> 16; }
cards threes(cards cs) { return (cs & THREE_MASK) >> 32; }
cards fours(cards cs) { return (cs & FOUR_MASK) >> 48; }

int count(bits64 bits) {
    int cnt = 0;
    while(bits != 0) {
        ++cnt;
        bits &= (bits - 1);
    }
    return cnt;
}

int count_cards(cards cs) {
    cards one = ones(cs), two = twos(cs), three = threes(cs), four = fours(cs);
    return count(one) + count(two)*2 + count(three)*3 + count(four)*4;
}

cards check_sequence(cards cs, int length) {
    cards test = (1<<length)-1;
    while(test <= 0xfffULL) {
        if((test & cs) == test)
            return test;
        test <<= 1;
    }
    return 0;
}

CardGroupType type(cards cs) {
    cards one = ones(cs), two = twos(cs), three = threes(cs), four = fours(cs);
    int singles = count(one), doubles = count(two);
    int cnt = singles + doubles*2 + count(three)*3 + count(four)*4;
    if(cnt >= 5 && cnt <= 13 && check_sequence(one, cnt))
        return Sequence;
    if(cnt % 2 == 0 && cnt >= 6 && cnt <= 20 && check_sequence(two, cnt >> 1))
        return DoubleSequence;
    if(cnt % 3 == 0 && cnt >= 6 && cnt <= 18 && check_sequence(three, cnt / 3))
        return Airplane;
    switch(cnt) {
        case 1:
            return Single;
        case 2:
            if(two) return Double;
            if((cs & RedJoker) && (cs & BlackJoker)) return KingBomb;
            break;
        case 3:
            if(three) return Three;
            break;
        case 4:
            if(four) return Bomb;
            if(three && one) return ThreeWithSingle;
            break;
        case 5:
            if(three && two) return ThreeWithDouble;
            break;
        case 6:
            if(four && (doubles * 2 + singles == 2)) return FourWithSingles;
            break;
        case 8:
            if(four && (doubles == 2)) return FourWithDoubles;
            if(check_sequence(three, 2) && doubles * 2 + singles == 2) return AirplaneWithSingles;
            break;
        case 10:
            if(check_sequence(three, 2) && doubles == 2) return AirplaneWithDoubles;
            break;
        case 12:
            if(check_sequence(three, 3) && !check_sequence(three, 4) && (count(three) - 3)*3 + doubles * 2 + singles == 3)
                return AirplaneWithSingles;
            break;
        case 15:
            if(check_sequence(three, 3) && doubles == 3) return AirplaneWithDoubles;
            break;
        case 16:
            if(check_sequence(three, 4) && !check_sequence(three, 5) && (count(three) - 3)*3 + doubles * 2 + singles == 4)
                return AirplaneWithSingles;
            break;
        case 20:
            if(check_sequence(three, 4) && doubles == 4) return AirplaneWithDoubles;
            if(check_sequence(three, 5) && !check_sequence(three, 6) && (count(three) - 3)*3 + doubles * 2 + singles == 5)
                return AirplaneWithSingles;
            break;
    }
    return Illegal;
}

int judge(cards last, cards hand) {
    CardGroupType last_type = type(last), hand_type = type(hand);
    if(last_type == KingBomb)
        return 0;
    if(hand_type == KingBomb)
        return 1;
    if(hand_type == Bomb)
        return (last_type == Bomb && last > hand) ? 0 : 1;
    if(hand_type == Illegal || hand_type != last_type || count(last) != count(hand))
        return -1;
    switch(last_type) {
        case Single:
        case Double:
        case Three:
        case Sequence:
        case DoubleSequence:
        case Airplane:
            return hand > last;
        case ThreeWithSingle:
        case ThreeWithDouble:
        case AirplaneWithDoubles:
            return threes(hand) > threes(last);
        case FourWithSingles:
        case FourWithDoubles:
            return fours(hand) > fours(last);
        case AirplaneWithSingles:
            for(int i = 5; i >= 2; --i) {
                cards last_res = check_sequence(last, i);
                cards hand_res = check_sequence(hand, i);
                if(last_res != 0 || hand_res != 0)
                    return hand_res > last_res;
            }
    }
    return -2;
}

cards can_take(cards cs, CardType ct) {
    if(ct >= _BlackJoker)
        return cs & (1ULL << ct);
    return cs & (0x1000100010001ULL << ct);
}

int can_add(cards cs, CardType ct) {
    if(ct >= _BlackJoker)
        return !(cs & (1ULL << ct));
    return !(cs & (0x1000000000000ULL << ct));
}


int take(cards* cs, CardType ct) {
    cards mask = 0x1000100010001ULL << ct;
    *cs = (~mask & *cs) | ((*cs & mask) >> 16);
    return 1;
}


int add(cards* cs, CardType ct) {
    cards mask = 0x1000100010001ULL << ct;
    if((mask & *cs) == 0)
        *cs |= 1ULL << ct;
    else
        *cs = (~mask & *cs) | ((mask & *cs) << 16);
    return 1;
}

void shuffle(cards cds[3], cards *landlord) {
    srand(time(0));
    int a[54];
    for(int i = 0; i < 54; ++i)
        a[i] = i;
    for(int i = 0; i < 54; ++i) {
        int random = i + rand()%(54-i);
        int num = a[i];
        a[i] = a[random];
        a[random] = num;
    }
    for(int i = 0; i < 51; ++i)
        add(&cds[i/17], (CardType)(a[i] >= 52 ? a[i]-52+13 : a[i]/4));
    for(int i = 51; i < 54; ++i)
        add(landlord, (CardType)(a[i] >= 52 ? a[i]-52+13 : a[i]/4));
}

int add_cards(cards* dest, cards* source) {
    while(*source != 0ULL) {
        for(int i = 0; i < 15; ++i) {
            if(can_take(*source, (CardType)i)) {
                take(source, (CardType)i);
                add(dest, (CardType)i);
            }
        }
    }
    return 0;
}