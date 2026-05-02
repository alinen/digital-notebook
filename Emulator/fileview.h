#ifndef fileview_H_
#define fileview_H_

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <cassert>
#include <cstring>
#include <algorithm>

namespace dnb { // namespace digital notebook

  const int LINEINC = 32;
  class Fileview {
  public:
    Fileview();
    ~Fileview();

    bool empty();
    bool open(const char* filename);
    bool save(const char* filename);

    int processChar(uint8_t ch, int keycode);
    int numLines() const;
    void getCursorPos(int& row, int& col) const;

    const char* errorstr() const;
    friend void fvTests();

    struct line {
      char* buf;
      int maxlen;
      int len;
      line* next;
      line* prev;
    };
    line* getStartRow(int width, int height) const;

  private:

    line* newLine();
    void clear(); 
    void insertChar(line* row, int c, char ch); // add ch after c
    void deleteLine(line* row);
    void deleteChar(line* row, int c);
    line* addLine(line* row); // add new line after row

    int mError;
    line* mCursorRow;
    line* mScreenRow;
    int mCursorCol;
    int mCursorRowNum;
    int mNLines;
    line* mFirst;
    line* mLast;
  };

  Fileview::Fileview() : 
    mError(0),
    mCursorRow(NULL),
    mScreenRow(NULL),
    mCursorCol(0),
    mCursorRowNum(0),
    mNLines(0),
    mFirst(NULL),
    mLast(NULL) 
  {
    empty();
  }

  Fileview::~Fileview() { 
    clear(); 
  }

  void Fileview::clear() {
    line* ll = mFirst;
    while (ll) {
      delete[] ll->buf;
      line* current = ll;
      ll = ll->next;
      delete current;
    }
  }

  bool Fileview::empty() {
    clear();

    line* line = newLine();
    mFirst = line;
    mLast = line;
    mNLines = 1;
    mCursorRow = mFirst;
    mCursorCol = 0;
    mCursorRowNum = 0;
    return true;
  }

  bool Fileview::open(const char* filename) {
    return true;
  }

  bool Fileview::save(const char* filename) {
    return true;
  }

  int Fileview::numLines() const {
    return mNLines;
  }

  Fileview::line* Fileview::getStartRow(int width, int height) const {
    if (mCursorRowNum < height) return mFirst;

    Fileview::line* start = mCursorRow;
    for (int i = 0; i < height; i++, start = start->prev);
    return start;
  }

  int Fileview::processChar(uint8_t ch, int keycode) {
    if (keycode == 0x4F) { // right
      mCursorCol = std::min(mCursorRow->maxlen, mCursorCol+1);
    }
    else if (keycode == 0x50) { // left 
      mCursorCol = std::max(0, mCursorCol-1);
    }
    else if (keycode == 0x51) { // down
      if (mCursorRow->next) {
        mCursorRow = mCursorRow->next;
        mCursorRowNum++;
        mCursorCol = std::min(mCursorRow->len-1, mCursorCol);
      }
    }
    else if (keycode == 0x52) {  // up
      if (mCursorRow->prev) {
        mCursorRow = mCursorRow->prev;
        mCursorRowNum--;
        mCursorCol = std::min(mCursorRow->len-1, mCursorCol);
      }
    }
    else if (ch == 8) { // backspace
      if (mCursorCol == 0) { // delete line
        line* prevLine = mCursorRow->prev;
        if (prevLine) {
          mCursorCol = prevLine->len - 1;
          ////if (mCursorRow->len > 1) { // backspace on non-empty line
          ////  appendLine(prevLine, mCursorRow);
          ////}
          deleteLine(prevLine->next);
          mCursorRow = prevLine;
          mCursorRowNum--;
        }
        else {
          mCursorRow = mFirst;
          mCursorCol = mFirst->len - 1;
          mCursorRowNum = 0;
        }
      }
      else { // delete char
        deleteChar(mCursorRow, mCursorCol);
        mCursorCol--;
      }
    }
    else if (ch == 13) {
      line* newRow = addLine(mCursorRow);
      //if (mCursorCol < mCursorRow->len - 1) { // newline in middle of line
      //  moveChars(newRow, mCursorRow, mCursorCol);
      //}
      mCursorRow = newRow;
      mCursorRowNum++;
      mCursorCol = 0;
    }
    else {
      insertChar(mCursorRow, mCursorCol, ch);
      mCursorCol++;
    }
    return 0;
  }

  void Fileview::getCursorPos(int& row, int& col) const {
    row = mCursorRowNum;
    col = mCursorCol;
  }

  void Fileview::insertChar(Fileview::line* row, int c, char ch) {
    if (row->len >= row->maxlen) { // allocate more space
      char* newBuf = new char[row->maxlen + LINEINC]; // TODO: Error codes
      strncpy(newBuf, row->buf, row->maxlen);
      delete[] row->buf;
      row->buf = newBuf;
      row->maxlen = row->maxlen + LINEINC;
    }
    // shift right
    for (int i = row->len; i > c; i--){
      row->buf[i] = row->buf[i-1];
    }
    row->buf[c] = ch;
    row->len++;
  }

  void Fileview::deleteLine(Fileview::line* row) {
    Fileview::line* prev = row->prev;
    Fileview::line* next = row->next;

    if (!prev && !next) return; // never delete all lines
    else if (!prev) { // change head of the list
      mFirst = next;
    }
    else {
      prev->next = next;
      if (next) next->prev = prev;
    }
    delete[] row->buf;
    delete row;
    mNLines--;
  }

  void Fileview::deleteChar(Fileview::line* row, int c) {
    assert(c > 0);
    for (int i = c-1; i < row->len; i++) { // shift right
      row->buf[i] = row->buf[i+1];
    }
    row->len--;
  }

  Fileview::line* Fileview::addLine(Fileview::line* row) {
    assert(row != NULL);
    line* ll = newLine();
    ll->next = row->next;
    ll->prev = row;
    if (row->next) row->next->prev = ll;
    row->next = ll;
    mNLines++;

    return ll;
  }

  const char* Fileview::errorstr() const {
    switch (mError) {
      case ENOENT: return "File not found";
      case ENOMEM: return "Out of memory";
      default: return "";
    } 
    return "";
  }
    
  Fileview::line* Fileview::newLine() {
    line* line = new Fileview::line();
    line->buf = new char[LINEINC];
    line->maxlen = LINEINC;
    line->len = 1;
    line->buf[0] = '\n';
    line->next = line->prev = NULL;
    return line;
  }

  void fvTests() {
    int r, c;
    dnb::Fileview fv;
    fv.processChar('a', 0); 
    fv.getCursorPos(r, c);
    assert(r == 0 && c == 1);
    assert(strncmp(fv.mFirst->buf, "a\n", 2) == 0);
    assert(fv.mFirst->len == 2);
    assert(fv.mNLines == 1);

    fv.processChar('b', 0);
    fv.getCursorPos(r, c);
    assert(r == 0 && c == 2);
    assert(strncmp(fv.mFirst->buf, "ab\n", 3) == 0);
    assert(fv.mFirst->len == 3);
    assert(fv.mNLines == 1);

    fv.processChar(13, 13);
    fv.getCursorPos(r, c);
    assert(r == 1 && c == 0);
    assert(strncmp(fv.mFirst->next->buf, "\n", 1) == 0);
    assert(fv.mFirst->next->len == 1);
    assert(fv.mNLines == 2);

    fv.processChar(13, 13);
    fv.processChar('x', 0);
    fv.processChar('y', 0);
    fv.processChar(13, 13);
    fv.processChar('1', 0);
    fv.processChar('2', 0);
    fv.getCursorPos(r, c);
    assert(r == 3 && c == 2);

    fv.processChar(0, 0x50);
    fv.getCursorPos(r, c);
    assert(r == 3 && c == 1);

    fv.processChar(0, 0x4F);
    fv.getCursorPos(r, c);
    assert(r == 3 && c == 2);

    fv.processChar(0, 0x52);
    fv.getCursorPos(r, c);
    assert(r == 2 && c == 2);

    fv.processChar(0, 0x51);
    fv.getCursorPos(r, c);
    assert(r == 3 && c == 2);

    fv.processChar(0, 0x50); // move left
    fv.processChar(8, 8);
    fv.getCursorPos(r, c);
    assert(r == 3 && c == 0);
    assert(strncmp(fv.mCursorRow->buf, "2\n", 2) == 0);
    assert(fv.mCursorRow->len == 2);
    assert(fv.mNLines == 4);

    fv.processChar(8, 8); // go to previous line
    fv.getCursorPos(r, c);
    assert(r == 2 && c == 2);
    assert(strncmp(fv.mCursorRow->buf, "xy\n", 2) == 0);
    assert(fv.mCursorRow->len == 3);
    assert(fv.mNLines == 3);
  }

}; // end namespace dnb
#endif