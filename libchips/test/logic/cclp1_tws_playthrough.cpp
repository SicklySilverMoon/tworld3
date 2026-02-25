#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "data/ccl/ccl_embeds.h"
#include "data/tws/tws_embeds.h"

extern "C" {
#include "formats.h"
#include "format-tws.h"
#include "logic.h"
}

struct LevelsetTwssetPair {
  LevelSet* set;
  TWSSet* tws;
};

namespace {
  LevelsetTwssetPair loadsets(uint8_t const* levelset, size_t levelset_size, uint8_t const* tws, size_t tws_size) {
    LevelsetTwssetPair pair = {};

    Result_LevelSetPtr res = parse_ccl(levelset, levelset_size);
    EXPECT_TRUE(res.success);
    pair.set = res.value;

    Result_TWSSetPtr tws_res = parse_tws(tws, tws_size);
    EXPECT_TRUE(tws_res.success);
    pair.tws = tws_res.value;
    return pair;
  }

  void freeset(LevelsetTwssetPair pair) {
    LevelSet_free(pair.set);
    TWSSet_free(pair.tws);
  }

  void print_moves(uint16_t level_num, const GameInput* move_list, uint32_t num_ticks) {
    const char moves_chars[] = {
      [DIRECTION_NIL] = '-',
      [DIRECTION_NORTH] = 'N',
      [DIRECTION_WEST] = 'W',
      [DIRECTION_SOUTH] = 'S',
      [DIRECTION_EAST] = 'E',
      [DIRECTION_NORTH | DIRECTION_WEST] = 'Q',
      [DIRECTION_SOUTH | DIRECTION_WEST] = 'Z',
      [DIRECTION_NORTH | DIRECTION_EAST] = 'R',
      [DIRECTION_SOUTH | DIRECTION_EAST] = 'V',
    };

    printf("%u: ", level_num);
    for (size_t i = 0; i < num_ticks; i++) {
      putc(moves_chars[move_list[i]], stdout);
    }
    putc('\n', stdout);
  }

  void testset(LevelsetTwssetPair pair) {
    for (size_t i = 0; i < pair.set->levels_n; i++) {
      Result_LevelPtr level_res = LevelMetadata_make_level(&pair.set->levels[i], &ms_logic);
      EXPECT_TRUE(level_res.success);
      Level* level = level_res.value;

      TWSMetadata* solution = &pair.tws->solutions[i];
      for (size_t j = 0; j < solution->num_ticks; j++) {
        level->game_input = solution->inputs[j];
        Level_tick(level);
      }
      if (!level->level_complete) {
        print_moves(solution->level_num, solution->inputs, solution->num_ticks);
      }
      EXPECT_TRUE(level->level_complete);
      Level_free(level);
    }
    freeset(pair);
  }

  TEST(CCLP1TWS, LoadAndPlayMS) {
    LevelsetTwssetPair pair = loadsets(CCLP1_ccl, sizeof(CCLP1_ccl), public_CCLP1_tws, sizeof(public_CCLP1_tws));
    EXPECT_EQ(pair.set->levels_n, pair.tws->solutions_n);
    testset(pair);
  }

  TEST(CCLP1TWS, LoadAndPlayLynx) {
    LevelsetTwssetPair pair = loadsets(CCLP1_ccl, sizeof(CCLP1_ccl), public_CCLP1_lynx_tws, sizeof(public_CCLP1_lynx_tws));
    EXPECT_EQ(pair.set->levels_n, pair.tws->solutions_n);
    testset(pair);
  }
}
