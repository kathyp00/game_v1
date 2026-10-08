#include "Game.h"
using namespace std;

int main(int argc, char* argv[]) {
    Game mygame("game_engine", 1088, 800, 1088, 800);
    mygame.init();
    while(mygame.running()) {
        mygame.handle_events();
        mygame.game_update();
        mygame.render();
    }

    mygame.clean();
    return 0;
}