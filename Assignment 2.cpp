#include <bits/stdc++.h>
using namespace std;

/* ===========================================================================
 * ENUMS and TYPE ALIASES
 * =========================================================================*/
enum Color      { GREEN, RED, GREENCLONE, REDCLONE };
enum Style      { NORMAL, ATTACKING };
using CoordPair = pair<int,int>;

/* ===========================================================================
 * MOVEMENT STRATEGY (Behavioral: Strategy Pattern)
 * =========================================================================*/
struct MovementStrategy {
    virtual ~MovementStrategy() = default;
    virtual CoordPair up(int x,int y)    const = 0;
    virtual CoordPair down(int x,int y)  const = 0;
    virtual CoordPair left(int x,int y)  const = 0;
    virtual CoordPair right(int x,int y) const = 0;
};

struct NormalMovement : MovementStrategy {
    CoordPair up   (int x,int y) const override { return {x-1, y}; }
    CoordPair down (int x,int y) const override { return {x+1, y}; }
    CoordPair left (int x,int y) const override { return {x, y-1}; }
    CoordPair right(int x,int y) const override { return {x, y+1}; }
};

struct AttackingMovement : MovementStrategy {
    CoordPair up   (int x,int y) const override { return {x-2, y}; }
    CoordPair down (int x,int y) const override { return {x+2, y}; }
    CoordPair left (int x,int y) const override { return {x, y-2}; }
    CoordPair right(int x,int y) const override { return {x, y+2}; }
};

/* ===========================================================================
 * FIGURE (Creational: Prototype Pattern)
 * =========================================================================*/
class Figure {
    Color color;
    bool alive            = true;
    bool clonedOnce       = false;
    int  x, y;
    Style style           = NORMAL;
    unique_ptr<MovementStrategy> moveStrat;

public:
    Figure(Color c,int X,int Y)
      : color(c), x(X), y(Y)
    {
        // default to normal
        moveStrat = make_unique<NormalMovement>();
    }

    virtual ~Figure() = default;

    // --- Prototype clone() ---
    virtual Figure* clone() const {
        // create a fresh Figure with the clone‑color
        Color nc = (color==GREEN ? GREENCLONE :
                    color==RED   ? REDCLONE   :
                                   color);
        Figure* f = new Figure(nc, x, y);
        f->clonedOnce = true;          // clones can’t be cloned again
        f->style      = NORMAL;        // always start NORMAL
        f->moveStrat  = make_unique<NormalMovement>();
        return f;
    }

    // getters / setters
    Color getColor() const      { return color; }
    int   getX()     const      { return x; }
    int   getY()     const      { return y; }
    bool  isAlive()  const      { return alive; }
    bool  gotCloned() const     { return clonedOnce; }
    Style getStyle() const      { return style; }

    void setAlive    (bool v)    { alive=v; }
    void setPosition(int X,int Y){ x=X; y=Y; }
    void markCloned  (bool v)    { clonedOnce=v; }

    // switch NORMAL<->ATTACKING
    bool toggleStyle(){
        if(!alive) return false;
        if(style==NORMAL){
            style = ATTACKING;
            moveStrat = make_unique<AttackingMovement>();
        } else {
            style = NORMAL;
            moveStrat = make_unique<NormalMovement>();
        }
        return true;
    }

    // delegate to strategy
    CoordPair up()    { return moveStrat->up   (x,y); }
    CoordPair down()  { return moveStrat->down (x,y); }
    CoordPair left()  { return moveStrat->left (x,y); }
    CoordPair right() { return moveStrat->right(x,y); }
};

/* ===========================================================================
 * FACADE: GAME ENGINE (Structural: Facade + Creational: Singleton)
 * =========================================================================*/
class GameEngine {
    int N;
    map<CoordPair,int> coins;                // (x,y)->value
    unordered_map<Color,Figure*> figures;    // color->figure ptr
    int greenScore = 0, redScore = 0;

    // --- Singleton plumbing ---
    GameEngine(int boardSize): N(boardSize) {}
    GameEngine(const GameEngine&)=delete;
    GameEngine& operator=(const GameEngine&)=delete;

public:
    static GameEngine& getInstance(int boardSize=0) {
        static GameEngine* inst = nullptr;
        if(!inst) inst = new GameEngine(boardSize);
        return *inst;
    }
    ~GameEngine(){
        for(auto &p:figures) delete p.second;
    }

    // init and data loading
    void initFigures(int gx,int gy,int rx,int ry){
        figures[GREEN] = new Figure(GREEN, gx, gy);
        figures[RED]   = new Figure(RED,   rx, ry);
    }
    void addCoin(int x,int y,int v){
        coins[{x,y}] = v;
    }

    // the one method the main() calls for every action
    string process(const string& name,const string& act){
        Color c = nameToColor(name);
        if(!figures.count(c) || !figures[c]->isAlive())
            return "INVALID ACTION";

        Figure* F = figures[c];
        if(act=="STYLE"){
            if(!F->toggleStyle()) return "INVALID ACTION";
            return name + " CHANGED STYLE TO " + sty2str(F->getStyle());
        }
        if(act=="COPY")  return handleCopy(name);
        if(act=="UP"||act=="DOWN"||act=="LEFT"||act=="RIGHT")
            return handleMove(name, act);
        return "INVALID ACTION";
    }

    // wrap up at the end
    string finalResult(){
        if(greenScore==redScore)
            return "TIE. SCORE "+to_string(greenScore)+" "+to_string(redScore);
        if(greenScore>redScore)
            return "GREEN TEAM WINS. SCORE "+to_string(greenScore)+" "+to_string(redScore);
        return "RED TEAM WINS. SCORE "+to_string(greenScore)+" "+to_string(redScore);
    }

private:
    // copy/clone logic
    string handleCopy(const string& name){
        Color c = nameToColor(name);
        Figure* F = figures[c];

        int X=F->getX(), Y=F->getY();
        if(X==Y)                        return "INVALID ACTION"; // diagonal no‑go
        if(c==GREENCLONE||c==REDCLONE)  return "INVALID ACTION"; // clone can’t clone
        if(F->gotCloned())              return "INVALID ACTION"; // one‑time only

        int tx=Y, ty=X;
        // occupied?
        for(auto &p:figures){
            Figure* G = p.second;
            if(G->isAlive() && G->getX()==tx && G->getY()==ty)
                return "INVALID ACTION";
        }
        if(coins.count({tx,ty}))        return "INVALID ACTION"; // coin there

        // do it!
        Figure* nc = F->clone();
        nc->setPosition(tx,ty);
        figures[nc->getColor()] = nc;
        F->markCloned(true);
        return name + " CLONED TO " + to_string(tx) + " " + to_string(ty);
    }

    // move logic
    string handleMove(const string& name,const string& act){
        Color c = nameToColor(name);
        Figure* F = figures[c];
        CoordPair np;
        if(act=="UP")    np = F->up();
        if(act=="DOWN")  np = F->down();
        if(act=="LEFT")  np = F->left();
        if(act=="RIGHT") np = F->right();

        int nx=np.first, ny=np.second;
        if(nx<1||nx>N||ny<1||ny>N)       return "INVALID ACTION";

        // block same‑team collision
        for(auto &p:figures){
            Figure* G = p.second;
            if(G->isAlive() && G->getColor()==c
            && G->getX()==nx && G->getY()==ny)
                return "INVALID ACTION";
        }

        // maybe kill
        bool killed=false; string who;
        for(auto &p:figures){
            Figure* G = p.second;
            if(G->isAlive()&&G->getX()==nx&&G->getY()==ny
            && !sameTeam(c,G->getColor())){
                killed=true;
                who = col2name(G->getColor());
                G->setAlive(false);
            }
        }
        // maybe collect a coin
        bool collected=false; int val=0;
        auto it = coins.find({nx,ny});
        if(it!=coins.end()){
            collected=true; val=it->second;
            if(isGreen(c)) greenScore+=val; else redScore+=val;
            coins.erase(it);
        }
        // move it
        F->setPosition(nx,ny);

        // build output
        if(killed)
            return name+" MOVED TO "+to_string(nx)+" "+to_string(ny)
                 +" AND KILLED "+who;
        if(collected)
            return name+" MOVED TO "+to_string(nx)+" "+to_string(ny)
                 +" AND COLLECTED "+to_string(val);
        return name+" MOVED TO "+to_string(nx)+" "+to_string(ny);
    }

    // helpers
    Color nameToColor(const string& s){
        if(s=="GREEN")      return GREEN;
        if(s=="RED")        return RED;
        if(s=="GREENCLONE") return GREENCLONE;
        if(s=="REDCLONE")   return REDCLONE;
        return GREEN; // fallback
    }
    string col2name(Color c){
        switch(c){
            case GREEN: return "GREEN";
            case RED:   return "RED";
            case GREENCLONE: return "GREENCLONE";
            case REDCLONE:   return "REDCLONE";
        }
        return "UNKNOWN";
    }
    bool isGreen(Color c){ return c==GREEN||c==GREENCLONE; }
    bool isRed  (Color c){ return c==RED  ||c==REDCLONE; }
    bool sameTeam(Color a,Color b){ return (isGreen(a)&&isGreen(b))||(isRed(a)&&isRed(b)); }
    string sty2str(Style s){ return s==NORMAL ? "NORMAL" : "ATTACKING"; }
};

/* ===========================================================================
 * MAIN
 * =========================================================================*/
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N; cin >> N;
    int gx,gy, rx,ry;
    cin >> gx >> gy;
    cin >> rx >> ry;

    // grab our one-and-only Engine (Singleton)
    GameEngine& engine = GameEngine::getInstance(N);

    engine.initFigures(gx,gy, rx,ry);

    int M; cin >> M;
    for(int i=0;i<M;i++){
        int x,y,v; cin>>x>>y>>v;
        engine.addCoin(x,y,v);
    }

    int P; cin >> P;
    while(P--){
        string fig,act;
        cin >> fig >> act;
        cout << engine.process(fig,act) << "\n";
    }

    cout << engine.finalResult() << "\n";
    return 0;
}
