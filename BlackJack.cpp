#include "displayapp/screens/BlackJack.h"
#include "components/motion/MotionController.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Controllers;

static void ButtonEvent(lv_obj_t *obj, lv_event_t event) {
    auto *screen = static_cast<BlackJack*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
}
//PLACEHOLDERS
void BlackJack::OnButtonEvent(lv_obj_t *obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
        return ;
    }
    
    if (obj == start.button) {
        update_play();
        lv_scr_load(play_scr);
    } else if (obj == bet_decr.button) {
        decr_bet();
    } else if (obj == bet_incr.button) {
        incr_bet();
    } else if (obj == hit.button) {
        hit_flag = true;
    } else if (obj == stand.button) {
        stand_flag = true;
    }
}

void BlackJack::Refresh() {
    // Make sure this is the play screen, as we will be updating labels only existing there
    if (lv_scr_act() != play_scr) {
        return ;
    }

    // Initial deal phase. If the players hand has less than two cards, initial dealing isn't done
    if (player_cards.size() < 2) {
        Pause(500);
        if (player_cards.size() == dealer_cards.size()) {
            deal(&dealer_cards, &dealer_total);
        } else {
            deal(&player_cards, &player_total);
        }
    } else if (player_total > 21) { // Player loses on bust
        winner = DEALER;
        curr_bet *= -1;
        Pause(1000);
        Reset();
    } else if (hit_flag) {
        deal(&player_cards, &player_total);
        hit_flag = false;
    } else if (stand_flag) {
        if (dealer_total < 16) { // Dealer needs to hit
            Pause(500);
            deal(&dealer_cards, &dealer_total);
        } else if (dealer_total > 21) { // Dealer always loses when player doesn't bust and he does
            winner = PLAYER;
            curr_bet *= 2;
            stand_flag = false;
            Pause(1000);
            Reset();
        } else if (player_total > dealer_total) { // Player beats dealer with no busts and player having higher hand
            winner = PLAYER;
            curr_bet *= 2;
            stand_flag = false;
            Pause(1000);
            Reset();
        } else { // Player loses
            winner = DEALER;
            curr_bet *= -1;
            stand_flag = false;
            Pause(1000);
            Reset();
        }
    }
    update_play();
}

//END PLACEHOLDERS

BlackJack::BlackJack(Controllers::MotionController &motionController) : motionController {motionController} {
    curr_bet = 5;
    total_chips = 100;
    std::seed_seq sseq {static_cast<uint32_t>(motionController.X()),
                        static_cast<uint32_t>(motionController.Y()),
                        static_cast<uint32_t>(motionController.Z())};
    rand_gen.seed(sseq);
    refresh_task = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

    dealer_cards.clear();
    dealer_total = 0;
    player_cards.clear();
    player_total = 0;

    init_home();
    init_play();

    update_home();
    lv_scr_load(home_scr);
}

BlackJack::~BlackJack() {
    lv_task_del(refresh_task);
    lv_obj_clean(lv_scr_act());
}

/******HOME PAGE******/
void BlackJack::init_home() {
    CreateButton(&start, home_scr, ButtonEvent, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, LV_ALIGN_IN_LEFT_MID, 0, 0, (char *) "Start");
    CreateButton(&bet_decr, home_scr, ButtonEvent, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0, (char *) "-");
    CreateButton(&bet_incr, home_scr, ButtonEvent, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, LV_ALIGN_IN_TOP_RIGHT, 0, 0, (char *) "+");
    CreateLabel(&bet_amount, home_scr, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, LV_ALIGN_IN_RIGHT_MID, -50, 0, NULL);
    CreateLabel(&last_winner, home_scr, HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT, LV_ALIGN_IN_TOP_LEFT, 10, 0, NULL);
}

void BlackJack::update_home() {
    lv_label_set_text_fmt(bet_amount, "%i / %i", curr_bet, total_chips);
    if (winner == PLAYER) {
        lv_label_set_text_static(last_winner, "Winner: \nPlayer");
        lv_obj_set_hidden(last_winner, false);
    } else if (winner == DEALER) {
        lv_label_set_text_static(last_winner, "Winner: \nDealer");
        lv_obj_set_hidden(last_winner, false);
    } else {
        lv_obj_set_hidden(last_winner, true);
    }
}

void BlackJack::decr_bet() {
    if (curr_bet > 1) {
        curr_bet--;
        update_home();
    }
}

void BlackJack::incr_bet() {
    if (curr_bet < total_chips) {
        curr_bet++;
        update_home();
    }
}

/******PLAY PAGE******/
void BlackJack::init_play() {
    CreateButton(&hit, play_scr, ButtonEvent, PLAY_BUTTON_WIDTH, PLAY_BUTTON_HEIGHT, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0, (char *) "Hit");
    CreateButton(&stand, play_scr, ButtonEvent, PLAY_BUTTON_WIDTH, PLAY_BUTTON_HEIGHT, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0, (char *) "Stand");
    CreateLabel(&bet, play_scr, PLAY_BUTTON_WIDTH, PLAY_BUTTON_HEIGHT, LV_ALIGN_IN_BOTTOM_MID, -15, -10, NULL);
    CreateLabel(&dealer_hand, play_scr, PLAY_BUTTON_WIDTH, PLAY_BUTTON_HEIGHT, LV_ALIGN_IN_TOP_LEFT, 0, 0, (char *) "Dealer: ");
    CreateLabel(&player_hand, play_scr, PLAY_BUTTON_WIDTH, PLAY_BUTTON_HEIGHT, LV_ALIGN_IN_LEFT_MID, 0, 0, (char *) "Player: ");
    
    hit_flag = false;
    stand_flag = false;
}

void BlackJack::update_play() {
    lv_label_set_text_fmt(bet, "%i", curr_bet);
    lv_label_set_text_fmt(dealer_hand, "Dealer: %i", dealer_total);
    lv_label_set_text_fmt(player_hand, "Player: %i", player_total);
}

void BlackJack::deal(std::vector<int> *hand, int *total) {
    std::uniform_int_distribution<> distrib(1, 11);
    int value = distrib(rand_gen);
    hand->push_back(value);
    *total += value;
}

/******HELPER FUNCTIONS******/
void BlackJack::Pause(int ms) {
    vTaskDelay(ms);
}

void BlackJack::Reset() {
    dealer_cards.clear();
    dealer_total = 0;
    player_cards.clear();
    player_total = 0;

    total_chips += curr_bet;
    if (total_chips >= 5) {
        curr_bet = 5;
    } else {
        curr_bet = total_chips;
    }

    update_home();
    lv_scr_load(home_scr);
}

void BlackJack::CreateButton(button *b, lv_obj_t *par, lv_event_cb_t event_cb, uint8_t w, uint8_t h, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, char *text) {
    b->button = lv_btn_create(par, nullptr);
    b->button->user_data = this;
    lv_obj_set_event_cb(b->button, event_cb);
    lv_obj_set_size(b->button, w, h);
    lv_obj_align(b->button, par, align, x_ofs, y_ofs);
    b->label = lv_label_create(b->button, nullptr);
    lv_label_set_text_static(b->label, text);
}

void BlackJack::CreateLabel(lv_obj_t **l, lv_obj_t *par, uint8_t w, uint8_t h, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, char *text) {
    *l = lv_label_create(par, nullptr);
    lv_obj_set_size(*l, w, h);
    lv_obj_align(*l, par, align, x_ofs, y_ofs);
    lv_label_set_text_static(*l, text);
}