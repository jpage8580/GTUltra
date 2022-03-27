#ifndef GCHAREDITOR_H
#define GCHAREDITOR_H

#ifndef GCHAREDITOR_C


#endif

int getPixel(int charToDisplay, int x, int y);
void setPixel(int charToDisplay, int x, int y, int onOff);
void displayZoomedChar(int charToDisplay, int xp, int yp, int color);
void saveCharset();
void displayCharWindow();
void copyChar(int sourceCharIndex, int destCharIndex);
void flipCharX(int charIndex);
void inverseChar(int charIndex);

#endif
