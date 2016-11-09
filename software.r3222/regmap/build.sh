#!/bin/sh

wuppercodegen=../wuppercodegen/wuppercodegen/cli.py
registers=../../firmware/sources/templates/registers-3.2.yaml
$wuppercodegen --version
$wuppercodegen $registers src/regmap-struct.h.template regmap/regmap-struct.h
$wuppercodegen $registers src/regmap-symbol.h.template regmap/regmap-symbol.h
$wuppercodegen $registers src/regmap-symbol.c.template src/regmap-symbol.c
