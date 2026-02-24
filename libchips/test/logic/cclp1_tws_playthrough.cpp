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
      EXPECT_TRUE(level->level_complete);
      Level_free(level);
    }
    freeset(pair);
  }

  TEST(CCLP1TWS, LoadAndPlayMS) {
    testset(loadsets(CCLP1_ccl, sizeof(CCLP1_ccl), public_CHIPS_tws, sizeof(public_CHIPS_tws)));
  }

  TEST(CCLP1TWS, LoadAndPlayLynx) {
    testset(loadsets(CCLP1_ccl, sizeof(CCLP1_ccl), public_CHIPS_lynx_tws, sizeof(public_CHIPS_lynx_tws)));
  }
}
