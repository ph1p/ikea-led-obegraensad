#ifndef GOL_HEADER_H
#define GOL_HEADER_H

uint8_t updateCell(int row, int col);
uint8_t countNeighbours(int row, int col);
void next();
void gameOfLifeSetup();
void gameOfLifeLoop();

#endif