// This program is just to test compilation, when run it will segfault

#include <stdio.h>

#include "regmap/regmap-struct.h"

int
main(int argc, char** argv)
{
  flxcard_bar2_regs_t bar2;

  // Read
  u_long board_id = bar2.BOARD_ID_TIMESTAMP;
  printf("Board ID:        %lx\n", board_id);

  // Read bitfield
  u_long interrupts = bar2.GENERIC_CONSTANTS.INTERRUPTS;
  printf("Interrupts:      %lx\n", interrupts);

  // Write
  bar2.STATUS_LEDS = 0x00ff;

  // Read Array
  u_long gbt_ctrl = bar2.CR_GBT_CTRL[0].EGROUP_TH[0].TH;
  printf("GBT CTRL:        %lx\n", gbt_ctrl);

  u_long gbt_mon = bar2.CR_GBT_MON[0].TH;
  printf("GBT MON:         %lx\n", gbt_mon);

  // Read Array bitfield
  u_long gbt_ic_datain = bar2.IC_FIFOS[0].FH.DATAIN;
  printf("GBT IC:        %lx\n", gbt_ic_datain);

  return 0;
}
