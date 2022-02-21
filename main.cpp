#ifndef __MAIN_CPP__
#define __MAIN_CPP__
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "source/Turn.h"
#include "source/chi_more_than_two.h"
#include "source/naki.h"
#include "source/player.h"
#include "source/tile.h"
#include "source/wall.h"
#include "source/win_check.h"

void SetUp(Player &player1, Player &player2, Wall &wall,
           std::vector<std::string> sortby) {
  // declare std::vector<std::vector<std::string>> for temphand
  std::vector<std::vector<std::string>> temphand = {{}, {}};
  wall.AppendToWallHand(sortby);
  random_shuffle(wall.hand.begin(), wall.hand.end());
  // player drawcard 3 time each time draw 4 tile total is 12 tile
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < temphand.size(); j++)
      for (int k = 0; k < 4; k++) wall.DrawCard(temphand[j]);
  // playerdraw the 13th tile
  for (int i = 0; i < temphand.size(); i++) wall.DrawCard(temphand[i]);
  // player1 draw the 14th tile
  wall.DrawCard(temphand[0]);
  // sort all playerhand
  for (int i = 0; i < temphand.size(); i++) Sort(sortby, temphand[i]);
  // player1 be bookmaker
  if (player1.bookmaker) {
    player1.hand = temphand[0];
    player2.hand = temphand[1];
  } else if (player2.bookmaker) {
    player2.hand = temphand[0];
    player1.hand = temphand[1];
  }
}

void Window_Draw_all(sf::RenderWindow &window, Player &player1, Player &player2,
                     Turn &turn, Naki &naki, Chi_More_Than_Two &cmtt,
                     sf::Text &tex) {
  // clear window
  window.clear(sf::Color::White);
  player1.Player_Window_Draw(window);
  player2.Player_Window_Draw(window);
  turn.WindowDraw(window);
  naki.Naki_Window_Draw(window);
  cmtt.Chi_Window_Draw(window);
  window.draw(tex);
  // window dispalyr
  window.display();
}

int main() {
  /////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// declare ///////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  // float
  float tilescale = 0.2f;
  float duration = 1.0f;
  // int
  int player_select = 0;
  // #-1 = off game, #0 = discard, #1 = waiting period after discard,
  // #2 = check win or naki, #3 = naki select, #4 = chi more than two,
  // #5 = make combined
  int state = 0;
  // declare vector<string>
  std::vector<std::string> sortby = {"1m", "2m", "3m", "4m", "5m", "6m",
                                     "7m", "8m", "9m", "1z", "2z", "3z",
                                     "4z", "5z", "6z", "7z"};

  // sf::Event
  sf::Event event;
  // sf::Vector2<T>
  sf::Vector2i mousepos;
  // sf::Clock
  sf::Clock clock;
  // sf::Time
  sf::Time countdowntime;
  // font
  sf::Font font;
  sf::Text tex;
  font.loadFromFile("./font/msjhbd.ttc");
  tex.setFont(font);
  tex.setCharacterSize(20);
  tex.setFillColor(sf::Color::Black);
  tex.setStyle(sf::Text::Bold);
  // sf::RenderWindow
  sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
  // struct and class
  Wall wall;
  Tile tile;
  Player player1;
  Player player2;
  Turn turn;
  CheckWin cw;
  Naki naki;
  Chi_More_Than_Two cmtt;
  // set random seed
  srand(unsigned(time(0)));
  // set clase sortby
  cw.SetSortby(sortby);
  // load image
  tile.LoadTileImg();
  turn.LoadArrowImg();
  naki.LoadNakiImg();
  // set player code, name and bookmaker
  player1.code = 0;
  player2.code = 1;
  player1.name = "Player1";
  player2.name = "player2";
  player1.bookmaker = true;
  // set up player and wall
  SetUp(player1, player2, wall, sortby);
  // set player hand, river and naki Scale, Position, Rotation and Texture
  player1.SetHand_MeldAndRiverScale(tilescale);
  player2.SetHand_MeldAndRiverScale(tilescale);
  naki.SetNakiScale(tilescale+0.1f);
  player1.SetHand_MeldAndRiverPositionAndRotation(175, 530, 175, 325, 710, 530);
  player2.SetHand_MeldAndRiverPositionAndRotation(203, 50, 175, 300, 150, 50);
  turn.SetArrowScaleAndPosition(tilescale + 0.3f, 175, 250);
  naki.SetNakiPosition(650, 100);
  player1.SetPlayerTexture(tile);
  player2.SetPlayerTexture(tile);

  if (cw.Win(player1, player2)) state = -1;

  while (window.isOpen()) {
    countdowntime = clock.restart();
    while (window.pollEvent(event)) {
      // get mouse position
      mousepos = sf::Mouse::getPosition(window);
      switch (event.type) {
        case sf::Event::Closed:
          window.close();
          break;
        case sf::Event::MouseButtonPressed:
          if (event.mouseButton.button == sf::Mouse::Left) {
            switch (state) {
              // discard
              case 0:
                // player1
                if (turn.turn == 0) {
                  player_select = player1.Select(mousepos);
                  if (player_select != 0) {
                    player1.DisCardFromHandToRiver(player_select);
                    Sort(sortby, player1.hand);
                    player1.SetHand_MeldAndRiverPositionAndRotation(
                        175, 530, 175, 325, 710, 530);
                    player1.SetPlayerTexture(tile);
                    state = 1;
                    Window_Draw_all(window, player1, player2, turn, naki, cmtt,
                                    tex);
                  }
                }
                break;
              // naki select
              case 3:
                if (turn.turn == 1) {
                  player_select = naki.Select(mousepos);
                  // player select chi, pong or kang
                  if (player_select != -1) {
                    switch (player_select) {
                      // chi
                      case 1:
                        // more than one
                        if (naki.probablysequence.size() > 1) {
                          // set state to 4(chi more than two)
                          state = 4;
                          // reset player_select
                          player_select = 0;
                          // set chi more than two scale ,position and texture
                          cmtt.SetScaleAndPosition(tilescale);
                          cmtt.SetChiTexture(tile, naki.probablysequence);
                          Window_Draw_all(window, player1, player2, turn, naki,
                                          cmtt, tex);
                        } else {
                          // set state to 5 (make combined)
                          state = 5;
                        }
                        break;
                      // pong
                      case 2:
                        state = 5;
                        break;
                      // kang
                      case 3:
                        state = 5;
                        break;
                    }

                    // player skip combined
                  } else {
                    // reset naki
                    naki.ClearNaki();
                    // set naki texture
                    naki.SetNakiTextureShow();
                    // player1 draw card
                    wall.DrawCard(player1.hand);
                    if (cw.Win(player1,player2)) state = -1;
                    // set texture
                    player1.SetPlayerTexture(tile);
                    // set gameturn = 0
                    turn.turn--;
                    // set state to 0(discard)
                    state = 0;
                    Window_Draw_all(window, player1, player2, turn, naki, cmtt,
                                    tex);
                  }
                }
                break;
              case 4:
                player_select = cmtt.Select(mousepos, naki.probablysequence);
                state = 5;
                break;
            }
          }
          break;
        default:  // no mouse click and not quite game
          break;
      }
    }
    // no mouse click and not quite game
    switch (state) {
      // highlight
      case 0:
        // player1 highlihgt
        if (turn.turn == 0) player1.ChooseHighLightOrNot(mousepos);
        // player2 discard
        else if (turn.turn == 1) {
          player_select = static_cast<int>(player2.hand.size());
          player2.DisCardFromHandToRiver(player_select);
          Sort(sortby, player2.hand);
          player2.SetPlayerTexture(tile);
          state = 1;
          Window_Draw_all(window, player1, player2, turn, naki, cmtt, tex);
        }
        break;
      // waiting period
      case 1:
        // time > 0
        if (duration > 0) {
          duration -= countdowntime.asSeconds();
          tex.setString(std::to_string((int)duration));
          Window_Draw_all(window, player1, player2, turn, naki, cmtt, tex);
          // time < 0
        } else if (duration <= 0) {
          // player1 discard wait
          if (turn.turn == 0)
            // set state to 2
            state = 2;
          // player2 discard wait
          else if (turn.turn == 1) {
            // check player2 hand is 14
            if (player2.hand.size() == 14)
              // set state to 0(discard)
              state = 0;
            // player hand size isn't 14
            else
              // set state to 2
              state = 2;
          }
          // reset duration
          duration = 1.0f;
        }
        Window_Draw_all(window, player1, player2, turn, naki, cmtt, tex);
        break;
      // check naki
      case 2:
        // player2 drawcard and set gameturn to 1(player2 discard)
        if (turn.turn == 0) {
          state = 1;
          wall.DrawCard(player2.hand);
          player2.SetPlayerTexture(tile);
          Window_Draw_all(window, player1, player2, turn, naki, cmtt, tex);
          turn.turn++;
          // player1 check is winning or meld
        } else if (turn.turn == 1) {
          // win check(ron)
          if (cw.Win(player1, player2)) state = -1;
          // combined check
          else {
            naki.Check(player2.river, player1);
            // has combined
            if (naki.hascombined) {
              // reset player_select
              player_select = 0;
              // set state to 3 to naki
              state = 3;
              // show naki
              naki.SetNakiTextureShow();
              // no combined
            } else {
              // set state to 0
              state = 0;
              // drawcard for player1
              wall.DrawCard(player1.hand);
              player1.SetPlayerTexture(tile);
              Window_Draw_all(window, player1, player2, turn, naki, cmtt, tex);
              // set gameturn to 0(player1 dicard)
              turn.turn--;
              // check win(tsumo)
              if (cw.Win(player1, player2)) {
                state = -1;
              }
            }
          }
        }
        break;
      // make combined
      case 5:
        if (turn.turn == 1) {
          switch (player_select) {
            case 1:  // chi
              player1.Chi_Combined(naki.probablysequence[0], player2.river);
              break;
            case 2:  // pong
              player1.Pong_Combined(player2.river);
              break;
            case 3:  // kang
              player1.Big_Kang_Combined(player2.river, wall.hand,
                                        wall.walltailtop);
              break;
            case 11:  // chi first
              player1.Chi_Combined(naki.probablysequence[0], player2.river);
              break;
            case 12:  // chi second
              player1.Chi_Combined(naki.probablysequence[1], player2.river);
              break;
            case 13:  // chi third
              player1.Chi_Combined(naki.probablysequence[2], player2.river);
              break;
          }
          player1.SetPlayerTexture(tile);
          player2.SetPlayerTexture(tile);
          naki.ClearNaki();
          naki.SetNakiTextureShow();
          cmtt.SetChiTexture(tile, naki.probablysequence);
          Window_Draw_all(window, player1, player2, turn, naki, cmtt, tex);
          player_select = 0;
          state = 0;
          turn.turn--;
        }
        break;
    }
    Window_Draw_all(window, player1, player2, turn, naki, cmtt, tex);
  }
  return 0;
}
#endif