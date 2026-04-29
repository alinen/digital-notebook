#ifndef SDStub_H_
#define SDStub_H_

#include <stdio.h>
#define FILE_WRITE "r+"

class File {
  public:
    File(FILE* fptr, bool isdir) : 
      mfp(fptr), mIsdir(isdir) {

    }
    
    void print(const char* str) {

    }

    void println(const char* str) {

    }
  
    void seek(int offset, int start) {

    }

    void close() {
      fclose(mfp);
    }
  
    ~File() {
      close();
      mfp = NULL;
    }
  private:
  FILE* mfp;
  bool mIsdir;
};

class SD {
  static File open(const char* name, const char* options) {
    FILE* fp = fopen(name, options);
    return File(fp, false);
  }

  static bool exists(const char* name) {
    return false;
  }

  static void mkdir(const char* name) {

  }
};

#endif