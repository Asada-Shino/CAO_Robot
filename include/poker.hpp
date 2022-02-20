/*** 
 * @Author       : KnightZJ
 * @LastEditTime : 2022-02-10 14:38:22
 * @LastEditors  : KnightZJ
 * @Description  : poker header file
 */

#ifndef _POKER_H_
#define _POKER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef uint64_t bits64;
typedef uint64_t cards;
/*
64bits, 

*/

typedef enum CardType {
    _3, _4, _5, _6, _7, _8, _9, _10, _J, _Q, _K, _A, _2, _BlackJoker, _RedJoker
}CardType;

typedef enum CardGroupType {
    Illegal,
    Single,
    Double,
    Three,
    Bomb,
    KingBomb,
    ThreeWithSingle,
    ThreeWithDouble,
    FourWithSingles,
    FourWithDoubles,
    Sequence,
    DoubleSequence,
    Airplane,
    AirplaneWithSingles,
    AirplaneWithDoubles
}CardGroupType;

extern const char* strCardGroupType[15];

// enum CardColor {
    
// }
/*** 
 * @description: count the num of 1s of given bits
 * @param      {*}
 * @return     {*}
 */
int count(bits64 bits);

int count_cards(cards cs);

cards ones(cards cs);
cards twos(cards cs);
cards threes(cards cs);
cards fours(cards cs);
cards check_sequence(cards cs, int length);

cards can_take(cards cs, CardType ct);

int can_add(cards cs, CardType ct);

int take(cards* cs, CardType ct);
int add(cards* cs, CardType ct);

int add_cards(cards* dest, cards* source);

void shuffle(cards cs[3], cards *landlord);

CardGroupType type(cards cs);
int judge(cards last, cards hand);

#endif