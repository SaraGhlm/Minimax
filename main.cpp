#define ROADLENGTH 30
#define GOALSCORE  120
#define GOALDELTA  10

int objPool[1000];

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <climits>
#include <string>
#include <climits>
#include <limits>
#include <vector>
#include <memory>
#include <algorithm>
#include <fstream>

using namespace std;


class obj
{
public:
    char sym;	    // symbol for the object
    int v;	    // value of object
    int b;	    // if a player picks the object, the player take b miles away the roadside
public:
    obj():sym(' '),v(0),b(0) {}
    obj(char SYM, int V, int B):sym(SYM),v(V),b(B) {}
    static const obj fourElem[4];
    static const obj damaged;
};
const obj obj::fourElem[4]= {obj('&',2,1),obj('#',5,2),obj('$',7,3),obj('@',10,4)};
const obj obj::damaged = obj('~',0,1);

class playerState
{
public:
    int m;	    // at mile m
    int b;	    // b miles away the roadside
    int v;	    // earned value
public:
    void init(int grant)
    {
        m=0;
        b=0;
        v=grant;
    }
};

class groundState
{
public:
    static const int M=ROADLENGTH;
    obj o[M];
    int owner[M];	    // who player owns the object o[i]
public:
    groundState()
    {
        int i;
        for(i=0; i<M; i++) owner[i]=0;
    }
    void init_rand()
    {
        int i;
        for(i=0; i<M; i++)
            o[i]=obj::fourElem[objPool[rand()%100]];
    }
};

class state
{
public:
    groundState g;
    playerState p[2];
    int turn;
public:
    void print()
    {
        int i,j;
        for(j=4; j>=0; j--)
        {
            bool any=false;
            for(i=0; i<4; i++)
            {
                if(obj::fourElem[i].b==j)
                {
                    cout<< setw(2)<<obj::fourElem[i].v<<' ';
                    any=true;
                }
            }
            if(!any){
                cout << "   ";
            }
            for(i=0; i<groundState::M; i++)
            {
                cout << ' ';
                if(p[0].m==i && p[0].b==j){
                    cout<<(turn==0 ? 'X':'x');
                }
                else if(g.owner[i]==1 && g.o[i].b==j){
                    cout<<g.o[i].sym;
                }
                else{
                    cout<< ' ';
                }
            }
            if(j==0){
                cout << "  ["<<p[0].v<<"]";
            }
            cout << endl;
        }
        cout << "   ";
        for(i=0; i<groundState::M; i++)
            if(g.owner[i]==0){
                cout << ' ' << g.o[i].sym;
            }
            else{
                cout << "  ";
            }
        cout << endl;
        for(j=0; j<=4; j++)
        {
            bool any=false;
            for(i=0; i<4; i++)
            {
                if(obj::fourElem[i].b==j)
                {
                    cout<< setw(2) << obj::fourElem[i].v<<' ';
                    any=true;
                }
            }
            if(!any){
                cout << "   ";
            }
            for(i=0; i<groundState::M; i++)
            {
                cout << ' ';
                if(p[1].m==i && p[1].b==j){
                    cout<< (turn==1 ? 'Z':'z');
                }
                else if(g.owner[i]==2 && g.o[i].b==j){
                    cout<< g.o[i].sym;
                }
                else{
                    cout<< ' ';
                }
            }
            if(j==0){
                cout << "  [" << p[1].v << "]";
            }
            cout << endl;
        }
        cout<<endl;
    }
};

enum action { Nop=0x0000, Pick= 0x0001, Damage= 0x0002, Leave= 0x0004 };

void printAction( action a )
{
    if(a==Pick){
        cout << "Pick ";
    } else if(a==Leave){
        cout << "Leave ";
    } else if(a==Damage){
        cout << "Damage ";
    } else{
        cout << "Nop ";
    }
}

class ActionSet
{
public:
    int A;
public:
    ActionSet()
    {
        A=Nop;
    }
    ActionSet( int _A )
    {
        A=_A;
    }
    bool isEmpty()
    {
        return (A==0);
    }
    bool include(action a)  const
    {
        return (A&a);
    }
    void add( action a )
    {
        A = (A|a);
    }
    void rem( action a )
    {
        A = (A&(~a));
    }
    action  get_first_a( ) const
    {
        if( A&Pick ) 	return Pick;
        if( A&Damage )	return Damage;
        if( A&Leave )	return Leave;
        return Nop;
    }
    action  get_next_a( action a ) const
    {
        while(a!=Leave)
        {
            a = action(a<<1);
            if(A&a) return a;
        }
        return Nop;
    }
};

class GameDef
{
public:
    static const int v_goal;
    static const int d_goal;
    static const int m_goal;

    static state initialState(int firstPlayer, int grantP1, int grantP2);
    static int value(const state &s);
    static ActionSet legalMoves(const state &s);
    static void transit(state &s, action a);
    static bool goalTest( const state &s );
};

state GameDef::initialState(int firstPlayer=1, int grantP1=0, int grantP2=0)
{
    state start;
    start.turn = firstPlayer;
    start.g = groundState();
    start.g.init_rand();
    start.p[0].init(grantP1);
    start.p[1].init(grantP2);
    return start;
}

int GameDef::value( const state &s )
{
    // zero-sum value from the player 0's view
    int v = s.p[0].v-s.p[1].v;
    if(goalTest(s))
    {
        if(v>0)
            v=v+1000000;
        else
            v=v-1000000;
    }
    return v;
}

bool GameDef::goalTest( const state &s )
{
    return ( (s.p[0].m>=m_goal && s.p[1].m>=m_goal)
             || (s.p[0].v>=v_goal && s.p[0].v-s.p[1].v>d_goal)
             || (s.p[1].v>=v_goal && s.p[1].v-s.p[0].v>d_goal) );
}

ActionSet GameDef::legalMoves( const state &s )
{
    ActionSet A(Nop);
    int m=s.p[s.turn].m;
    if(m<m_goal)
    {
        if(s.g.o[m].v==0 || s.g.owner[m]!=0)
            A=ActionSet(Leave);
        else
            A=ActionSet(Pick|Damage|Leave);
    }

    if(s.p[s.turn].b>0) A.rem(Pick);

    // Damage is legal but unreasonable when the opponent is far away roadside.
    int opp=1-s.turn;
    if(s.p[opp].b>m-s.p[opp].m) A.rem(Damage);
    return A;
}

void GameDef::transit( state &s, action a )
{
    // Note: it would be more convenient to implement the stateTransition by this way where
    //           we manipulate the current state in-place. So, you may make a copy of the
    //           current state before call this function.
    int opp=1-s.turn;
    int m=s.p[s.turn].m;
    if( a==Pick )
    {
        s.g.owner[m]=s.turn+1;
        s.p[s.turn].v+=s.g.o[m].v;
        s.p[s.turn].m++;
        s.p[s.turn].b=s.g.o[m].b;
        if(s.p[opp].m<=m)
        {
            s.p[opp].m++;
            if(s.p[opp].b>0) s.p[opp].b--;
        }
    }
    else if( a==Leave )
    {
        s.p[s.turn].m++;
        if(s.p[s.turn].b>0) s.p[s.turn].b--;
    }
    else if( a==Damage )
    {
        s.g.o[m]=obj::damaged;
        s.p[s.turn].m++;
        if(s.p[opp].m<=m)
        {
            s.p[opp].m++;
            if(s.p[opp].b>0) s.p[opp].b--;
        }
    }
    s.turn=opp;
}

const int GameDef::v_goal=GOALSCORE;
const int GameDef::d_goal=GOALDELTA;
const int GameDef::m_goal=groundState::M;


class agent
{
public:
    virtual void init(const state &start) = 0;
    virtual action acton(const state &s, const ActionSet &legalMoves) = 0;
    // Agent is asked by the game engine to choose an action on state s from legalMoves;
    // Note that by this kind of interaction, the agent is **passive** about the perception since that it is assumed that the game engine tell
    // the whole state of the world to the agent once asks for an action.
    virtual string badge() = 0; // each agent should have a badge to introduce itself :)
};

class humanAgent: public agent
{
    string name;
public:
    void init(const state &start)
    {
        if(name=="")
        {
            cout << "Please, enter your name: ";
            cin >> name;
            cout << endl << endl << endl;
        }
    }
    string badge()
    {
        return name;
    }
    action acton(const state &s, const ActionSet &legalMoves)
    {
        action a;
        char c;
        cin >> c;
        if(c=='p' || c=='P')
            a=Pick;
        else if(c=='l' || c=='L')
            a=Leave;
        else if(c=='d' || c=='D')
            a=Damage;
        else
            a=Nop;
        return a;
    }
};

class noleaveGreedy: public agent
{
    string name;
public:
    void init(const state &start)
    {
        static int cloneNum=0;
        if(name=="")
        {
            cloneNum++;
            name="noLeaveGreedy";
            name.push_back('0'+cloneNum);
        }
        // Nothing for brain ... !
    }
    string badge()
    {
        return name;
    }
    action acton(const state &s, const ActionSet &legalMoves)
    {
        // As an example, here, a very simple computer agent for this game is implemented
        //
        // This agent always prefers to Pick, if it is possible, then Damage, and unwillingly, choose Leave if it does not have any choice.
        action a;
        if(legalMoves.include(Pick))
            a=Pick;
        else if(legalMoves.include(Damage))
            a=Damage;
        else if(legalMoves.include(Leave))
            a=Leave;
        else
            a=Nop;
        printAction(a);
        cout << endl;
        return a;
    }
};

class yAgent: public agent
{
    string name;
    int my_turn;
public:
    void init(const state &start)
    {
        static int cloneNum=0;
        if(name=="")
        {
            cloneNum++;
            name="Your AI Agent";
            name.push_back('0'+cloneNum);
        }
    }
    string badge()
    {
        return name;
    }
    action acton(const state &s, const ActionSet &legalMoves)
    {
        my_turn = s.turn;
        state* new_s = new state(s);
        action ac = alpha_beta(*new_s);
        printAction(ac);
        cout << endl;
        return ac;
    }

    action alpha_beta(state &s){
        int alpha = std::numeric_limits<int>::min();
        int beta = std::numeric_limits<int>::max();
        ActionSet action_set = GameDef::legalMoves(s);
        action a = action_set.get_first_a();
        action best_action = a;
        int best_value = alpha;
        do{
            state *new_s = new state(s);
            GameDef::transit(*new_s, a);
            int value = min_value(*new_s, alpha, beta);
            if(value > best_value){
                best_action = a;
                best_value = value;
            }
            a = action_set.get_next_a(a);
            delete new_s;
        } while(a != Nop);
        return best_action;
    }

    int max_value(const state &s, int alpha, int beta)
    {
        if(GameDef::goalTest(s))
        {
            return utility(s);
        }

        int v = std::numeric_limits<int>::max();
        ActionSet action_set = GameDef::legalMoves(s);
        action a = action_set.get_first_a();
        do{
            state *new_s = new state(s);
            GameDef::transit(*new_s, a);
            v = std::max(v, min_value(*new_s, alpha, beta));
            if(v >= beta) {
                return v;
            }
            alpha = std::max(alpha, v);
            delete new_s;
            a = action_set.get_next_a(a);
        } while(a != Nop);
        return v;
    }

    int min_value(const state &s, int alpha, int beta)
    {
        if(GameDef::goalTest(s))
        {
            return utility(s);
        }

        int v = std::numeric_limits<int>::min();
        ActionSet action_set = GameDef::legalMoves(s);
        action a = action_set.get_first_a();
        do{
            state *new_s = new state(s);
            GameDef::transit(*new_s, a);
            v = std::min(v, max_value(*new_s, alpha, beta));
            if(v <= alpha) {
                return v;
            }
            beta = std::min(beta, v);
            delete new_s;
            a = action_set.get_next_a(a);
        } while(a != Nop);
        return v;
    }

    int utility(const state &s)
    {
        return GameDef::value(s);
    }
};


// may not work for windows, req for usleep
#include <unistd.h>

int main(int argc, char *argv[])
{
    // main plays the role of the game engine (the master cycle evolving the world)

    agent *ag1, *ag2;
    ag1=ag2=NULL;

    // The program settings: random seed, relative frequencies of &, #, $, @ to initialize the road
    int F[20]= {123454,60,30,20,15};

    // command line format:
    // cedeside <palyer_1> <player_2> <random_seed> <rel_freq&> <rel_freq#> <rel_freq$> <rel_freq@>
    //   every parameter is optional.
    //   default for
    //         player_1:       Human
    //         player_2:       noLeaveGreedy
    //         random_seed:    123456
    //         rel_freq&:      60
    //         rel_freq#:      30
    //         rel_freq$:      20
    //         rel_freq@:      15
    //
    // parsing command line arguments
    int i;
    int k=0;
    for(i=1; i<argc; i++)
    {
        int f=atoi(argv[i]);
        if(f!=0) F[k++]=f;
        else if(ag2==NULL)
        {
            if(argv[i][0]=='h' || argv[i][0]=='H')
                ag2=new humanAgent;
            else if(argv[i][0]=='n' || argv[i][0]=='N')
                ag2=new noleaveGreedy;
            else if(argv[i][0]=='y' || argv[i][0]=='Y')
                ag2=new yAgent;
            if(ag1==NULL)
            {
                ag1=ag2;
                ag2=NULL;
            }
        }
    }
    srand(F[0]);
    for(i=2; i<=4; i++)
        F[i]+=F[i-1];
    int j=1;
    for(i=0; i<100; i++)
    {
        if(i*F[4]>100*F[j]) j++;
        objPool[i]=j-1;
    }
    if(ag1==NULL)
        ag1=new yAgent;
    if(ag2==NULL)
        ag2=new noleaveGreedy;

    // The main game engine starts here
    agent *pa[2];
    pa[0]=ag1;
    pa[1]=ag2;
    state current;
    ActionSet legalMoves;
    int turn=1;
    int round=1;
    int player1Grant, player2Grant;
    player1Grant=player2Grant=0;
    while(true)
    {
        current=GameDef::initialState(turn,player1Grant,player2Grant);
        pa[0]->init(current);
        pa[1]->init(current);
        //int round = 1;
        while(!GameDef::goalTest(current))
        {
            round++;
            current.print();
            usleep(150000);  // delay to slow down the scrolling screen
            cout<< pa[current.turn]->badge() <<"] ";
            legalMoves = GameDef::legalMoves(current);
            int na=0;
            ActionSet A = legalMoves;
            action a=A.get_first_a();
            do
            {
                printAction(a);
                a=A.get_next_a(a);
                na++;
            }
            while(a!=Nop);
            if(na<2)
            {
                a=legalMoves.get_first_a();
                cout<<endl;
            }
            else
            {
                cout << "? ";
                a=pa[current.turn]->acton(current,legalMoves);
            }
            if(na==0 || legalMoves.include(a))
                GameDef::transit(current,a);
            else{
                cout << " Illegal action !!!";
            }
            cout << endl << endl << endl;
        }
        current.print();
        cout << "Round " << round << "] ";
        if(current.p[0].v == current.p[1].v){
            cout << "Draw" << endl;
        }
        else
        {
            int winner;
            if(current.p[0].v > current.p[1].v)
                winner=0;
            else
                winner=1;

            cout << " Winner: " << pa[winner]->badge() << endl;
        }
        round++;
        turn=1-turn;
        player1Grant=current.p[0].v;
        player2Grant=current.p[1].v;
        if(current.p[0].m<ROADLENGTH || current.p[1].m<ROADLENGTH) break;
        char cnt;
        cout << endl << "Continue [y/n]? ";
        cin >> cnt;
        if(cnt=='n' || cnt=='N') break;
        cout << endl;
    }
    cout << GameDef::value(current) << endl;
    delete pa[0];
    delete pa[1];
    return EXIT_SUCCESS;
}
