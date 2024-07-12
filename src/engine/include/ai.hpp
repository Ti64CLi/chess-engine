#ifndef __AI_HPP__
#define __AI_HPP__

#include "engine.hpp"

namespace ai {
    struct MoveValuation {
        engine::Move move;
        int valuation;
    };

    class AI {
        private:
            engine::Game &game;
        
        public:
            AI() = delete;
            AI(engine::Game &game);

            int quiesceSearch(int alpha, int beta, unsigned int maxDepth);
            int alphaBeta(int alpha, int beta, unsigned int depth);
            MoveValuation negaMax(unsigned int depth);
    };
}

#endif