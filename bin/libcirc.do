#!/bin/sh
redo-ifchange ../src/g_game
ln --symbolic ../src/g_game "$3"
