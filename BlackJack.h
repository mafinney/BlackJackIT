#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include "components/stopwatch/StopWatchController.h"
#include <vector>
#include <random>

namespace Pinetime::Applications::Screens {
    class BlackJack : public Screen {
        public:
            BlackJack(Controllers::MotionController &motionController);
            ~BlackJack() override;
            void OnButtonEvent(lv_obj_t *obj, lv_event_t event);
            void Refresh() override;

        private:
            static constexpr uint8_t HOME_BUTTON_WIDTH = 115;
            static constexpr uint8_t HOME_BUTTON_HEIGHT = 80;
            static constexpr uint8_t PLAY_BUTTON_WIDTH = 80;
            static constexpr uint8_t PLAY_BUTTON_HEIGHT = 50;

            // Each button is made of two lv objects, a button and a label
            struct button {
                lv_obj_t *button;
                lv_obj_t *label;
            };

            struct hand {
                lv_obj_t *label; // Display the hand
                std::vector<int> cards; // Keep all the cards dealt to them
                int total; // Total value of the hand
            };

            enum Winner {
                PLAYER,
                DEALER,
                NONE
            };

            lv_task_t *refresh_task; // Refresh task is the main game loop. it updates on hit and does stand logic

            std::mt19937 rand_gen;
            Controllers::MotionController &motionController;

            int total_chips;
            int curr_bet;

            /*HOME PAGE*/
            lv_obj_t *home_scr = lv_obj_create(NULL, NULL);
            button start;
            button bet_decr;
            button bet_incr;
            lv_obj_t *bet_amount;
            lv_obj_t *last_winner;
            Winner winner = NONE;

            /**
             * init_home does all first time setup for the home page
             * This includes creating the buttons and labels
             * This should only be called once
             */
            void init_home();

            /**
             * update_home does any housekeeping for the home page when it is about to be redisplayed
             * This includes updating the values for chip counts
             * This should be called right before switching the display to the home screen
             */
            void update_home();
            
            /**
             * decr_bet decreases the current bet amount by one. Doesn't allow rollovers or values lower than 1
             */
            void decr_bet();

            /**
             * incr_bet increases the current bet amount by one. Doesn't allow rollovers or values greater than the total chips
             */
            void incr_bet();

            /*PLAY PAGE*/
            lv_obj_t *play_scr = lv_obj_create(NULL, NULL);
            button hit;
            button stand;
            lv_obj_t *bet;

            struct hand dealer;
            struct hand player;

            bool hit_flag;
            bool stand_flag;

            /**
             * init_play does all first time setup for the play page
             * This includes creating buttons and labels
             * This should only be called once
             */
            void init_play();

            /**
             * update_play does any housekeeping for the play page when it is about to be redisplayed
             * This includes updated the values for chip counts
             * This should be called right before switching the display to the play screen
             */
            void update_play();

            /**
             * hand_to_str converts a blackjack hand to a string.
             * Ex: if the hand contains a 5 and a 7, this function will return "5 7 (12)"
             */
            std::string hand_to_str(struct hand *h);

            /**
             * deal deals a random card to the specified hand
             */
            void deal(struct hand *h);

            /**
             * stand_logic figures out which player won and sets curr_bet and winner accordingly
             * This should only be called after both players are done being dealt too
             */
            void stand_logic();

            /*HELPER FUNCTIONS*/

            /**
             * Pause puts the process to sleep for a specified number of milliseconds
             */
            void Pause(int ms);

            /**
             * Reset resets game state and applies any wins or losses to the total chip count
             * then takes the player to the home screen
             */
            void Reset();

            /**
             * CreateButton is a wrapper function for all the calls needed to create a button struct object
             * Takes a pointer to a button
             * A parent screen
             * A function callback
             * A size
             * An alignment
             * An initial label
             */
            void CreateButton(button *b, lv_obj_t *par, lv_event_cb_t event_cb, uint8_t w, uint8_t h, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, char *text);

            /**
             * CreateLabel is a wrapper function for all the calls needed to create a label
             * Takes a pointer to a label
             * A parent screen
             * A size
             * An alignment
             * An initial label
             */
            void CreateLabel(lv_obj_t **l, lv_obj_t *par, uint8_t w, uint8_t h, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, char *text);
    };
}

namespace Pinetime::Applications {
    template <>
    struct AppTraits<Apps::BlackJack> {
        static constexpr Apps app = Apps::BlackJack;
        static constexpr const char *icon = Screens::Symbols::dice;

        static Screens::Screen *Create(AppControllers &controllers) {
            return new Screens::BlackJack(controllers.motionController);
        };

        static bool IsAvailable(Pinetime::Controllers::FS &) {
            return true;
        };
    };
}