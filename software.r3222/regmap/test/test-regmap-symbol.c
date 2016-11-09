// This program is just to test compilation, when run it will segfault

#include <stdio.h>

#include "regmap/regmap-symbol.h"

int
main(int argc, char** argv)
{
  u_long offset = 0xFB900000;

  // Read
  u_long board_id=0;
  regmap_cfg_get_option(offset, BF_BOARD_ID_SVN, &board_id);
  printf("Board ID:        %lx\n", board_id);

  // Write
  regmap_cfg_set_option(offset, BF_STATUS_LEDS, 0xFF);

  // Read Array
  u_long gbt_ctrl=0;
  regmap_cfg_get_option(offset, BF_CR_TH_GBT00_EGROUP0_CTRL, &gbt_ctrl);
  printf("GBT CTRL:        %lx\n", gbt_ctrl);

  u_long gbt_mon=0;
  regmap_cfg_get_option(offset, BF_CR_TH_GBT00_MON, &gbt_mon);
  printf("GBT MON:         %lx\n", gbt_mon);

  return 0;
}
